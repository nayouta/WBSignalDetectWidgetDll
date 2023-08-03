#include "../inc/signaldetecttableview.h"

#include <QHeaderView>

#include "wbsignaldetectmodel.h"

SignalDetectTableView::SignalDetectTableView(QWidget *parent)
    : QTableView{parent}
{
    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    setSortingEnabled(true);

    setModel(m_model = new QStandardItemModel(this));
    RenewItems();
}

void SignalDetectTableView::RenewItems()
{
    m_model->clear();
    m_model->setColumnCount(7);

    m_model->setHeaderData(0, Qt::Horizontal, tr("典型频率点(MHz)"));
    m_model->setHeaderData(1, Qt::Horizontal, tr("测量频率(MHz)"));
    m_model->setHeaderData(2, Qt::Horizontal, tr("时间(h:m)"));
    m_model->setHeaderData(3, Qt::Horizontal, tr("测量电平(dB/uV)"));
    m_model->setHeaderData(4, Qt::Horizontal, tr("平均电平(dB/uV)"));
    m_model->setHeaderData(5, Qt::Horizontal, tr("最大电平(dB/uV)"));
    m_model->setHeaderData(6, Qt::Horizontal, tr("最小电平(dB/uV)"));
    m_model->setHeaderData(7, Qt::Horizontal, tr("检波方式(dB/uV)"));
    m_model->setHeaderData(8, Qt::Horizontal, tr("中频带宽(dB/uV)"));
}

SignalDetectTableView::~SignalDetectTableView(){
    if(m_model){
        delete m_model;
    }
}

