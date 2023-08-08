#include "../inc/wbsignaldetectmodel.h"
#include <QDateTime>
#include <QTimer>

//考虑使用全局量记录频点识别门限以及带宽识别门限
uint g_FreqPointThreshold = 0;      //单位为Hz
uint g_BandwidthThreshold = 0;      //单位为Hz

WBSignalDetectModel::WBSignalDetectModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    m_Font.setFamily("Microsoft Yahei");
    //TODO:根据实际显示结果调整
    m_Font.setPixelSize(17);
    connect(this, &WBSignalDetectModel::sigTriggerRefreshData, this, &WBSignalDetectModel::UpdateData);

    m_pSignalActiveChecker = new QTimer(this);
    connect(m_pSignalActiveChecker, &QTimer::timeout, this, &WBSignalDetectModel::slotCheckSignalActive);
}

QVariant WBSignalDetectModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();
    if(m_eUserViewType == NOT_USED){
        return QVariant();
    }
    if (orientation == Qt::Horizontal) {
        if(m_eUserViewType == SIGNAL_DETECT_TABLE){
            switch (section) {
            case 0 :
                return "序号";
            case 1 :
                return "中心频率";
            case 2 :
                return "电平";
            case 3 :
                return "带宽";
            case 4 :
                return "起始时间";
            case 5 :
                return "结束时间";
            case 6 :
                return "占用带宽";
            case 7 :
                return "信号占用度";
            case 8 :
                return "合法信号";
            default :
                return "";
            }
        }else if(m_eUserViewType == DISTURB_NOISE_TABLE){
            switch (section) {
            case 0 :
                return "序号";
            case 1 :
                return "中心频率";
            case 2 :
                return "大信号电平";
            case 3 :
                return "测量时间";
            default :
                return "";
            }

        }else if(m_eUserViewType == MAN_MADE_NOISE_TABLE){
            switch (section) {
            case 0 :
                return "序号";
            case 1 :
                return "测量频率";
            case 2 :
                return "时间";
            case 3 :
                return "测量电平";
            case 4 :
                return "平均电平";
            case 5 :
                return "最大电平";
            case 6 :
                return "最小电平";
            case 7 :
                return "检波方式";
            case 8 :
                return "中频带宽";
            default :
                return "";
            }

        }
    } else {
        return QString("%1").arg(section + 1);
    }
    return QVariant();
}

int WBSignalDetectModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    //当前总列数为存活信号与完成信号的总长度
    //TODO:其他场景情况根据显示需要增加逻辑，如仅显示存活信号或仅显示已完成信号等，或者过多长时间后在删除对应记录等
    return m_mapValidSignalCharacter.keys().length();
}

int WBSignalDetectModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    //三个表格同时只能显示其中一个，根据当前使用者表格类型进行显示
    //TODO:后续可能需要三个表格同时显示，则采用对col进行hide的方式实现
    if(m_eUserViewType == NOT_USED){
        return 0;
    }
    if(m_eUserViewType == SIGNAL_DETECT_TABLE){
        return 9;
    }
    if(m_eUserViewType == DISTURB_NOISE_TABLE){
        return 4;
    }
    if(m_eUserViewType == MAN_MADE_NOISE_TABLE){
        return 9;
    }
    return 0;
}

bool WBSignalDetectModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (index.isValid() && role == Qt::EditRole) {
        QStringList &rowData = m_DisplayData[index.row()];
        rowData[index.column()] = value.toString();
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

Qt::ItemFlags WBSignalDetectModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return Qt::NoItemFlags;

    Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;

    if (index.column() == 8 && m_eUserViewType == SIGNAL_DETECT_TABLE)      //仅信号选择表格的最后一列可编辑
        flags |= Qt::ItemIsEditable;

    return flags;
}


