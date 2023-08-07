#include "wbsignaldetectwidget.h"
#include "ui_wbsignaldetectwidget.h"

#include <signaldetecttableview.h>
#include <wbsignaldetectmodel.h>
#include <manmadenoisetableview.h>
#include <disturbnoisetableview.h>


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
