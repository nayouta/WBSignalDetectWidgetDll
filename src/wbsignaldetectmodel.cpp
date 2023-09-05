#include "WBSignalDetectModel.h"
#include <QDateTime>
#include <QTimer>
#include <QSettings>

//考虑使用全局量记录频点识别门限以及带宽识别门限
uint g_FreqPointThreshold = 10; //单位为Hz
uint g_BandwidthThreshold = 10; //单位为Hz

WBSignalDetectModel::WBSignalDetectModel(QObject *parent): QAbstractTableModel(parent)
{
    m_Font.setFamily("Microsoft Yahei");
    connect(this, &WBSignalDetectModel::sigTriggerRefreshData, this, &WBSignalDetectModel::UpdateData);

    //通过信号存活检测定时器触发界面数据更新
    m_pSignalActiveChecker = new QTimer(this);
    m_pSignalActiveChecker->setInterval(1000);
    connect(m_pSignalActiveChecker, &QTimer::timeout, this, [this] {
        emit sigTriggerRefreshData();
    });
    m_pSignalActiveChecker->start();
    QList<int> exampleFreqList;
    exampleFreqList << 2e6 << 2.5e6 << 5e6 << 10e6 << 15e6 << 20e6 << 25e6;
    setLstTypicalFreq(exampleFreqList);
}

QVariant WBSignalDetectModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && m_eUserViewType != NOT_USED && orientation == Qt::Horizontal)
    {
        if (m_eUserViewType == SIGNAL_DETECT_TABLE)
        {
            static constexpr const char* HEADER_LABEL[] = { "序号", "中心频率(MHz)", "电平(dBuV)", "带宽(MHz)", "起始时间", "结束时间", "占用带宽", "信号占用度", "合法信号" };
            return (section < 0 || section > 8)? "": HEADER_LABEL[section];
        }
        else if (m_eUserViewType == DISTURB_NOISE_TABLE)
        {
            static constexpr const char* HEADER_LABEL[] = { "序号", "中心频率(MHz)", "大信号电平(dBuV)", "起始时间", "结束时间" };
            return (section < 0 || section > 4)? "": HEADER_LABEL[section];
        }
        else if (m_eUserViewType == MAN_MADE_NOISE_TABLE)
        {
            static constexpr const char* HEADER_LABEL[] = { "典型频率点(MHz)", "测量频率(MHz)", "起始时间", "结束时间", "测量电平(dBuV)" };
            return (section < 0 || section > 4)? "": HEADER_LABEL[section];
        }
    }
    return QVariant();
}

int WBSignalDetectModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid()? 0: m_DisplayData.length();
}

int WBSignalDetectModel::columnCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
    {
        //三个表格同时只能显示其中一个，根据当前使用者表格类型进行显示
        //TODO:后续可能需要三个表格同时显示，则采用对col进行hide的方式实现
        if (m_eUserViewType == NOT_USED)
            return 0;
        if (m_eUserViewType == DISTURB_NOISE_TABLE)
            return 5;
        if (m_eUserViewType == SIGNAL_DETECT_TABLE)
            return 9;
        if(m_eUserViewType == MAN_MADE_NOISE_TABLE){
            return 5;
        }
    }
    return 0;
}

bool WBSignalDetectModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole)
    {
        m_DisplayData[index.row()][index.column()] = value;
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

Qt::ItemFlags WBSignalDetectModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    auto flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    if (index.column() == 8 && m_eUserViewType == SIGNAL_DETECT_TABLE && m_bIsSettingLegalFreqFlag)      //仅信号选择表格的最后一列，且当前处于正在编辑合法信号的暂停状态时可编辑
        flags |= Qt::ItemIsEditable;
    return flags;
}

QVariant WBSignalDetectModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && m_eUserViewType != NOT_USED)
    {
        if (role == Qt::DisplayRole || role == Qt::EditRole) //显示内容
        {
            if (index.column() == 8)
                return m_DisplayData[index.row()][index.column()].toBool()? "是": "否";
            else
                return m_DisplayData[index.row()][index.column()];
        }
        else if (role == Qt::TextAlignmentRole) //内容排版
            return Qt::AlignCenter;
        else if (role == Qt::FontRole) //字体
            return m_Font;
    }
    return QVariant();
}

