#include "ManMadeNoiseTableView.h"
#include <QHeaderView>

#include "xlsxdocument.h"
#include "xlsxcellrange.h"

ManMadeNoiseTableView::ManMadeNoiseTableView(QWidget *parent): QTableView{parent}
{
    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    setSortingEnabled(true);
    verticalHeader()->hide();
}

bool ManMadeNoiseTableView::GenerateExcelTable(QString folderName, QMap<int, int> mapExistTypicalFreqNoiseRecordAmount)
{
    QString fileName = folderName + "/电磁环境人为噪声电平测量记录" + QDateTime::currentDateTime().toString(" yyyy-MM-dd hh：mm：ss") + ".xlsx";
    QXlsx::Document xlsx;
    //不跟随当前实际信号状态递增的部分
    QXlsx::Format format;
    format.setHorizontalAlignment(QXlsx::Format::AlignHCenter);
    format.setVerticalAlignment(QXlsx::Format::AlignVCenter);
    format.setBorderStyle(QXlsx::Format::BorderThin);

    QXlsx::CellRange range("B2:E2");
    xlsx.mergeCells(range, format);
    xlsx.write("B2", "测量日期和时间：     年   月   日		", format);

    range = QXlsx::CellRange("F2:J2");
    xlsx.mergeCells(range, format);
    xlsx.write("F2", "测试地点：         北纬：       东经：				", format);

    xlsx.write("B3", "环境条件", format);

    range = QXlsx::CellRange("C3:E3");
    xlsx.mergeCells(range, format);
    xlsx.write("C3", "天气状况：      温度：           湿度：						", format);

    xlsx.write("F3", "测量仪器", format);

    range = QXlsx::CellRange("G3:J3");
    xlsx.mergeCells(range, format);
    xlsx.write("G3", "", format);

    // Write column headers
    xlsx.write("B4", "典型频率点(MHz)", format);
    xlsx.write("C4", "测量频率(MHz)", format);
    xlsx.write("D4", "测量时间(时：分)", format);
    xlsx.write("E4", "测量电平(dBuV)", format);
    xlsx.write("F4", "平均电平(dBuV)", format);
    xlsx.write("G4", "最大电平(dBuV)", format);
    xlsx.write("H4", "最小电平(dBuV)", format);
    xlsx.write("I4", "检波方式", format);
    xlsx.write("J4", "中频带宽", format);

    // Write table data
    //先将典型频率点占用的单元格合并
    int startRow = 5;
    foreach(const auto& curTypicalFreq, mapExistTypicalFreqNoiseRecordAmount.keys()){
        int curNoiseRecordAmount = mapExistTypicalFreqNoiseRecordAmount[curTypicalFreq];
        range = QXlsx::CellRange("B" + QString::number(startRow) + ":B" + QString::number(startRow + curNoiseRecordAmount - 1));
        xlsx.mergeCells(range, format);
        xlsx.write("B" + QString::number(startRow), QString::number(double(curTypicalFreq) / 1e6, 'f', 6), format);
        startRow += curNoiseRecordAmount;
    }

    //填写界面上相应的数据
    int curDataCol;
    int dataPosRow = 5;
    QList<int> existAmpForEveryTypicalFreqLst;
    QModelIndex item;
    for (int row = 0; row < this->model()->rowCount(); ++row) {
        curDataCol = 1;
        item = this->model()->index(row, curDataCol);
        if (item.isValid()) {
            xlsx.write("C" + QString::number(dataPosRow), this->model()->data(item), format);
        }
        curDataCol += 1;

        item = this->model()->index(row, curDataCol);
        if (item.isValid()) {
            xlsx.write(QString("D") + QString::number(dataPosRow), this->model()->data(item), format);
        }
        curDataCol += 1;

        item = this->model()->index(row, curDataCol);
        if (item.isValid()) {
            xlsx.write(QString("E") + QString::number(dataPosRow), this->model()->data(item), format);
        }
        existAmpForEveryTypicalFreqLst.append(this->model()->data(item).toString().toDouble());
        dataPosRow += 1;
    }

    //计算平均电平、最大电平、最小电平，合并对应单元格并填入
    int staticStartRow = 5;
    foreach(const auto& curTypicalFreq, mapExistTypicalFreqNoiseRecordAmount.keys()){
        QList<int> existAmpForCurrentTypicalFreqLst;
        int curNoiseRecordAmount = mapExistTypicalFreqNoiseRecordAmount[curTypicalFreq];
        int totalAmpValue = 0;
        for(int num = 0; num < curNoiseRecordAmount; ++num){
            existAmpForCurrentTypicalFreqLst.append(existAmpForEveryTypicalFreqLst.takeFirst());
            totalAmpValue += existAmpForCurrentTypicalFreqLst.constLast();
        }

        //平均电平
        range = QXlsx::CellRange("F" + QString::number(staticStartRow) + ":F" + QString::number(staticStartRow + curNoiseRecordAmount - 1));
        xlsx.mergeCells(range, format);
        xlsx.write("F" + QString::number(staticStartRow), QString::number(double(totalAmpValue) / curNoiseRecordAmount, 'f', 1), format);

        std::sort(existAmpForCurrentTypicalFreqLst.begin(), existAmpForCurrentTypicalFreqLst.end());

        //最大电平
        range = QXlsx::CellRange("G" + QString::number(staticStartRow) + ":G" + QString::number(staticStartRow + curNoiseRecordAmount - 1));
        xlsx.mergeCells(range, format);
        xlsx.write("G" + QString::number(staticStartRow), QString::number(existAmpForCurrentTypicalFreqLst.constLast()), format);
        //最小电平
        range = QXlsx::CellRange("H" + QString::number(staticStartRow) + ":H" + QString::number(staticStartRow + curNoiseRecordAmount - 1));
        xlsx.mergeCells(range, format);
        xlsx.write("H" + QString::number(staticStartRow), QString::number(existAmpForCurrentTypicalFreqLst.constFirst()), format);
        //检波方式
        range = QXlsx::CellRange("I" + QString::number(staticStartRow) + ":I" + QString::number(staticStartRow + curNoiseRecordAmount - 1));
        xlsx.mergeCells(range, format);
        xlsx.write("I" + QString::number(staticStartRow), "RMS", format);
        //中频带宽
        range = QXlsx::CellRange("J" + QString::number(staticStartRow) + ":J" + QString::number(staticStartRow + curNoiseRecordAmount - 1));
        xlsx.mergeCells(range, format);
        xlsx.write("J" + QString::number(staticStartRow), "2.4KHz", format);
        staticStartRow += curNoiseRecordAmount;
    }

    // Save the Excel file
    return xlsx.saveAs(fileName);
}

ManMadeNoiseTableView::~ManMadeNoiseTableView()
{

}


