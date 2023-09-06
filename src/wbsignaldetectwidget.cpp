#include "../inc/WBSignalDetectWidget.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QBoxLayout>
#include "global.h"

WBSignalDetectWidget::WBSignalDetectWidget(QWidget *parent): QWidget(parent)
{
    setupUi();
    m_pGenericModel = new WBSignalDetectModel;
    tabWidget_SignalDetectTable->addTab(m_pSignalDetectTable = new SignalDetectTableView, "信号检测表");
    m_pSignalDetectTable->setModel(m_pGenericModel);
    tabWidget_SignalDetectTable->addTab(m_pDisturbNoiseTable = new DisturbNoiseTableView, "干扰信号测量表");
    m_pDisturbNoiseTable->setModel(m_pGenericModel);
    tabWidget_SignalDetectTable->addTab(m_pManMadeNoiseTable = new ManMadeNoiseTableView, "电磁环境人为噪声电平测量表");
    m_pManMadeNoiseTable->setModel(m_pGenericModel);

    turnToCorrectTableModel();
    connect(this, &WBSignalDetectWidget::startDetect, m_pGenericModel, &WBSignalDetectModel::SetStartTime);
    connect(this, &WBSignalDetectWidget::stopDetect, m_pGenericModel, &WBSignalDetectModel::SetStopTime);
    connect(m_pPopupParamSet = new PopupParamSet, &PopupParamSet::sigUpdateParam, this, [this](ParamSet param) {
        m_DetectParam = param;
        m_pGenericModel->setBandwidthThreshold(m_DetectParam.BandwidthThreshold);
        m_pGenericModel->setActiveThreshold(m_DetectParam.ActiveThreshold);
        m_pGenericModel->setFreqPointThreshold(m_DetectParam.FreqPointThreshold);
    });
    m_pPopupParamSet->setModal(false);
    m_pPopupParamSet->hide();
    connect(m_pTypicalFreqSetWidget = new TypicalFreqSetWidget, &TypicalFreqSetWidget::sigHaveTypicalFreq, m_pGenericModel, &WBSignalDetectModel::setMapTypicalFreqAndItsTestFreq);
    m_pTypicalFreqSetWidget->setModal(false);
    m_pTypicalFreqSetWidget->hide();
}

void WBSignalDetectWidget::sigTriggerSignalDetect(unsigned char* amplData, int InStep, int length, int Freqency, int BandWidth)
{
    auto FFtin = ippsMalloc_32f(length);
    for (int i = 0; i < length; ++i)
    {
        FFtin[i] = (short)amplData[i] + AMPL_OFFSET;
    }
    m_pGenericModel->FindSignal(FFtin, InStep, length, Freqency, BandWidth);
    ippsFree(FFtin);
}

void WBSignalDetectWidget::sigSetValidAmpThreshold(float amp)
{
    m_pGenericModel->setFThreshold(amp);
}

