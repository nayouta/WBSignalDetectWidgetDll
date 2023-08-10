#ifndef MANMADENOISETABLEVIEW_H
#define MANMADENOISETABLEVIEW_H

#include <QWidget>
#include <QTableView>

class ManMadeNoiseTableView : public QTableView
{
    Q_OBJECT
public:
    explicit ManMadeNoiseTableView(QWidget *parent = nullptr);
    //电磁环境人为噪声电平测量记录  //输入为，传入每个界面显示的记录，key为典型频点记录
    bool GenerateExcelTable(QString folderName, QMap<int, int> mapExistTypicalFreqNoiseRecordAmount);

signals:

protected:
    virtual ~ManMadeNoiseTableView();

};

#endif // MANMADENOISETABLEVIEW_H
