#ifndef MANMADENOISETABLEVIEW_H
#define MANMADENOISETABLEVIEW_H

#include <QWidget>
#include <QTableView>

class ManMadeNoiseTableView : public QTableView
{
    Q_OBJECT
public:
    explicit ManMadeNoiseTableView(QWidget *parent = nullptr);
    //TODO:电磁环境人为噪声电平测量记录
    bool GenerateExcelTable(QString folderName);

signals:

protected:
    virtual ~ManMadeNoiseTableView();

};

#endif // MANMADENOISETABLEVIEW_H
