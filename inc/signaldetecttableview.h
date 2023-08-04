#ifndef SIGNALDETECTTABLEVIEW_H
#define SIGNALDETECTTABLEVIEW_H

#include <QWidget>
#include <QTableView>

class SignalDetectTableView : public QTableView
{
    Q_OBJECT
public:
    explicit SignalDetectTableView(QWidget *parent = nullptr);

protected:
    virtual ~SignalDetectTableView();


signals:

};

#endif // SIGNALDETECTTABLEVIEW_H
