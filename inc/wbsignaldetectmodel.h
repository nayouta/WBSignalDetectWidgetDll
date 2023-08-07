#ifndef WBSIGNALDETECTMODEL_H
#define WBSIGNALDETECTMODEL_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <QList>
#include <QAbstractTableModel>
#include <QFont>

#include "ipp.h"
#include "ippcore.h"
#include "ippvm.h"
#include "ipps.h"

typedef struct SignalInfo
{
    int CentFreq;       //HZ
    int Bound;          //HZ
    float Snr;
    float CodeRate;
    float Amp;
}SignalInfoStr;

struct DisplaySignalCharacter{
    SignalInfo Info;
    qint64 startTime = 0;       //时间戳   ms级  //TODO:根据实际需要调整
    qint64 stopTime = 0;
};

enum MODEL_USER_VIEW{
    NOT_USED = 0,
    SIGNAL_DETECT_TABLE = 1,
    DISTURB_NOISE_TABLE = 2,
    MAN_MADE_NOISE_TABLE = 3
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

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;


    MODEL_USER_VIEW UserViewType() const;
    void setUserViewType(MODEL_USER_VIEW newEUserViewType);

    void setBandwidthThreshold(uint newBandwidthThreshold);

    void setActiveThreshold(uint newActiveThreshold);

    void setFreqPointThreshold(uint newFreqPointThreshold);

signals:
    void sigTriggerRefreshData();

public slots:
    //更新表格中数据
    void UpdateData();
    void SetStartTime();
    void SetStopTime();

    void setFThreshold(float newFThreshold);
    //InStep:平滑滑窗的宽度   length:FFT的点数 Freqency:输入中心频点 Bandwidth 当前FFT覆盖带宽
    int FindSignal(float *FFtin, int InStep, int length, int Freqency, int BandWidth);

private:
    int SampleDownFromInBuf(short* inBuf, int len, short * outBuf,int factor);

    bool findPeakIteratively(Ipp32f *FFtAvg, int length, int Freqency, int BandWidth);
    bool findPeakCyclically(Ipp32f *FFtAvg, int length, int Freqency, int BandWidth);

    //当前采集到的信号列表
    QList<SignalInfo> m_lstSignalInfo;
    //积累下来的存活信号 key: freq
    QMap<int, DisplaySignalCharacter> m_mapActiveSignalCharacter;
    //积累下来的完成信号
    QList<DisplaySignalCharacter> m_lstFinishedSignalCharacter;

    MODEL_USER_VIEW m_eUserViewType = NOT_USED;

    float m_fThreshold = -100;

    QList<QVector<QString>> m_DisplayData;

    QFont m_Font;

    int m_iFullBandWidth;       //总带宽       HZ

    qint64 m_i64SystemStartTime = 0;

    qint64 m_i64SystemStopTime = 0;

    uint m_FreqPointThreshold = 0;
    uint m_BandwidthThreshold = 0;
    uint m_ActiveThreshold = 0;
};

#endif // WBSIGNALDETECTMODEL_H
