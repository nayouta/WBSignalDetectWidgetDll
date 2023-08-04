#ifndef MANMADENOISETABLEVIEW_H
#define MANMADENOISETABLEVIEW_H

#include <QWidget>
#include <QTableView>

class ManMadeNoiseTableView : public QTableView
{
    Q_OBJECT
public:
    explicit ManMadeNoiseTableView(QWidget *parent = nullptr);

signals:

protected:
    virtual ~ManMadeNoiseTableView();

};

#endif // MANMADENOISETABLEVIEW_H
