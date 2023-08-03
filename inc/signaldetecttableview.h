#ifndef SIGNALDETECTTABLEVIEW_H
#define SIGNALDETECTTABLEVIEW_H

#include <QWidget>
#include <QTableView>
#include <QStandardItemModel>

class SignalDetectTableView : public QTableView
{
    Q_OBJECT
public:
    explicit SignalDetectTableView(QWidget *parent = nullptr);

    void RenewItems();
protected:
    virtual ~SignalDetectTableView();

private:
    QStandardItemModel* m_model = nullptr;

signals:

};

#endif // SIGNALDETECTTABLEVIEW_H
