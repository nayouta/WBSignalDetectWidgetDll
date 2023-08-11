#include "typicalfreqsetwidget.h"
#include "ui_typicalfreqsetwidget.h"

TypicalFreqSetWidget::TypicalFreqSetWidget(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TypicalFreqSetWidget)
{
    ui->setupUi(this);
}

TypicalFreqSetWidget::~TypicalFreqSetWidget()
{
    delete ui;
}

void TypicalFreqSetWidget::on_pushButton_Confirm_clicked()
{
    if(ui->checkBox_Enable->isChecked()){
        m_mapValue.insert(ui->lineEdit_TypicalFreq->text().toDouble() * 1e3, ui->lineEdit_TestFreq->text().toDouble() * 1e3);
    }
    if(ui->checkBox_Enable_2->isChecked()){
        m_mapValue.insert(ui->lineEdit_TypicalFreq_2->text().toDouble() * 1e3, ui->lineEdit_TestFreq_2->text().toDouble() * 1e3);
    }
    if(ui->checkBox_Enable_3->isChecked()){
        m_mapValue.insert(ui->lineEdit_TypicalFreq_3->text().toDouble() * 1e3, ui->lineEdit_TestFreq_3->text().toDouble() * 1e3);
    }
    if(ui->checkBox_Enable_4->isChecked()){
        m_mapValue.insert(ui->lineEdit_TypicalFreq_4->text().toDouble() * 1e3, ui->lineEdit_TestFreq_4->text().toDouble() * 1e3);
    }
    if(ui->checkBox_Enable_5->isChecked()){
        m_mapValue.insert(ui->lineEdit_TypicalFreq_5->text().toDouble() * 1e3, ui->lineEdit_TestFreq_5->text().toDouble() * 1e3);
    }
    if(ui->checkBox_Enable_6->isChecked()){
        m_mapValue.insert(ui->lineEdit_TypicalFreq_6->text().toDouble() * 1e3, ui->lineEdit_TestFreq_6->text().toDouble() * 1e3);
    }
    emit sigHaveTypicalFreq(m_mapValue);
}

void TypicalFreqSetWidget::on_pushButton_Cancel_clicked()
{
    this->close();
}

