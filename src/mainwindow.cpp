#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "wbsignaldetectwidget.h"

#include "wbsignaldetectmodel.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_pSigDetectWidget = new WBSignalDetectWidget(this);

    this->setCentralWidget(m_pSigDetectWidget);

    //测试假数据
    float fftin[1024];
    for (int index = 0; index < 1024; ++index){
        if(index < 512){
            fftin[index] = index;
        }else{
            fftin[index] = 1023 - index;
        }
    }


    emit m_pSigDetectWidget->pGenericModel()->sigTriggerUpdateData(fftin, 32, 1024, 100, 10);
}

MainWindow::~MainWindow()
{
    delete ui;
}

