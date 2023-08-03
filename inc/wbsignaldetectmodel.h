#ifndef WBSIGNALDETECTMODEL_H
#define WBSIGNALDETECTMODEL_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <QList>
#include <QAbstractTableModel>

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
    qint64 startTime;
    qint64 stopTime;
};

class WBSignalDetectModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit WBSignalDetectModel(QObject *parent = nullptr);
    //InStep:平滑滑窗的宽度   length:FFT阶数（也就是FFT的点数） Freqency:输入中心频点 Bandwidth 当前FFT覆盖带宽
    int FindSignal(float *FFtin, int InStep, int length, int Freqency, int BandWidth);

    //返回行数
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    //返回列数
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    //根据模型索引返回当前的数据
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    int SampleDownFromInBuf(short* inBuf, int len, short * outBuf,int factor);

    bool findPeakIteratively(Ipp32f *FFtAvg, int length, int Freqency, int BandWidth);
    bool findPeakCyclically(Ipp32f *FFtAvg, int length, int Freqency, int BandWidth);
    //key: freq
    QList<SignalInfo> m_lstSignalInfo;
    QMap<int, DisplaySignalCharacter> m_mapActiveSignalCharacter;
    QList<DisplaySignalCharacter> m_lstFinishedSignalCharacter;

    float m_fThreshold = -100;
};

#endif // WBSIGNALDETECTMODEL_H
