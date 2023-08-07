#include "popupparamset.h"
#include "ui_popupparamset.h"

PopupParamSet::PopupParamSet(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PopupParamSet)
{
    qRegisterMetaType<ParamSet>("ParamSet");
    ui->setupUi(this);
}

PopupParamSet::~PopupParamSet()
{
    delete ui;
}

void PopupParamSet::on_pushButton_Confirm_clicked()
{
    ParamSet curParam;
    curParam.FreqPointThreshold = ui->doubleSpinBox_FreqPointThreshold->value() * 1e6;
    curParam.BandwidthThreshold = ui->doubleSpinBox_BandwidthThreshold->value() * 1e3;
    curParam.ActiveThreshold = ui->spinBox_ActiveThreshold->value();
    emit sigUpdateParam(curParam);
    this->close();
}

void PopupParamSet::on_pushButton_Cancel_clicked()
{
    this->close();
}

