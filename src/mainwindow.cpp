#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileInfo>
#include <QFile>
#include <QTextStream>
#include <QList>

#include "wbsignaldetectwidget.h"

#include "wbsignaldetectmodel.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //读取截取的数据
    QFileInfo dataFileInfo("output.txt");
    if(!dataFileInfo.exists()){
        return;
    }
    QFile dataFile("output.txt");
    if(!dataFile.open(QIODevice::ReadOnly | QIODevice::Text)){
        return;
    }
    QTextStream outStream(&dataFile);
    QString allData = outStream.readAll();
    dataFile.close();

    auto fullDataList = allData.split(" ");
    //当前数据为1024阶FFT，单包点数为640，故取头部640长度的原始数据作为测试
    m_pFFTIn = new float[640];
    for(int index = 0; index < 640; ++index){
        m_pFFTIn[index] = fullDataList[index].toFloat();
    }


    m_pSigDetectWidget = new WBSignalDetectWidget(this);

    this->setCentralWidget(m_pSigDetectWidget);
    emit m_pSigDetectWidget->sigSetValidAmpThreshold(-100);

    connect(&timer, &QTimer::timeout, this, &MainWindow::slotTriggerTimer);

    timer.start(10);
}



MainWindow::~MainWindow()
{
    if(m_pFFTIn){
        delete[] m_pFFTIn;
    }

    delete ui;
}

void MainWindow::slotTriggerTimer()
{
    emit m_pSigDetectWidget->sigTriggerSignalDetect((unsigned char*)m_pFFTIn, 32, 640, 15e6, 30e6);
}

void MainWindow::on_pushButton_StartDetect_clicked()
{
    emit m_pSigDetectWidget->startDetect();
}

void MainWindow::on_pushButton_StopDetect_clicked()
{
    emit m_pSigDetectWidget->stopDetect();
}

