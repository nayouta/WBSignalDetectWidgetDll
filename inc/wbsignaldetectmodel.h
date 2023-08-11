#ifndef WBSIGNALDETECTMODEL_H
#define WBSIGNALDETECTMODEL_H

#include <QObject>
#include <QMap>
#include <QList>
#include <QAbstractTableModel>
#include <QFont>

#include "ipp.h"
#include "ippcore.h"
#include "ippvm.h"
#include "ipps.h"

class QTimer;

struct SignalBaseChar{
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
}SignalInfoStr;

struct DisplaySignalCharacter{
    SignalInfo Info;
    qint64 startTime = 0;       //时间戳   ms级  //TODO:根据实际需要调整
    qint64 stopTime = 0;
    bool isLegal = true;
};

enum MODEL_USER_VIEW{
    NOT_USED = 0,
    SIGNAL_DETECT_TABLE = 1,
    DISTURB_NOISE_TABLE = 2,
    MAN_MADE_NOISE_TABLE = 3
};

class noiseAmp{
public:
    int freqPointPos;
    int amp;
    //仅用于判断当前幅度值的关系
    bool operator<(const noiseAmp& other) const{
        if(this->amp < other.amp){
            return true;
        }else if(this->amp == other.amp){
            return this->freqPointPos < other.freqPointPos;
        }
        return false;
    }
};

class WBSignalDetectModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit WBSignalDetectModel(QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    MODEL_USER_VIEW UserViewType() const;
    void setUserViewType(MODEL_USER_VIEW newEUserViewType);

    void setBandwidthThreshold(uint newBandwidthThreshold);

    void setActiveThreshold(uint newActiveThreshold);

    void setFreqPointThreshold(uint newFreqPointThreshold);

    QList<int> lstTypicalFreq() const;
    void setLstTypicalFreq(const QList<int> &newLstTypicalFreq);

    QMap<int, int> mapExistTypicalFreqNoiseRecordAmount() const;

    QMap<int, int> mapTypicalFreqAndItsTestFreq() const;

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
    void setMapTypicalFreqAndItsTestFreq(const QMap<int, int> &newMapTypicalFreqAndItsTestFreq);

private:
    bool findPeakIteratively(Ipp32f *FFtAvg, int length, int Freqency, int BandWidth);
    bool findPeakCyclically(Ipp32f *FFtAvg, int length, int Freqency, int BandWidth);
    bool findManMadeNoiseFreqAutolly(Ipp32f *FFtAvg, int length, int Freqency, int BandWidth);
    //根据当前典型频点自动找出用于长时间累计记录的测量频点
    bool findNoiseCharaAroundTypicalFreq(Ipp32f *FFtAvg, int length, int Freqency, int BandWidth);

    QTimer* m_pSignalActiveChecker = nullptr;

    //当前采集到的信号列表
    QList<SignalInfo> m_lstSignalInfo;
    //积累下来信号 key: SignalBaseChar ，头部节点为首次识别到该频点时的记录，需频点与带宽都满足区分阈值内才能算作有效信号
    //TODO: 可能需要长时间使用，需要设置一个处理中间数据的行为，可以利用1s的定时器，定时清理map中的list的中段数据，仅保留头尾元素
    QMap<SignalBaseChar, QList<DisplaySignalCharacter>> m_mapValidSignalCharacter;
    //用于统计人为噪声（底噪）在各个频点时间内的表现形式 key: 典型频率点
    QMap<int, QList<DisplaySignalCharacter>> m_mapManMadeNoiseCharacter;

    //TODO:用于完成单次信号分析处理后，重新计算中心频点key，修改key后会影响界面上的中心频点的显示
    void reAlignValidSignalCharacterMap();

    MODEL_USER_VIEW m_eUserViewType = NOT_USED;

    float m_fThreshold = -100;

    QList<QList<QString>> m_DisplayData;

    QFont m_Font;

    int m_iFullBandWidth;       //总带宽       HZ

    qint64 m_i64SystemStartTime = 0;

    qint64 m_i64SystemStopTime = 0;

    uint m_ActiveThreshold = 0;         //单位为s

    bool m_bIsSettingLegalFreqFlag = false;

    QMap<int, int> m_mapTypicalFreqAndItsTestFreq;        //记录当前典型频率点及其附近找到的测试频点的对应关系       //默认测试频点未成功设置的情况下其值为0

    QMap<int, QList<int>> m_mapStoreAmpValueToGetManMadeNoiseValue;  //暂存当前典型频率点附近找到的测试频点的幅值

    int m_iCheckBandAroundTypicalFreq = 2e6;    //默认检测典型频率点周围2m范围内信号的噪声特性

    qint64 m_iFindNoiseCharaTimeGap = 0;

    bool m_bManuallySetManMadeNoiseFreq = false;    //手动设置人为噪声测量频点标志位


private slots:
    void slotCheckSignalActive();

};

#endif // WBSIGNALDETECTMODEL_H