int WBSignalDetectModel::FindSignal(float *FFtin, int InStep, int length, int Freqency, int BandWidth)
{
    if(!m_bIsDetecting){
        return 0;
    }
    if (m_i64SystemStartTime == 0)
    {
        m_i64SystemStartTime = QDateTime::currentMSecsSinceEpoch();
        m_i64SystemStopTime = m_i64SystemStartTime;
        m_i64CurrentDetectStartTime = m_i64SystemStartTime;
    }
    m_iFullBandWidth = BandWidth;
    int ret = 1;
    m_lstSignalInfo.clear();
    findPeakCyclically(FFtin, length, Freqency, BandWidth);
    if (!m_lstSignalInfo.isEmpty())
        ret = 0;
    bool foundFlag = false;
    //两种情况：当前信号 1、从未有过  2、曾经有过
    //LZMK: 不再区分曾有现停的情况，用户从信号检测表格选择合法信号仅针对单个信号特征进行检测，无所谓停止后再出现
    foreach (const SignalInfo& curInfo, m_lstSignalInfo)
    {
        foreach (const SignalBaseChar& curBaseInfo, m_mapValidSignalCharacter.keys())
        {
            if (curInfo.BaseInfo == curBaseInfo)        //有过记录，更新记录
            {
                foundFlag = true;
                m_mapValidSignalCharacter[curBaseInfo].last().Info = curInfo;
                m_mapValidSignalCharacter[curBaseInfo].last().stopTime = QDateTime::currentMSecsSinceEpoch();
                break;
            }
        }
        //从未有过：增加map中的键值对，新增累计list
        if (!foundFlag)
        {
            DisplaySignalCharacter newSignalChar;
            newSignalChar.Info = curInfo;
            newSignalChar.startTime = QDateTime::currentMSecsSinceEpoch();
            newSignalChar.stopTime = newSignalChar.startTime;
            QList<DisplaySignalCharacter> newSignalList;
            newSignalList.append(newSignalChar);
            m_mapValidSignalCharacter.insert(curInfo.BaseInfo, newSignalList);
        }
    }
    //查找典型频率点周围的人为噪声信号特征
    if (findNoiseCharaAroundTypicalFreq(FFtin, length, Freqency, BandWidth))
        ret = 0;
    return ret;
}

void WBSignalDetectModel::SlotTriggerLegalFreqSet(bool checked)
{
    //仅供signalDetecttable使用
    if (m_eUserViewType != SIGNAL_DETECT_TABLE)
    {
        m_bIsSettingLegalFreqFlag = false;
        return;
    }
    m_bIsSettingLegalFreqFlag = checked;
    if (!checked)
    {
        //完成修改后根据已修改的m_DisplayData中的状态修改map中对应数据的值
        foreach (const auto& signalIndex, m_DisplayData)
        {
            if (signalIndex.length() >= 9)
            {
                //LZMK:反向从用于显示的m_DisplayData中获取用于查询map的key，存在一些问题 //TODO:当显示频点和带宽的数据的单位发生变化时需要同时变更
                //显示时使用单位为MHz
                int centerFreq = signalIndex.at(1).toDouble() * 1e6;
                int bandWidth = signalIndex.at(3).toDouble() * 1e6;
                SignalBaseChar curKey;
                curKey.Bound = bandWidth;
                curKey.CentFreq = centerFreq;
                foreach (const auto& keyOfMap, m_mapValidSignalCharacter.keys())
                {
                    if (keyOfMap.CentFreq == curKey.CentFreq && keyOfMap.Bound == curKey.Bound)
                    {
                        m_mapValidSignalCharacter[keyOfMap].first().isLegal = signalIndex.at(8).toBool();
                        break;
                    }
                }
            }
        }
    }
}

void WBSignalDetectModel::SlotCleanUp()
{
    m_mapValidSignalCharacter.clear(); //直接清理，不存在跨线程访问的问题
}