void WBSignalDetectWidget::setupUi()
{
    auto mainLayout = new QVBoxLayout(this);
    auto horizontalLayout = new QHBoxLayout;
    horizontalLayout->addWidget(pushButton_ParamSet = new QPushButton("参数设置"));
    connect(pushButton_ParamSet, &QPushButton::clicked, this, [this] {
        m_pPopupParamSet->setModal(true);
        m_pPopupParamSet->show();
    });
    horizontalLayout->addWidget(pushButton_TypicalFreqSet = new QPushButton("典型频点设置"));
    connect(this, &WBSignalDetectWidget::startDetect, this, &WBSignalDetectWidget::slotHideTypicalFreqSetButton);
    connect(this, &WBSignalDetectWidget::stopDetect, this, &WBSignalDetectWidget::slotShowTypicalFreqSetButton);
    connect(pushButton_TypicalFreqSet, &QPushButton::clicked, this, [this] {
        m_pTypicalFreqSetWidget->setModal(true);
        m_pTypicalFreqSetWidget->SetCurrentTypicalFreqFromTable(m_pGenericModel->lstTypicalFreq());
        m_pTypicalFreqSetWidget->show();
    });
    horizontalLayout->addStretch();
    horizontalLayout->addWidget(pushButton_importLegal = new QPushButton("导入非法频点设置"));
    connect(pushButton_importLegal, &QPushButton::clicked, this, [this] {
        QMessageBox::information(nullptr, "导入非法频点设置", (m_pGenericModel && m_pGenericModel->SlotImportLegalFreqConf())? "导入成功！": "导入失败！");
    });
    horizontalLayout->addWidget(pushButton_ExportLegal = new QPushButton("导出非法频点设置"));
    connect(pushButton_ExportLegal, &QPushButton::clicked, this, [this] {
        QMessageBox::information(nullptr, "导出非法频点设置", (m_pGenericModel && m_pGenericModel->SlotExportLegalFreqConf())? "导出成功！": "导出失败！");
    });
    horizontalLayout->addWidget(pushButton_cleanAllData = new QPushButton("清理"));
    connect(pushButton_cleanAllData, &QPushButton::clicked, this, [this] {
        if (m_pGenericModel)
            m_pGenericModel->SlotCleanUp();
    });
    horizontalLayout->addWidget(pushButton_setLegalFreq = new QPushButton("开始设置非法频点"));
    connect(pushButton_setLegalFreq, &QPushButton::clicked, this, [this](bool checked) {
        if(checked)
            pushButton_setLegalFreq->setText("完成设置");
        else
            pushButton_setLegalFreq->setText("开始设置非法频点");
        if (m_pGenericModel)
            m_pGenericModel->SlotTriggerLegalFreqSet(checked);
    });
    pushButton_setLegalFreq->setCheckable(true);

    mainLayout->addLayout(horizontalLayout);
    mainLayout->addWidget(tabWidget_SignalDetectTable = new QTabWidget);
    connect(tabWidget_SignalDetectTable, &QTabWidget::currentChanged, this, [this](int) {
        turnToCorrectTableModel();
    });

    horizontalLayout = new QHBoxLayout;
    horizontalLayout->addStretch();
    horizontalLayout->addWidget(pushButton_GenerateSignalDetect = new QPushButton("保存信号检测记录"));
    connect(pushButton_GenerateSignalDetect, &QPushButton::clicked, this, [this] {
        //先将tabwidget转到对应的tab上
        tabWidget_SignalDetectTable->setCurrentIndex(0);
        QFileDialog dialog;
        QString selectedFolder = dialog.getExistingDirectory(this, tr("Select Directory"), QDir::currentPath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if (selectedFolder.isEmpty())
        {
            qDebug() << "File saving cancelled.";
            return;
        }
        if (!m_pSignalDetectTable->GenerateExcelTable(selectedFolder))
        {
            //TODO: 生成失败时的处理方法
        }
    });
    horizontalLayout->addWidget(pushButton_GenerateDisturbSIgnal = new QPushButton("保存干扰信号测量记录"));
    connect(pushButton_GenerateDisturbSIgnal, &QPushButton::clicked, this, [this] {
        //先将tabwidget转到对应的tab上
        tabWidget_SignalDetectTable->setCurrentIndex(1);
        QFileDialog dialog;
        QString selectedFolder = dialog.getExistingDirectory(this, tr("Select Directory"), QDir::currentPath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if (selectedFolder.isEmpty())
        {
            qDebug() << "File saving cancelled.";
            return;
        }
        if (!m_pDisturbNoiseTable->GenerateExcelTable(selectedFolder))
        {
            //TODO: 生成失败时的处理方法
        }
    });
    horizontalLayout->addWidget(pushButton_GenerateManMadeNoise = new QPushButton("保存电磁环境人为噪声电平测量记录"));
    connect(pushButton_GenerateManMadeNoise, &QPushButton::clicked, this, [this] {
        //先将tabwidget转到对应的tab上
        tabWidget_SignalDetectTable->setCurrentIndex(2);
        QFileDialog dialog;
        QString selectedFolder = dialog.getExistingDirectory(this, tr("Select Directory"), QDir::currentPath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if (selectedFolder.isEmpty())
        {
            qDebug() << "File saving cancelled.";
            return;
        }
        if (!m_pManMadeNoiseTable->GenerateExcelTable(selectedFolder, m_pGenericModel->mapExistTypicalFreqNoiseRecordAmount()))
        {
            //TODO: 生成失败时的处理方法
        }
    });
    horizontalLayout->addWidget(pushButton_GenerateElecEnvReport = new QPushButton("生成电磁环境测试报告"));
    connect(pushButton_GenerateElecEnvReport, &QPushButton::clicked, this, [this] {

    });
    mainLayout->addLayout(horizontalLayout);
}

bool WBSignalDetectWidget::turnToCorrectTableModel()
{
    if (m_pGenericModel == nullptr)
        return false;
    //默认直接切换当前tab也能触发写入合法信号的状态
    emit pushButton_setLegalFreq->clicked(false);
    if (tabWidget_SignalDetectTable->currentIndex() != 0)
    {
        pushButton_setLegalFreq->setEnabled(false);
        pushButton_setLegalFreq->hide();
    }
    else
    {
        pushButton_setLegalFreq->setEnabled(true);
        pushButton_setLegalFreq->show();
    }
    switch (tabWidget_SignalDetectTable->currentIndex())
    {
    case 0: m_pGenericModel->setUserViewType(SIGNAL_DETECT_TABLE); break;
    case 1: m_pGenericModel->setUserViewType(DISTURB_NOISE_TABLE); break;
    case 2: m_pGenericModel->setUserViewType(MAN_MADE_NOISE_TABLE); break;
    default: m_pGenericModel->setUserViewType(NOT_USED); break;
    }
    emit m_pGenericModel->sigTriggerRefreshData();
    return true;
}

void WBSignalDetectWidget::slotHideTypicalFreqSetButton()
{
    pushButton_TypicalFreqSet->setVisible(false);
}

void WBSignalDetectWidget::slotShowTypicalFreqSetButton()
{
    pushButton_TypicalFreqSet->setVisible(true);
}

