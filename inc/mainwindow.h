#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

#include <QTimer>

class WBSignalDetectWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    WBSignalDetectWidget* m_pSigDetectWidget = nullptr;

    float* m_pFFTIn = nullptr;
    QTimer timer;

private slots:
    void slotTriggerTimer();
    void on_pushButton_StartDetect_clicked();
    void on_pushButton_StopDetect_clicked();
};
#endif // MAINWINDOW_H
