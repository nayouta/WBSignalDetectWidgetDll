#ifndef DISTURBNOISETABLEVIEW_H
#define DISTURBNOISETABLEVIEW_H

#include <QTableView>
#include <QWidget>

class DisturbNoiseTableView : public QTableView
{
    Q_OBJECT
public:
    explicit DisturbNoiseTableView(QWidget *parent = nullptr);
protected:
    virtual ~DisturbNoiseTableView();

};

#endif // DISTURBNOISETABLEVIEW_H