bool WBSignalDetectModel::SlotImportLegalFreqConf()
{
    QSettings legalSetting("legalFreq.ini",  QSettings::IniFormat);
    auto groups = legalSetting.childGroups();
    foreach (const auto& curGroup, groups)
    {
        legalSetting.beginGroup(curGroup);
        int centerFreq = legalSetting.value("centerFreq").toInt();
        int bandWidth = legalSetting.value("bandWidth").toInt();
        foreach (const auto& keyOfMap, m_mapValidSignalCharacter.keys())
        {
            if (keyOfMap.CentFreq == centerFreq && keyOfMap.Bound == bandWidth)
            {
                m_mapValidSignalCharacter[keyOfMap].first().isLegal = false;
                break;
            }
        }
        legalSetting.endGroup();
    }
    return true;
}

bool WBSignalDetectModel::SlotExportLegalFreqConf()
{
    QSettings legalSetting("legalFreq.ini",  QSettings::IniFormat);
    QString groupName = "IllegalFreqGroup";
    int groupIndex = 0;
    foreach (const auto& keyOfMap, m_mapValidSignalCharacter.keys())
    {
        if (m_mapValidSignalCharacter[keyOfMap].first().isLegal == false)
        {
            legalSetting.beginGroup(groupName + QString::number(groupIndex));
            legalSetting.setValue("centerFreq", m_mapValidSignalCharacter[keyOfMap].first().Info.BaseInfo.CentFreq);
            legalSetting.setValue("bandWidth", m_mapValidSignalCharacter[keyOfMap].first().Info.BaseInfo.Bound);
            legalSetting.endGroup();
            groupIndex += 1;
        }
    }
    return true;
}

bool WBSignalDetectModel::findPeakIteratively(Ipp32f * FFtAvg, int length, int Freqency, int BandWidth)
{
    //从左向右查找第一个峰值的位置
    int MaxAddr = 0;
    //LZMK:对原有逻辑进行改造，原有逻辑为根据对应区间获取最大值最高点作为信号peak；
    //当前采用门限进行限制，只要存在某一点比前后两个点的幅度大的情况，即可认为是一个尖峰
    Ipp32f FFtMax = m_fThreshold;
    for (int index = 1; index < length - 1; ++index)
    {
        if (FFtAvg[index] <= m_fThreshold || FFtAvg[index + 1] >= FFtAvg[index] || FFtAvg[index] <= FFtAvg[index - 1])
            continue;
        FFtMax = FFtAvg[index];
        MaxAddr = index;
        break;
    }
    //找不到最大点了就可以退出了
    if (MaxAddr == 0)
        return true;
    int LeftAddr = 0, RightAddr = length;
    //TODO: 是否需要处理两个波峰过于接近，导致没能在右侧找到6dB边界的情况？
    //处理出现未找到6dB右边界的情况且包络走势出现上扬的趋势时直接作为右边界
    for (int index = MaxAddr; index < length - 1; ++index)
    {
        RightAddr = index;
        if (FFtAvg[index + 1] > FFtAvg[index] || FFtMax - FFtAvg[index] > 6)
            break;
    }
    //按照最高峰位置反向搜索信号 寻找左边界
    for (int index = MaxAddr; index > 0; --index)
    {
        LeftAddr = index;//获取左边界
        if (FFtAvg[index - 1] > FFtAvg[index] || FFtMax - FFtAvg[index] > 6)
            break;
    }
    SignalInfoStr currentSignalInfo; //计算信号属性
    currentSignalInfo.BaseInfo.Bound = (RightAddr - LeftAddr) * (float)((float)BandWidth / (float)length);
    currentSignalInfo.BaseInfo.CentFreq = Freqency + ((RightAddr + LeftAddr)/2 - length / 2)*(float)((float)BandWidth / (float)length);
    currentSignalInfo.Amp = FFtMax;
    Ipp32f SignalPower, NoiseLeftPower, NoiseRightPower;
    ippsMean_32f(&FFtAvg[LeftAddr], RightAddr - LeftAddr + 1, &SignalPower, ippAlgHintFast); //获取信号平均功率
    ippsMean_32f(FFtAvg, LeftAddr, &NoiseLeftPower, ippAlgHintFast); //获取噪声平均功率
    ippsMean_32f(&FFtAvg[RightAddr], length - RightAddr, &NoiseRightPower, ippAlgHintFast); //获取噪声平均功率
    auto NoisePower = (NoiseRightPower + NoiseLeftPower) / 2;
    currentSignalInfo.Snr = SignalPower - NoisePower;
    m_lstSignalInfo.append(currentSignalInfo);

    //后续处理时全带宽也会缩短
    int notDealedBandWidth = BandWidth - currentSignalInfo.BaseInfo.Bound;
    int notDealedBandCenterFreq = Freqency + RightAddr * (float)BandWidth / (float)length;
    return findPeakIteratively(FFtAvg + (RightAddr + 1), length - (RightAddr + 1), notDealedBandCenterFreq, notDealedBandWidth);
}

