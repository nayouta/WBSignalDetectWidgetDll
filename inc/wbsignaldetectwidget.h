#ifndef WBSIGNALDETECTWIDGET_H
#define WBSIGNALDETECTWIDGET_H

#include <QWidget>

class WBSignalDetectModel;
class SignalDetectTableView;
class ManMadeNoiseTableView;
class DisturbNoiseTableView;

namespace Ui {
class WBSignalDetectWidget;
}

class WBSignalDetectWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WBSignalDetectWidget(QWidget *parent = nullptr);
    ~WBSignalDetectWidget();

private slots:
    void on_tabWidget_SignalDetectTable_currentChanged(int index);

private:
    Ui::WBSignalDetectWidget *ui;

    WBSignalDetectModel* m_pGenericModel = nullptr;

    SignalDetectTableView* m_pSignalDetectTable = nullptr;

    DisturbNoiseTableView* m_pDisturbNoiseTable = nullptr;

    ManMadeNoiseTableView* m_pManMadeNoiseTable = nullptr;

    bool turnToCorrectTableModel();

//测试入口
public:
    WBSignalDetectModel *pGenericModel() const;
};

#endif // WBSIGNALDETECTWIDGET_H
