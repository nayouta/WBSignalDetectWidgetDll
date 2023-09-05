#include "../inc/PopupParamSet.h"
#include <QFormLayout>
#include <QLabel>

PopupParamSet::PopupParamSet(QWidget *parent): QDialog(parent)
{
    qRegisterMetaType<ParamSet>("ParamSet");
    setupUi();
}

void PopupParamSet::setupUi()
{
    auto formLayout = new QFormLayout;
    formLayout->addRow(new QLabel("载波检测频点匹配差值(MHz):"), doubleSpinBox_FreqPointThreshold = new QDoubleSpinBox);
    formLayout->addRow(new QLabel("载波检测带宽匹配差值(KHz):"), doubleSpinBox_BandwidthThreshold = new QDoubleSpinBox);
    formLayout->addRow(new QLabel("非活动信号时间范围(秒):"), spinBox_ActiveThreshold = new QSpinBox);

    spinBox_ActiveThreshold->setValue(10);
    doubleSpinBox_BandwidthThreshold->setValue(10);
    doubleSpinBox_FreqPointThreshold->setValue(10);


    auto horizontalLayout = new QHBoxLayout;
    horizontalLayout->addStretch();
    horizontalLayout->addWidget(pushButton_Confirm = new QPushButton("确认"));
    connect(pushButton_Confirm, &QPushButton::clicked, this, [this] {
        ParamSet curParam;
        curParam.FreqPointThreshold = doubleSpinBox_FreqPointThreshold->value() * 1e6;
        curParam.BandwidthThreshold = doubleSpinBox_BandwidthThreshold->value() * 1e3;
        curParam.ActiveThreshold = spinBox_ActiveThreshold->value();
        emit sigUpdateParam(curParam);
        close();
    });
    horizontalLayout->addStretch();
    horizontalLayout->addWidget(pushButton_Cancel = new QPushButton("取消"));
    connect(pushButton_Cancel, &QPushButton::clicked, this, [this] {
        close();
    });
    horizontalLayout->addStretch();

    auto verticalLayout = new QVBoxLayout(this);
    verticalLayout->addLayout(formLayout);
    verticalLayout->addLayout(horizontalLayout);
}