bool WBSignalDetectModel::findPeakCyclically(Ipp32f * FFtAvg, int length, int Freqency, int BandWidth)
{
    //从左向右查找第一个峰值的位置
    Ipp32f SignalPower = 0;
    Ipp32f NoisePower = 0;
    Ipp32f NoiseLeftPower = 0;
    Ipp32f NoiseRightPower = 0;
    //LZMK:对原有逻辑进行改造，原有逻辑为根据对应区间获取最大值最高点作为信号peak；
    //当前采用门限进行限制，只要存在某一点比前后两个点的幅度大的情况，即可认为是一个尖峰
    for (int totalIndex = 0; totalIndex < length;)
    {
        Ipp32f FFtMax = m_fThreshold;
        int MaxAddr = 0;
        int LeftAddr = 0;
        int RightAddr = length;
        for (auto index = totalIndex + 1; index < length - 1; ++index)
        {
            if (FFtAvg[index] <= m_fThreshold || FFtAvg[index + 1] >= FFtAvg[index] || FFtAvg[index] <= FFtAvg[index - 1])
                continue;
            FFtMax = FFtAvg[index];
            MaxAddr = index;
            break;
        }
        //找不到最大点了就可以退出了
        if (MaxAddr == 0)
            return true;

        //TODO: 是否需要处理两个波峰过于接近，导致没能在右侧找到6dB边界的情况？
        //处理出现未找到6dB右边界的情况且包络走势出现上扬的趋势时直接作为右边界
        for (auto index = MaxAddr; index < length - 1; ++index)
        {
            RightAddr = index;
            if(FFtAvg[index + 1] > FFtAvg[index] || FFtMax - FFtAvg[index] > 6)
                break;
        }

        //按照最高峰位置反向搜索信号 寻找左边界
        for (auto index = MaxAddr; index > totalIndex; index--)
        {
            LeftAddr = index;//获取左边界
            if(FFtAvg[index - 1] > FFtAvg[index] || FFtMax - FFtAvg[index] > 6)
                break;
        }

        //计算信号属性
        SignalInfoStr currentSignalInfo;
        currentSignalInfo.BaseInfo.Bound = (RightAddr - LeftAddr)*(float)((float)BandWidth / (float)length);
        currentSignalInfo.BaseInfo.CentFreq = Freqency + ((RightAddr+ LeftAddr)/2 - length / 2) * (float)((float)BandWidth / (float)length);
        currentSignalInfo.Amp = FFtMax;
        //获取信号平均功率
        ippsMean_32f(&FFtAvg[LeftAddr], RightAddr - LeftAddr + 1, &SignalPower, ippAlgHintFast);
        //获取噪声平均功率
        ippsMean_32f(FFtAvg, LeftAddr, &NoiseLeftPower, ippAlgHintFast);
        ippsMean_32f(&FFtAvg[RightAddr], length - RightAddr, &NoiseRightPower, ippAlgHintFast);
        NoisePower = (NoiseRightPower + NoiseLeftPower) / 2;
        currentSignalInfo.Snr = SignalPower - NoisePower;
        m_lstSignalInfo.append(currentSignalInfo);
        //更新次回处理起点
        totalIndex = RightAddr + 1;
    }
    return true;
}

