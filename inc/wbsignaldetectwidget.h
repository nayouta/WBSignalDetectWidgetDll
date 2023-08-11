#ifndef WBSIGNALDETECTWIDGET_H
#define WBSIGNALDETECTWIDGET_H

#include <QWidget>

class WBSignalDetectModel;
class SignalDetectTableView;
class ManMadeNoiseTableView;
class DisturbNoiseTableView;
class TypicalFreqSetWidget;

#include <popupparamset.h>


namespace Ui {
class WBSignalDetectWidget;
}

class WBSignalDetectWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WBSignalDetectWidget(QWidget *parent = nullptr);
    ~WBSignalDetectWidget();

signals:
    //记录开始检测时间
    void startDetect();
    //记录最终完成检测时间
    void stopDetect();
    //单次数据传入触发处理
    void sigTriggerSignalDetect(float *FFtin,       //FFT输入数据
                                int InStep,         //平滑滑窗的宽度
                                int length,         //FFT的点数
                                int Freqency,       //中心频点
                                int BandWidth);     //当前FFT覆盖带宽
    //设置有效电平门限   dBm
    void sigSetValidAmpThreshold(float amp);

private:
    Ui::WBSignalDetectWidget *ui;

    WBSignalDetectModel* m_pGenericModel = nullptr;

    SignalDetectTableView* m_pSignalDetectTable = nullptr;

    DisturbNoiseTableView* m_pDisturbNoiseTable = nullptr;

    ManMadeNoiseTableView* m_pManMadeNoiseTable = nullptr;

    PopupParamSet* m_pPopupParamSet = nullptr;

    TypicalFreqSetWidget* m_pTypicalFreqSetWidget = nullptr;

    bool turnToCorrectTableModel();

    ParamSet m_DetectParam;

private slots:
    void on_tabWidget_SignalDetectTable_currentChanged(int index);

    void on_pushButton_ParamSet_clicked();

    void slotSetDetectParam(ParamSet param);

    void on_pushButton_setLegalFreq_clicked(bool checked);
    void on_pushButton_cleanAllData_clicked();
    void on_pushButton_GenerateDisturbSIgnal_clicked();
    void on_pushButton_GenerateManMadeNoise_clicked();
    void on_pushButton_GenerateElecEnvReport_clicked();
    //默认配置文件保存在可执行文件同级目录下名称为 legalFreq.ini 文件中
    void on_pushButton_importLegal_clicked();
    void on_pushButton_ExportLegal_clicked();
    void on_pushButton_TypicalFreqSet_clicked();
};

#endif // WBSIGNALDETECTWIDGET_H
