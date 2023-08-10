#include "wbsignaldetectwidget.h"
#include "ui_wbsignaldetectwidget.h"

#include <QFileDialog>

#include <signaldetecttableview.h>
#include <wbsignaldetectmodel.h>
#include <manmadenoisetableview.h>
#include <disturbnoisetableview.h>

#include <QMessageBox>

WBSignalDetectWidget::WBSignalDetectWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WBSignalDetectWidget)
{
    ui->setupUi(this);

    m_pGenericModel = new WBSignalDetectModel(this);

    m_pSignalDetectTable = new SignalDetectTableView(this);
    m_pSignalDetectTable->setModel(m_pGenericModel);

    m_pDisturbNoiseTable = new DisturbNoiseTableView(this);
    m_pDisturbNoiseTable->setModel(m_pGenericModel);

    m_pManMadeNoiseTable = new ManMadeNoiseTableView(this);
    m_pManMadeNoiseTable->setModel(m_pGenericModel);

    ui->tabWidget_SignalDetectTable->addTab(m_pSignalDetectTable, "信号检测表");
    ui->tabWidget_SignalDetectTable->addTab(m_pDisturbNoiseTable, "干扰信号测量表");
    ui->tabWidget_SignalDetectTable->addTab(m_pManMadeNoiseTable, "电磁环境人为噪声电平测量表");

    m_pPopupParamSet = new PopupParamSet(this);
    m_pPopupParamSet->setModal(false);
    m_pPopupParamSet->hide();

    turnToCorrectTableModel();

    connect(this, &WBSignalDetectWidget::startDetect, m_pGenericModel, &WBSignalDetectModel::SetStartTime);
    connect(this, &WBSignalDetectWidget::stopDetect, m_pGenericModel, &WBSignalDetectModel::SetStopTime);
    connect(this, &WBSignalDetectWidget::sigTriggerSignalDetect, m_pGenericModel, &WBSignalDetectModel::FindSignal);
    connect(this, &WBSignalDetectWidget::sigSetValidAmpThreshold, m_pGenericModel,&WBSignalDetectModel::setFThreshold);

    connect(m_pPopupParamSet, &PopupParamSet::sigUpdateParam, this, &WBSignalDetectWidget::slotSetDetectParam);

}

WBSignalDetectWidget::~WBSignalDetectWidget()
{
    delete ui;
}

void WBSignalDetectWidget::on_tabWidget_SignalDetectTable_currentChanged(int index)
{
    turnToCorrectTableModel();
}

bool WBSignalDetectWidget::turnToCorrectTableModel()
{

    if(m_pGenericModel == nullptr){
        return false;
    }

    //默认直接切换当前tab也能触发写入合法信号的状态
    ui->pushButton_setLegalFreq->setChecked(false);
    on_pushButton_setLegalFreq_clicked(false);

    if(ui->tabWidget_SignalDetectTable->currentIndex() != 0){
        ui->pushButton_setLegalFreq->setEnabled(false);
        ui->pushButton_setLegalFreq->hide();
    }else{
        ui->pushButton_setLegalFreq->setEnabled(true);
        ui->pushButton_setLegalFreq->show();
    }

    switch (ui->tabWidget_SignalDetectTable->currentIndex()) {
    case 0:
        m_pGenericModel->setUserViewType(SIGNAL_DETECT_TABLE);
        break;
    case 1:
        m_pGenericModel->setUserViewType(DISTURB_NOISE_TABLE);
        break;
    case 2:
        m_pGenericModel->setUserViewType(MAN_MADE_NOISE_TABLE);
        break;
    default:
        m_pGenericModel->setUserViewType(NOT_USED);
        break;
    }

    emit m_pGenericModel->sigTriggerRefreshData();
    return true;
}

void WBSignalDetectWidget::slotSetDetectParam(ParamSet param)
{
    m_DetectParam = param;
    m_pGenericModel->setBandwidthThreshold(m_DetectParam.BandwidthThreshold);
    m_pGenericModel->setActiveThreshold(m_DetectParam.ActiveThreshold);
    m_pGenericModel->setFreqPointThreshold(m_DetectParam.FreqPointThreshold);
}


void WBSignalDetectWidget::on_pushButton_ParamSet_clicked()
{
    m_pPopupParamSet->setModal(true);
    m_pPopupParamSet->show();
}

void WBSignalDetectWidget::on_pushButton_setLegalFreq_clicked(bool checked)
{
    if(checked){
        ui->pushButton_setLegalFreq->setText("完成设置");
    }else{
        ui->pushButton_setLegalFreq->setText("开始设置非法频点");
    }
    if(m_pGenericModel){
        m_pGenericModel->SlotTriggerLegalFreqSet(checked);
    }
}


void WBSignalDetectWidget::on_pushButton_cleanAllData_clicked()
{
    if(m_pGenericModel){
        m_pGenericModel->SlotCleanUp();
    }
}


void WBSignalDetectWidget::on_pushButton_GenerateDisturbSIgnal_clicked()
{
    //先将tabwidget转到对应的tab上
    ui->tabWidget_SignalDetectTable->setCurrentIndex(1);
    QFileDialog dialog;
    QString selectedFolder = dialog.getExistingDirectory(this, tr("Select Directory"),
                                                         QDir::currentPath(),
                                                         QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (selectedFolder.isEmpty()) {
        qDebug() << "File saving cancelled.";
        return;
    }
    if(!m_pDisturbNoiseTable->GenerateExcelTable(selectedFolder)){
        //TODO: 生成失败时的处理方法
    }
}


void WBSignalDetectWidget::on_pushButton_GenerateManMadeNoise_clicked()
{
    //先将tabwidget转到对应的tab上
    ui->tabWidget_SignalDetectTable->setCurrentIndex(2);
    QFileDialog dialog;
    QString selectedFolder = dialog.getExistingDirectory(this, tr("Select Directory"),
                                                         QDir::currentPath(),
                                                         QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (selectedFolder.isEmpty()) {
        qDebug() << "File saving cancelled.";
        return;
    }
    if(!m_pManMadeNoiseTable->GenerateExcelTable(selectedFolder, m_pGenericModel->mapExistTypicalFreqNoiseRecordAmount())){
        //TODO: 生成失败时的处理方法
    }
}


void WBSignalDetectWidget::on_pushButton_GenerateElecEnvReport_clicked()
{

}

void WBSignalDetectWidget::on_pushButton_importLegal_clicked()
{
    QMessageBox msgBox;
    if(m_pGenericModel){
        if(m_pGenericModel->SlotImportLegalFreqConf()){
            msgBox.setText("导入成功！");
            msgBox.exec();
            return;
        }
    }
    msgBox.setText("导入失败！");
    msgBox.exec();

}

void WBSignalDetectWidget::on_pushButton_ExportLegal_clicked()
{
    QMessageBox msgBox;
    if(m_pGenericModel){
        m_pGenericModel->SlotExportLegalFreqConf();
        msgBox.setText("导出成功！");
        msgBox.exec();
        return;
    }
    msgBox.setText("导出失败！");
    msgBox.exec();
}