bool WBSignalDetectModel::getManMadeNoiseAmpInEveryTestFreqSpanFromFullSpan(Ipp32f *FFtAvg, int length, int Freqency, int BandWidth)
{
    QList<noiseAmp> noiseAmpLst;
    noiseAmp curNoiseAmp;

    int startFreq = Freqency - BandWidth / 2;
    foreach (const auto& typicalFreq, m_ManMadNoiseAnalyse.m_mapStoreAmpValueToGetManMadeNoiseValue.keys())
    {
        foreach(const auto& testFreq, m_ManMadNoiseAnalyse.m_mapStoreAmpValueToGetManMadeNoiseValue[typicalFreq].keys()){
            noiseAmpLst.clear();
            //每次计算间隔为左右0.04m
            int noiseInterval = ceil(double(0.04e6) / double(BandWidth) * length);
            int testFreqIndex = ceil(double(testFreq - startFreq) / double(BandWidth) * length);
            for (int index = testFreqIndex - ceil(double(noiseInterval) / 2); index < testFreqIndex + ceil(double(noiseInterval) / 2); ++index)
            {
                curNoiseAmp.freqPointPos = double(index) / double(length) * BandWidth + startFreq;
                curNoiseAmp.amp = FFtAvg[index];
                noiseAmpLst.append(curNoiseAmp);
            }

            //采用20%处理法，选取幅值电平由小到大前20%的信号的中位数(10%)位置的值，作为要找的目标频点的幅值，将其更新给记录人为噪声的map中的对应元素
            std::sort(noiseAmpLst.begin(), noiseAmpLst.end());

            int currentNoiseAmp = noiseAmpLst.at(floor(double(noiseAmpLst.length()) / 10)).amp;
            int existNoiseAmp = m_ManMadNoiseAnalyse.m_mapStoreAmpValueToGetManMadeNoiseValue[typicalFreq].value(testFreq);
            int currentNoiseAmpAvg = (existNoiseAmp * m_ManMadNoiseAnalyse.m_lGetAmpTimes + currentNoiseAmp) / (m_ManMadNoiseAnalyse.m_lGetAmpTimes + 1);
            m_ManMadNoiseAnalyse.m_mapStoreAmpValueToGetManMadeNoiseValue[typicalFreq][testFreq] = currentNoiseAmpAvg;
        }
    }
    m_ManMadNoiseAnalyse.m_lGetAmpTimes += 1;
    return true;
}

bool WBSignalDetectModel::findNoiseCharaAroundTypicalFreq(Ipp32f *FFtAvg, int length, int Freqency, int BandWidth)
{
    //1s执行一次
    qint64 nowtime = QDateTime::currentMSecsSinceEpoch();
    if (nowtime - m_iFindNoiseCharaTimeGap <= 1000)
        return true;
    m_iFindNoiseCharaTimeGap = nowtime;

    getManMadeNoiseAmpInEveryTestFreqSpanFromFullSpan(FFtAvg, length, Freqency, BandWidth);

    foreach (const auto& typicalFreq, m_ManMadNoiseAnalyse.m_mapStoreAmpValueToGetManMadeNoiseValue.keys())
    {
        ManMadeNoiseInfo curItem;
        QList<ManMadeNoiseInfo> lstStarter;
        foreach (const auto& testFreq, m_ManMadNoiseAnalyse.m_mapStoreAmpValueToGetManMadeNoiseValue[typicalFreq].keys()) {
            curItem.CentFreq = testFreq;
            curItem.Amp = m_ManMadNoiseAnalyse.m_mapStoreAmpValueToGetManMadeNoiseValue[typicalFreq][testFreq];       //获取当前的10s区间内的噪声电平
            curItem.startTime = m_i64CurrentDetectStartTime;              //当前噪声的起始时间使用开始分析的时间
            lstStarter.append(curItem);
        }
        if (!m_mapManMadeNoiseCharacter.contains(typicalFreq))          //初次处理
        {
            m_mapManMadeNoiseCharacter.insert(typicalFreq, lstStarter);
        }
        else
        {
            bool foundExistUnstopedTestFreq = false;
            //遍历当前典型频点下已有的list中的元素，将未设置终止时间的元素直接替换掉
            for(int index = 0; index < m_mapManMadeNoiseCharacter[typicalFreq].length(); ++index){
                for(int innerIndex = 0; innerIndex < lstStarter.length(); ++innerIndex){
                    if(lstStarter[innerIndex] == m_mapManMadeNoiseCharacter[typicalFreq][index] &&
                        m_mapManMadeNoiseCharacter[typicalFreq][index].stopTime == 0){
                        m_mapManMadeNoiseCharacter[typicalFreq][index] = lstStarter[innerIndex];
                        foundExistUnstopedTestFreq = true;
                        break;
                    }
                }
            }
            if(!foundExistUnstopedTestFreq)         //终止之后再开始，整体添加当前的lst
            {
                for(int index = 0; index < lstStarter.length(); ++index){
                    lstStarter[index].startTime = m_i64CurrentDetectStartTime;
                }
                m_mapManMadeNoiseCharacter[typicalFreq].append(lstStarter);
            }
        }
        //连续处理四个小时后，下次进入新一轮分析处理
        if(nowtime - m_mapManMadeNoiseCharacter[typicalFreq].constLast().startTime >= 4 * 3600 * 1000){
            for(int index = 0; index < m_mapManMadeNoiseCharacter[typicalFreq].length(); ++index){
                m_mapManMadeNoiseCharacter[typicalFreq][index].stopTime = nowtime;
            }
            m_ManMadNoiseAnalyse.m_lGetAmpTimes = 0;
            m_i64CurrentDetectStartTime = nowtime;
        }
    }
    return true;
}