QVariant WBSignalDetectModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if(m_eUserViewType == NOT_USED){
        return QVariant();
    }
    if (role == Qt::DisplayRole || role == Qt::EditRole) {              //显示内容
        return m_DisplayData[index.row()].at(index.column());
    } else if (role == Qt::TextAlignmentRole) {   //内容排版
        return Qt::AlignCenter;
    } else if (role == Qt::FontRole) {           //字体
        return m_Font;
    } else if(role == Qt::UserRole){
        return QVariant::fromValue(m_DisplayData[index.row()].at(index.column()));
    }

    //TODO:显示自定义实现方法
    return QVariant();
}

int WBSignalDetectModel::FindSignal(float *FFtin, int InStep, int length, int Freqency, int BandWidth)
{
    if(m_i64SystemStartTime == 0){
        m_i64SystemStartTime = QDateTime::currentMSecsSinceEpoch();
        m_i64SystemStopTime = m_i64SystemStartTime;
    }

    if(!m_pSignalActiveChecker->isActive()){
        m_pSignalActiveChecker->start(1000);
    }

    Ipp32f * FFtAvg = ippsMalloc_32f(length);
    m_iFullBandWidth = BandWidth;
    //暂时跳过进行fft平滑的步骤
//    int AvgCnt = 0;
//    //获取活滑动Step个点的平均FFt波形
//    for (int i = 0; i < length; i++)
//    {
//        AvgCnt = (i - InStep / 2) >= 0 ? 32 : (i + InStep / 2);
//        if (i <= InStep / 2)
//        {
//            AvgCnt = i + InStep / 2;
//            ippsMean_32f(FFtin, AvgCnt, &FFtAvg[i], ippAlgHintNone);
//        }
//        else if((length - i) <= InStep)
//        {
//            AvgCnt = length - i;
//            ippsMean_32f(&FFtin[i], AvgCnt, &FFtAvg[i], ippAlgHintNone);
//        }
//        else
//        {
//            ippsMean_32f(&FFtin[i], InStep, &FFtAvg[i], ippAlgHintNone);
//        }
//    }
//    //fprint32f("FFtIn.txt", FFtin, length);
    //TODO: 状态错误时的处理方法
    //记录本次检测到的大功率信号 list
    int ret = 1;
    m_lstSignalInfo.clear();
//    findPeakCyclically(FFtAvg, length, Freqency, BandWidth);
    findPeakCyclically(FFtin, length, Freqency, BandWidth);
    if(!m_lstSignalInfo.isEmpty()){
        ret = 0;
    }

    bool foundFlag = false;
    //三种情况：当前信号 1、从未有过  2、一直都有  3、曾有现停
    foreach (const SignalInfo& curInfo, m_lstSignalInfo) {
        DisplaySignalCharacter newSignalChar;
        foreach (const SignalBaseChar& curBaseInfo, m_mapValidSignalCharacter.keys()) {
            if(curInfo.BaseInfo == curBaseInfo){     //曾有现停，增加累计list的末尾节点，记录新的起始时间
                foundFlag = true;
                if(m_mapValidSignalCharacter[curBaseInfo].constLast().stopTime != 0){        //已经停止过一次
                    newSignalChar.Info = curInfo;
                    newSignalChar.startTime = QDateTime::currentMSecsSinceEpoch();
                    m_mapValidSignalCharacter[curBaseInfo].append(newSignalChar);
                }
            }else{}      //一直都有，不做处理
            break;
        }
        //从未有过：增加map中的键值对，新增累计list
        if(!foundFlag){
            newSignalChar.Info = curInfo;
            newSignalChar.startTime = QDateTime::currentMSecsSinceEpoch();
            QList<DisplaySignalCharacter> newSignalList;
            newSignalList.append(newSignalChar);
            m_mapValidSignalCharacter.insert(curInfo.BaseInfo, newSignalList);
        }
    }
    ippsFree(FFtAvg);
    UpdateData();
    return ret;
}

