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
    int CentFreq;
    int Bound;
    float Snr;
    float CodeRate;
}SignalInfoStr;

struct DisplaySignalCharacter{
    SignalInfo Info;
    qint64 startTime;       //时间戳   ms级  //TODO:根据实际需要调整
    qint64 stopTime;
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

signals:
    //InStep:平滑滑窗的宽度   length:FFT阶数（也就是FFT的点数） Freqency:输入中心频点 Bandwidth 当前FFT覆盖带宽
    void sigTriggerUpdateData(float *FFtin, int InStep, int length, int Freqency, int BandWidth);
    void sigTriggerRefreshData();

public slots:
    //更新表格中数据
    void UpdateData();

private slots:
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
};

#endif // WBSIGNALDETECTMODEL_H