void WBSignalDetectModel::reAlignValidSignalCharacterMap()
{

}

bool WBSignalDetectModel::bIsDetecting() const
{
    return m_bIsDetecting;
}

void WBSignalDetectModel::setMapTypicalFreqAndItsTestFreq(const QList<int>& lstValue)
{
    setLstTypicalFreq(lstValue);
}

QMap<int, int> WBSignalDetectModel::mapExistTypicalFreqNoiseRecordAmount() const
{
    QMap<int, int> ExistTypicalFreqNoiseRecordAmount;
    foreach (const int& freqPoint, m_mapManMadeNoiseCharacter.keys())
        ExistTypicalFreqNoiseRecordAmount.insert(freqPoint, m_mapManMadeNoiseCharacter[freqPoint].length());
    return ExistTypicalFreqNoiseRecordAmount;
}

QList<int> WBSignalDetectModel::lstTypicalFreq() const
{
    return m_ManMadNoiseAnalyse.m_mapStoreAmpValueToGetManMadeNoiseValue.keys();
}

void WBSignalDetectModel::setLstTypicalFreq(const QList<int> &newLstTypicalFreq)
{
    m_ManMadNoiseAnalyse.m_mapStoreAmpValueToGetManMadeNoiseValue.clear();
    m_ManMadNoiseAnalyse.m_lGetAmpTimes = 0;
    //通过初始化典型频点，计算当前典型频点及其各自的测试频点
    int currentTestFreq;
    foreach (const int& typicalFreq, newLstTypicalFreq){
        QMap<int, int> testFreqAndItsAmp;
        for(int index = 0; index < 10; ++index){
            currentTestFreq = typicalFreq - 0.2e6 + 0.02e6 + index * 0.04e6;
            if(currentTestFreq < 0 || currentTestFreq > 30e6){
                continue;
            }
            testFreqAndItsAmp.insert(currentTestFreq, 0);
        }
        if(testFreqAndItsAmp.keys().isEmpty()){
            continue;
        }
        m_ManMadNoiseAnalyse.m_mapStoreAmpValueToGetManMadeNoiseValue.insert(typicalFreq, testFreqAndItsAmp);
    }
}

void WBSignalDetectModel::setFThreshold(float newFThreshold)
{
    m_fThreshold = newFThreshold;
}

void WBSignalDetectModel::setFreqPointThreshold(uint newFreqPointThreshold)
{
    g_FreqPointThreshold = newFreqPointThreshold;
}

void WBSignalDetectModel::setActiveThreshold(uint newActiveThreshold)
{
    m_ActiveThreshold = newActiveThreshold;
}