void WBSignalDetectModel::SlotTriggerLegalFreqSet(bool checked)
{
    //仅供signalDetecttable使用
    if(m_eUserViewType != SIGNAL_DETECT_TABLE){
        m_bIsSettingLegalFreqFlag = false;
        return;
    }

    m_bIsSettingLegalFreqFlag = checked;
    if(!checked){
        //完成修改后根据已修改的m_DisplayData中的状态修改map中对应数据的值
        foreach (const auto& signalIndex, m_DisplayData) {
            if(signalIndex.length() >= 9 && signalIndex.at(8) == "否"){
                //LZMK:反向从用于显示的m_DisplayData中获取用于查询map的key，存在一些问题
                //TODO:当显示频点和带宽的数据的单位发生变化时需要同时变更
                int centerFreq = signalIndex.at(1).toInt();
                int bandWidth = signalIndex.at(3).toInt();
                SignalBaseChar curKey;
                curKey.Bound = bandWidth;
                curKey.CentFreq = centerFreq;
                foreach (const auto& keyOfMap, m_mapValidSignalCharacter.keys()) {
                    if(keyOfMap.CentFreq == curKey.CentFreq && keyOfMap.Bound == curKey.Bound){
                        m_mapValidSignalCharacter[keyOfMap].first().isLegal = false;
                    }
                }
            }
        }
    }
}

