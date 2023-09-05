#ifndef WBSIGNALDETECTMODEL_H
#define WBSIGNALDETECTMODEL_H

#include <QObject>
#include <QMap>
#include <QList>
#include <QAbstractTableModel>
#include <QFont>

#include <mutex>

#include "ipp.h"
#include "ippcore.h"
#include "ippvm.h"
#include "ipps.h"

struct SignalBaseChar
{
    int CentFreq;       //HZ
    int Bound;          //HZ
    bool operator==(const SignalBaseChar& other) const;
    bool operator<(const SignalBaseChar &other) const;
};

typedef struct SignalInfo
{
    SignalBaseChar BaseInfo;
    float Snr;
    float CodeRate;
    float Amp;
} SignalInfoStr;

struct DisplaySignalCharacter
{
    SignalInfo Info;
    qint64 startTime = 0;       //时间戳   ms级  //TODO:根据实际需要调整
    qint64 stopTime = 0;
    bool isLegal = true;
};

struct ManMadeNoiseInfo{
    qint64 startTime = 0;
    qint64 stopTime = 0;
    float Amp;
    int CentFreq;             //HZ

    bool operator==(const ManMadeNoiseInfo& other) const{
        //频点
        if(CentFreq != other.CentFreq){
            return false;
        }
        //时间元素
        if(startTime != other.startTime ||
            stopTime != other.stopTime){
            return false;
        }
        return true;
    }
};


enum MODEL_USER_VIEW
{
    NOT_USED = 0,
    SIGNAL_DETECT_TABLE = 1,
    DISTURB_NOISE_TABLE = 2,
    MAN_MADE_NOISE_TABLE = 3
};

class noiseAmp
{
public:
    int freqPointPos = 0;
    int amp = 0;
    //仅用于判断当前幅度值的关系
    bool operator<(const noiseAmp& other) const
    {
        if (this->amp < other.amp)
            return true;
        else if (this->amp == other.amp)
            return this->freqPointPos < other.freqPointPos;
        return false;
    }
};

struct ManMadeNoiseAnalyse
{
    //典型频点-------测量频点0----测量电平0
    //           |
    //           |---测量频点1
    long m_lGetAmpTimes = 0;
    QMap<int, QMap<int, int>> m_mapStoreAmpValueToGetManMadeNoiseValue;  //记录当前典型频率点对应的10个测量频点，包含每次分析所得的电平幅值，每次都与前一次算均值
};

class WBSignalDetectModel: public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit WBSignalDetectModel(QObject *parent = nullptr);
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    MODEL_USER_VIEW UserViewType() const;
    void setUserViewType(MODEL_USER_VIEW newEUserViewType);
    void setBandwidthThreshold(uint newBandwidthThreshold);
    void setActiveThreshold(uint newActiveThreshold);
    void setFreqPointThreshold(uint newFreqPointThreshold);

    QList<int> lstTypicalFreq() const;
    void setLstTypicalFreq(const QList<int> &newLstTypicalFreq);
    QMap<int, int> mapExistTypicalFreqNoiseRecordAmount() const;

    bool bIsDetecting() const;

signals:
    void sigTriggerRefreshData();

public slots:
    //更新表格中数据，用户在选择当前信号是否合法时不进行界面更新
    void UpdateData();
    void SetStartTime();
    void SetStopTime();

    void setFThreshold(float newFThreshold);
    //InStep:平滑滑窗的宽度   length:FFT的点数 Freqency:输入中心频点 Bandwidth 当前FFT覆盖带宽
    int FindSignal(float *FFtin, int InStep, int length, int Freqency, int BandWidth);
    //控制进入合法频点设置阶段flag，正在修改合法频点属性时不更新界面元素，完成修改时更新map
    void SlotTriggerLegalFreqSet(bool checked);
    //清理当前map中全部数据，开始重新计算
    void SlotCleanUp();
    //管理合法频点设置
    bool SlotImportLegalFreqConf();
    bool SlotExportLegalFreqConf();
    void setMapTypicalFreqAndItsTestFreq(const QList<int> &lstValue);

private:
    bool findPeakIteratively(Ipp32f *FFtAvg, int length, int Freqency, int BandWidth);
    bool findPeakCyclically(Ipp32f *FFtAvg, int length, int Freqency, int BandWidth);
    bool getManMadeNoiseAmpInEveryTestFreqSpanFromFullSpan(Ipp32f *FFtAvg, int length, int Freqency, int BandWidth);
    //根据当前典型频点自动找出用于长时间累计记录的测量频点
    bool findNoiseCharaAroundTypicalFreq(Ipp32f *FFtAvg, int length, int Freqency, int BandWidth);
    QTimer* m_pSignalActiveChecker = nullptr;
    //当前采集到的信号列表
    QList<SignalInfo> m_lstSignalInfo;
    //积累下来信号 key: SignalBaseChar ，头部节点为首次识别到该频点时的记录，需频点与带宽都满足区分阈值内才能算作有效信号
    //TODO: 可能需要长时间使用，需要设置一个处理中间数据的行为，可以利用1s的定时器，定时清理map中的list的中段数据，仅保留头尾元素
    QMap<SignalBaseChar, QList<DisplaySignalCharacter>> m_mapValidSignalCharacter;
    //用于统计人为噪声（底噪）在各个频点时间内的表现形式 key: 典型频率点
    QMap<int, QList<ManMadeNoiseInfo>> m_mapManMadeNoiseCharacter;
    //TODO:用于完成单次信号分析处理后，重新计算中心频点key，修改key后会影响界面上的中心频点的显示
    void reAlignValidSignalCharacterMap();
    MODEL_USER_VIEW m_eUserViewType = NOT_USED;
    float m_fThreshold = -100;
    QList<QList<QVariant>> m_DisplayData;
    QFont m_Font;
    int m_iFullBandWidth;       //总带宽       HZ

    qint64 m_i64SystemStartTime = 0;                //系统启动时的时间，软件启动时开始计算，用于计算信号占用率
    qint64 m_i64SystemStopTime = 0;                 //系统/信号分析 停止时的时间，用于计算信号占用率，也作为当前处理的停止时间
    qint64 m_i64CurrentDetectStartTime = 0;         //当前信号处理的开始时间

    uint m_ActiveThreshold = 10;         //单位为s
    bool m_bIsSettingLegalFreqFlag = false;

    ManMadeNoiseAnalyse m_ManMadNoiseAnalyse;
    qint64 m_iFindNoiseCharaTimeGap = 0;
    std::mutex m_mutex;
    bool m_bIsDetecting = false;            //用于记录当前是否正在检测信号

};

#endif // WBSIGNALDETECTMODEL_H