void WBSignalDetectModel::setBandwidthThreshold(uint newBandwidthThreshold)
{
    g_BandwidthThreshold = newBandwidthThreshold;
}

MODEL_USER_VIEW WBSignalDetectModel::UserViewType() const
{
    return m_eUserViewType;
}

void WBSignalDetectModel::setUserViewType(MODEL_USER_VIEW newEUserViewType)
{
    m_eUserViewType = newEUserViewType;
}

void WBSignalDetectModel::UpdateData()
{
    if (m_bIsSettingLegalFreqFlag)
        return;
//    std::lock_guard<std::mutex> lk(m_mutex);
    beginResetModel();
    m_DisplayData.clear();
    if (m_eUserViewType == SIGNAL_DETECT_TABLE || m_eUserViewType == DISTURB_NOISE_TABLE)
    {
        //对检测信号map进行统计处理
        int i = 0;
        foreach (const auto& curSigBaseInfo, m_mapValidSignalCharacter.keys())
        {
            ++i;
            //根据当前用于显示的view类型进行区分
            if (m_eUserViewType == NOT_USED)
                break;
            QVector<QVariant> line;
            line.append(QString("%1").arg(i));
            if (m_eUserViewType == SIGNAL_DETECT_TABLE)
            {
                //LZMK: 此处将HZ转为MHZ显示有可能造成后面设置合法信号时无法反向找回到map中对应的那条数据，可能存在风险
                line.append(QString::number(double(curSigBaseInfo.CentFreq) / 1e6, 'f', 6));
                line.append(QString::number(m_mapValidSignalCharacter.value(curSigBaseInfo).constLast().Info.Amp + 107));        //电平，采用107算法
                line.append(QString::number(double(m_mapValidSignalCharacter.value(curSigBaseInfo).constLast().Info.BaseInfo.Bound) / 1e6, 'f', 6));        //带宽

                QDateTime time;
                if (m_mapValidSignalCharacter.value(curSigBaseInfo).constFirst().startTime == 0)
                    line.append("");
                else
                {
                    time.setMSecsSinceEpoch(m_mapValidSignalCharacter.value(curSigBaseInfo).constFirst().startTime);
                    line.append(time.toString("MM-dd hh:mm:ss")); //起始时间
                }

                if (m_mapValidSignalCharacter.value(curSigBaseInfo).constLast().stopTime == 0)
                    line.append("");
                else
                {
                    time.setMSecsSinceEpoch(m_mapValidSignalCharacter.value(curSigBaseInfo).constLast().stopTime);
                    line.append(time.toString("MM-dd hh:mm:ss")); //结束时间
                }

                if (m_iFullBandWidth <= 0 || m_iFullBandWidth < m_mapValidSignalCharacter.value(curSigBaseInfo).constLast().Info.BaseInfo.Bound)
                    line.append("");
                else
                    line.append(QString("%1%").arg(100 * double(m_mapValidSignalCharacter.value(curSigBaseInfo).constLast().Info.BaseInfo.Bound) / double(m_iFullBandWidth)));        //占用带宽

                qint64 duringTime = 0;
                qint64 nowTime = QDateTime::currentMSecsSinceEpoch();
                //计算一个确定频点带宽特征的信号在持续的总时间长度
                foreach (const auto& curSigInfoInList, m_mapValidSignalCharacter.value(curSigBaseInfo))
                {
                    if (curSigInfoInList.stopTime == 0)
                    {
                        duringTime += nowTime - curSigInfoInList.startTime;
                        break;
                    }
                    duringTime += curSigInfoInList.stopTime - curSigInfoInList.startTime;
                }
                //信号占用度
                if (m_i64SystemStopTime == m_i64SystemStartTime)
                    line.append(QString("%1%").arg(100 * double(duringTime) / double(nowTime - m_i64SystemStartTime)));
                else
                    line.append(QString("%1%").arg(100 * double(duringTime) / double(m_i64SystemStopTime - m_i64SystemStartTime)));
                //当前频点的信号是否合法记录在每个数据链的头部元素中
                line.append(m_mapValidSignalCharacter.value(curSigBaseInfo).constFirst().isLegal);
                m_DisplayData.append(line);
            }
            else
            {
                //干扰信号测量表格
                line.append(QString::number(double(curSigBaseInfo.CentFreq) / double(1e6), 'f', 6));
                line.append(QString::number(m_mapValidSignalCharacter.value(curSigBaseInfo).constLast().Info.Amp + 107));        //电平，采用107算法
                QDateTime time;
                if (m_mapValidSignalCharacter.value(curSigBaseInfo).constFirst().startTime == 0)
                    line.append("");
                else
                {
                    time.setMSecsSinceEpoch(m_mapValidSignalCharacter.value(curSigBaseInfo).constFirst().startTime);
                    line.append(time.toString("MM-dd hh:mm:ss")); //起始时间
                }

                if (m_mapValidSignalCharacter.value(curSigBaseInfo).constLast().stopTime == 0)
                    line.append("");
                else
                {
                    time.setMSecsSinceEpoch(m_mapValidSignalCharacter.value(curSigBaseInfo).constLast().stopTime);
                    line.append(time.toString("MM-dd hh:mm:ss")); //结束时间
                }
                if (!m_mapValidSignalCharacter.value(curSigBaseInfo).constFirst().isLegal)
                    m_DisplayData.append(line);
            }
        }
    }
    else if (m_eUserViewType == MAN_MADE_NOISE_TABLE)
    {
        //电磁环境人为噪声电平测量表格 //对人为噪声统计map进行统计处理
        foreach (const auto& curNoiseFreq, m_mapManMadeNoiseCharacter.keys())
        {
            foreach (const auto& restoredNoiseCharacter, m_mapManMadeNoiseCharacter.value(curNoiseFreq))
            {
                QVector<QVariant> line;
                line.append(QString::number(double(curNoiseFreq) / double(1e6), 'f', 6));
                line.append(QString::number(double(restoredNoiseCharacter.CentFreq) / double(1e6), 'f', 6));
                QDateTime time;
                if (restoredNoiseCharacter.startTime == 0)
                    line.append("");
                else
                {
                    time.setMSecsSinceEpoch(restoredNoiseCharacter.startTime);
                    line.append(time.toString("MM-dd hh:mm:ss")); //起始时间
                }

                if (restoredNoiseCharacter.stopTime == 0)
                    line.append("");
                else
                {
                    time.setMSecsSinceEpoch(restoredNoiseCharacter.stopTime);
                    line.append(time.toString("MM-dd hh:mm:ss")); //结束时间
                }
                line.append(QString::number(restoredNoiseCharacter.Amp + 107));
                m_DisplayData.append(line);
            }
        }
    }
    endResetModel();
}