bool WBSignalDetectModel::findPeakIteratively(Ipp32f * FFtAvg, int length, int Freqency, int BandWidth)
{
    //从左向右查找第一个峰值的位置
    Ipp32f SignalPower = 0;
    Ipp32f NoisePower = 0;
    Ipp32f NoiseLeftPower = 0;
    Ipp32f NoiseRightPower = 0;
    Ipp32f FFtMax = m_fThreshold;
    int MaxAddr = 0;
    int LeftAddr = 0;
    int RightAddr = length;

    //LZMK:对原有逻辑进行改造，原有逻辑为根据对应区间获取最大值最高点作为信号peak；
    //当前采用门限进行限制，只要存在某一点比前后两个点的幅度大的情况，即可认为是一个尖峰
    for(int index = 1; index < length - 1; ++index){
        if(FFtAvg[index] <= m_fThreshold){
            continue;
        }
        if(FFtAvg[index + 1] >= FFtAvg[index] || FFtAvg[index] <= FFtAvg[index - 1]){
            continue;
        }
        FFtMax = FFtAvg[index];
        MaxAddr = index;
        break;
    }

    //找不到最大点了就可以退出了
    if(MaxAddr == 0){
        return true;
    }

    //TODO: 是否需要处理两个波峰过于接近，导致没能在右侧找到6dB边界的情况？
    //处理出现未找到6dB右边界的情况且包络走势出现上扬的趋势时直接作为右边界
    for(int index = MaxAddr; index < length - 1; ++index){
        RightAddr = index;
        if(FFtAvg[index + 1] > FFtAvg[index]){
            break;
        }
        if(FFtMax - FFtAvg[index] > 6){
            break;
        }
    }

    //按照最高峰位置反向搜索信号 寻找左边界
    for (int index = MaxAddr; index > 0; index--)
    {
        LeftAddr = index;//获取左边界
        if(FFtAvg[index - 1] > FFtAvg[index]){
            break;
        }
        if (FFtMax - FFtAvg[index] > 6)//6dB右边界
        {
            break;
        }
    }

    //计算信号属性
    SignalInfoStr currentSignalInfo;
    currentSignalInfo.BaseInfo.Bound = (RightAddr - LeftAddr)*(float)((float)BandWidth / (float)length);
    currentSignalInfo.BaseInfo.CentFreq = Freqency + ((RightAddr+ LeftAddr)/2 - length / 2)*(float)((float)BandWidth / (float)length);
    currentSignalInfo.Amp = FFtMax;
    //获取信号平均功率
    ippsMean_32f(&FFtAvg[LeftAddr], RightAddr - LeftAddr + 1, &SignalPower, ippAlgHintFast);
    //获取噪声平均功率
    ippsMean_32f(FFtAvg, LeftAddr, &NoiseLeftPower, ippAlgHintFast);
    ippsMean_32f(&FFtAvg[RightAddr], length - RightAddr, &NoiseRightPower, ippAlgHintFast);
    NoisePower = (NoiseRightPower + NoiseLeftPower) / 2;
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
    for(int totalIndex = 0; totalIndex < length;){
        Ipp32f FFtMax = m_fThreshold;
        int MaxAddr = 0;
        int LeftAddr = 0;
        int RightAddr = length;
        for(int index = totalIndex + 1; index < length - 1; ++index){
            if(FFtAvg[index] <= m_fThreshold){
                continue;
            }
            if(FFtAvg[index + 1] >= FFtAvg[index] || FFtAvg[index] <= FFtAvg[index - 1]){
                continue;
            }
            FFtMax = FFtAvg[index];
            MaxAddr = index;
            break;
        }

        //找不到最大点了就可以退出了
        if(MaxAddr == 0){
            return true;
        }

        //TODO: 是否需要处理两个波峰过于接近，导致没能在右侧找到6dB边界的情况？
        //处理出现未找到6dB右边界的情况且包络走势出现上扬的趋势时直接作为右边界
        for(int index = MaxAddr; index < length - 1; ++index){
            RightAddr = index;
            if(FFtAvg[index + 1] > FFtAvg[index]){
                break;
            }
            if(FFtMax - FFtAvg[index] > 6){
                break;
            }
        }

        //按照最高峰位置反向搜索信号 寻找左边界
        for (int index = MaxAddr; index > totalIndex; index--)
        {
            LeftAddr = index;//获取左边界
            if(FFtAvg[index - 1] > FFtAvg[index]){
                break;
            }
            if (FFtMax - FFtAvg[index] > 6)//6dB右边界
            {
                break;
            }
        }

        //计算信号属性
        SignalInfoStr currentSignalInfo;
        currentSignalInfo.BaseInfo.Bound = (RightAddr - LeftAddr)*(float)((float)BandWidth / (float)length);
        currentSignalInfo.BaseInfo.CentFreq = Freqency + ((RightAddr+ LeftAddr)/2 - length / 2)*(float)((float)BandWidth / (float)length);
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

void WBSignalDetectModel::reAlignValidSignalCharacterMap()
{

}

void WBSignalDetectModel::slotCheckSignalActive()
{
    qint64 nowTime = QDateTime::currentMSecsSinceEpoch();
    foreach (const auto& existKey, m_mapValidSignalCharacter.keys()) {
        if(m_mapValidSignalCharacter[existKey].constLast().stopTime == 0 &&
            nowTime - m_mapValidSignalCharacter[existKey].constLast().stopTime >= m_ActiveThreshold * 1000){
            m_mapValidSignalCharacter[existKey].last().stopTime = nowTime;
        }
    }
    UpdateData();
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
    if(m_bIsSettingLegalFreqFlag){
        return;
    }

    beginResetModel();
    m_DisplayData.clear();

    //重置model中的数据
    int i = 0;
    foreach (const auto& curSigBaseInfo, m_mapValidSignalCharacter.keys()) {
        ++i;
        //根据当前用于显示的view类型进行区分
        if(m_eUserViewType == NOT_USED){
            break;
        }
        QVector<QString> line;
        line.append(QString("%1").arg(i));
        line.append(QString::number(curSigBaseInfo.CentFreq));
        if(m_eUserViewType == SIGNAL_DETECT_TABLE){
            //line.append(QString::number(m_mapValidSignalCharacter.value(curSigBaseInfo).last().Info));
            line.append(QString::number(m_mapValidSignalCharacter.value(curSigBaseInfo).constLast().Info.Amp + 107));        //电平，采用107算法
            line.append(QString::number(m_mapValidSignalCharacter.value(curSigBaseInfo).constLast().Info.BaseInfo.Bound));        //带宽

            QDateTime time;
            if(m_mapValidSignalCharacter.value(curSigBaseInfo).constFirst().startTime == 0){
                line.append("");
            }else{
                time.setMSecsSinceEpoch(m_mapValidSignalCharacter.value(curSigBaseInfo).constFirst().startTime);
                line.append(time.toString("MM-dd hh:mm:ss"));          //起始时间      //仅显示到s
            }
            if(m_mapValidSignalCharacter.value(curSigBaseInfo).constLast().stopTime == 0){
                line.append("");
            }else{
                time.setMSecsSinceEpoch(m_mapValidSignalCharacter.value(curSigBaseInfo).constLast().stopTime);
                line.append(time.toString("MM-dd hh:mm:ss"));      //结束时间          //仅显示到s
            }

            if(m_iFullBandWidth <= 0 || m_iFullBandWidth < m_mapValidSignalCharacter.value(curSigBaseInfo).constLast().Info.BaseInfo.Bound){
                line.append("");
            }else{
                line.append(QString("%1%").arg(100 * double(m_mapValidSignalCharacter.value(curSigBaseInfo).constLast().Info.BaseInfo.Bound) / double(m_iFullBandWidth)));        //占用带宽
            }

            qint64 duringTime = 0;
            qint64 nowTime = QDateTime::currentMSecsSinceEpoch();
            //计算一个确定频点带宽特征的信号在持续的总时间长度
            foreach (const auto& curSigInfoInList, m_mapValidSignalCharacter.value(curSigBaseInfo)) {
                if(curSigInfoInList.stopTime == 0){
                    duringTime += nowTime - curSigInfoInList.startTime;
                    break;
                }
                duringTime += curSigInfoInList.stopTime - curSigInfoInList.startTime;
            }
            //信号占用度
            if(m_i64SystemStopTime == m_i64SystemStartTime){
                line.append(QString("%1%").arg(100 * double(duringTime) / double(nowTime - m_i64SystemStartTime)));
            }else{
                line.append(QString("%1%").arg(100 * double(duringTime) / double(m_i64SystemStopTime - m_i64SystemStartTime)));
            }

            //当前频点的信号是否合法记录在每个数据链的头部元素中
            line.append(m_mapValidSignalCharacter.value(curSigBaseInfo).constFirst().isLegal ? QString("是") : QString("否"));
        }else if(m_eUserViewType == DISTURB_NOISE_TABLE){
            //TODO:干扰信号测量表格
            line.append(QString::number(m_mapValidSignalCharacter.value(curSigBaseInfo).constLast().Info.Amp + 107));        //电平，采用107算法
            line.append("");
        }else if(m_eUserViewType == MAN_MADE_NOISE_TABLE){
            //TODO:电磁环境认为噪声电平测量表格
            line.append("");
            line.append("");
            line.append("");
            line.append("");
            line.append("");
            line.append("");
            line.append("");
        }
        m_DisplayData.append(line);
    }
    endResetModel();
}

void WBSignalDetectModel::SetStartTime()
{
    m_i64SystemStartTime = QDateTime::currentMSecsSinceEpoch();
    m_i64SystemStopTime = m_i64SystemStartTime;
}

void WBSignalDetectModel::SetStopTime()
{
    m_i64SystemStopTime = QDateTime::currentMSecsSinceEpoch();
}

bool SignalBaseChar::operator==(const SignalBaseChar &other) const{
    return (qAbs(this->CentFreq - other.CentFreq) < g_FreqPointThreshold) && (qAbs(this->Bound - other.Bound) < g_BandwidthThreshold);
}

bool SignalBaseChar::operator<(const SignalBaseChar &other) const{
    if(qAbs(this->CentFreq - other.CentFreq) < g_FreqPointThreshold && qAbs(this->Bound - other.Bound) < g_BandwidthThreshold){
        return false;
    }

    if(qAbs(this->CentFreq - other.CentFreq) < g_FreqPointThreshold){       //两值中心频点约等
        if((this->Bound < other.Bound)){
            return true;
        }
    }else{
        //优先判断频点，然后判断带宽
        if((this->CentFreq < other.CentFreq)){
            return true;
        }
    }
    return false;
}
