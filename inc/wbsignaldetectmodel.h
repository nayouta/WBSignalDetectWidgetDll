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

private:
    bool findPeakIteratively(Ipp32f *FFtAvg, int length, int Freqency, int BandWidth);
    bool findPeakCyclically(Ipp32f *FFtAvg, int length, int Freqency, int BandWidth);

    QTimer* m_pSignalActiveChecker = nullptr;

    //当前采集到的信号列表
    QList<SignalInfo> m_lstSignalInfo;
    //积累下来信号 key: SignalBaseChar ，头部节点为首次识别到该频点时的记录，需频点与带宽都满足区分阈值内才能算作有效信号
    QMap<SignalBaseChar, QList<DisplaySignalCharacter>> m_mapValidSignalCharacter;
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

private slots:
    void slotCheckSignalActive();

};

#endif // WBSIGNALDETECTMODEL_H
