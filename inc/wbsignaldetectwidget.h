#ifndef WBSIGNALDETECTWIDGET_H
#define WBSIGNALDETECTWIDGET_H

#include <QWidget>

#include "../inc/PopupParamSet.h"
#include "../inc/TypicalFreqSetWidget.h"
#include "../inc/signaldetecttableview.h"
#include "../inc/manmadenoisetableview.h"
#include "../inc/disturbnoisetableview.h"
#include "../inc/wbsignaldetectmodel.h"

class WBSignalDetectWidget: public QWidget
{
    Q_OBJECT
public:
    explicit WBSignalDetectWidget(QWidget *parent = nullptr);
    //单次数据传入触发处理
    void sigTriggerSignalDetect(unsigned char *FFtin, //FFT输入数据
                                int InStep,         //平滑滑窗的宽度
                                int length,         //FFT的点数
                                int Freqency,       //中心频点
                                int BandWidth);     //当前FFT覆盖带宽
    //设置有效电平门限   dBm
    void sigSetValidAmpThreshold(float amp);

signals:
    //记录开始检测时间
    void startDetect();
    //记录最终完成检测时间
    void stopDetect();

private:
    void setupUi();
    bool turnToCorrectTableModel();
    WBSignalDetectModel* m_pGenericModel = nullptr;
    SignalDetectTableView* m_pSignalDetectTable = nullptr;
    DisturbNoiseTableView* m_pDisturbNoiseTable = nullptr;
    ManMadeNoiseTableView* m_pManMadeNoiseTable = nullptr;
    PopupParamSet* m_pPopupParamSet = nullptr;
    TypicalFreqSetWidget* m_pTypicalFreqSetWidget = nullptr;
    ParamSet m_DetectParam;

private:
    QPushButton *pushButton_ParamSet;
    QPushButton *pushButton_TypicalFreqSet;
    QPushButton *pushButton_importLegal;
    QPushButton *pushButton_ExportLegal;
    QPushButton *pushButton_cleanAllData;
    QPushButton *pushButton_setLegalFreq;
    QTabWidget *tabWidget_SignalDetectTable;
    QPushButton *pushButton_GenerateSignalDetect;
    QPushButton *pushButton_GenerateDisturbSIgnal;
    QPushButton *pushButton_GenerateManMadeNoise;
    QPushButton *pushButton_GenerateElecEnvReport;

private slots:
    //根据当前检测状态控制典型频点设置按钮的显隐
    void slotHideTypicalFreqSetButton();
    void slotShowTypicalFreqSetButton();
};

#endif // WBSIGNALDETECTWIDGET_H