void WBSignalDetectModel::SetStartTime()
{
    m_i64CurrentDetectStartTime = QDateTime::currentMSecsSinceEpoch();
    m_ManMadNoiseAnalyse.m_lGetAmpTimes = 0;
    m_bIsDetecting = true;
}

void WBSignalDetectModel::SetStopTime()
{
    m_bIsDetecting = false;
    m_i64SystemStopTime = QDateTime::currentMSecsSinceEpoch();
    //停止处理时更新人为噪声中包含记录的结束时间
    foreach (const auto& typicalFreq, m_mapManMadeNoiseCharacter.keys()) {
        for(int index = 0; index < m_mapManMadeNoiseCharacter[typicalFreq].length(); ++index){
            if(m_mapManMadeNoiseCharacter[typicalFreq][index].stopTime == 0){
                m_mapManMadeNoiseCharacter[typicalFreq][index].stopTime = m_i64SystemStopTime;
            }
        }
    }
}

bool SignalBaseChar::operator==(const SignalBaseChar &other) const
{
    return std::abs(CentFreq - other.CentFreq) < g_FreqPointThreshold && std::abs(Bound - other.Bound) < g_BandwidthThreshold;
}

bool SignalBaseChar::operator<(const SignalBaseChar &other) const
{
    return (std::abs(CentFreq - other.CentFreq) < g_FreqPointThreshold && Bound < other.Bound) || CentFreq < other.CentFreq;
}
