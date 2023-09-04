#include "DisturbNoiseTableView.h"
#include <QHeaderView>

#include "xlsxdocument.h"
#include "xlsxcellrange.h"

DisturbNoiseTableView::DisturbNoiseTableView(QWidget *parent): QTableView (parent)
{
    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    setSortingEnabled(true);
    verticalHeader()->hide();
}

bool DisturbNoiseTableView::GenerateExcelTable(QString folderName)
{
    QString fileName = folderName + "/干扰信号测量记录" + QDateTime::currentDateTime().toString(" yyyy-MM-dd hh：mm：ss") + ".xlsx";

    QXlsx::Document xlsx;

    //不跟随当前实际信号状态递增的部分
    QXlsx::Format format;
    format.setHorizontalAlignment(QXlsx::Format::AlignHCenter);
    format.setVerticalAlignment(QXlsx::Format::AlignVCenter);
    format.setBorderStyle(QXlsx::Format::BorderThin);
    QXlsx::CellRange range("B2:D2");
    xlsx.mergeCells(range, format);
    xlsx.write("B2", "测量日期和时间：     年   月   日		", format);

    range = QXlsx::CellRange("E2:I2");
    xlsx.mergeCells(range, format);
    xlsx.write("E2", "测试地点：         北纬：       东经：				", format);

    xlsx.write("B3", "环境条件", format);

    range = QXlsx::CellRange("C3:I3");
    xlsx.mergeCells(range, format);
    xlsx.write("C3", "天气状况：      温度：           湿度：						", format);

    xlsx.write("B4", "测量仪器", format);

    range = QXlsx::CellRange("C4:D4");
    xlsx.mergeCells(range, format);
    xlsx.write("C4", "", format);

    xlsx.write("E4", "中频带宽", format);
    xlsx.write("F4", "2.4KHz", format);

    xlsx.write("G4", "检波方式", format);

    range = QXlsx::CellRange("H4:I4");
    xlsx.mergeCells(range, format);
    xlsx.write("H4", "RMS", format);

    // Write column headers
    QHeaderView *headerView = this->horizontalHeader();
    //记录表格无序号列
    QString headerText = headerView->model()->headerData(1, Qt::Horizontal, Qt::DisplayRole).toString();
    xlsx.write("B5", headerText, format);

    headerText = headerView->model()->headerData(2, Qt::Horizontal, Qt::DisplayRole).toString();
    range = QXlsx::CellRange("C5:D5");
    xlsx.mergeCells(range, format);
    xlsx.write("C5", headerText, format);

    headerText = headerView->model()->headerData(3, Qt::Horizontal, Qt::DisplayRole).toString();
    range = QXlsx::CellRange("E5:F5");
    xlsx.mergeCells(range, format);
    xlsx.write("E5", headerText, format);

    range = QXlsx::CellRange("G5:I5");
    xlsx.mergeCells(range, format);
    xlsx.write("G5", "说明", format);


    // Write table data
    int dataPosRow = 6;
    int curDataCol;
    QModelIndex item;
    for (int row = 0; row < this->model()->rowCount(); ++row) {
        curDataCol = 1;
        item = this->model()->index(row, curDataCol);
        if (item.isValid()) {
            xlsx.write("B" + QString::number(dataPosRow), this->model()->data(item), format);
        }
        curDataCol += 1;

        range = QXlsx::CellRange("C" + QString::number(dataPosRow) + ":" + "D" + QString::number(dataPosRow));
        xlsx.mergeCells(range, format);
        item = this->model()->index(row, curDataCol);
        if (item.isValid()) {
            xlsx.write(QString("C") + QString::number(dataPosRow), this->model()->data(item), format);
        }
        curDataCol += 1;

        range = QXlsx::CellRange("E" + QString::number(dataPosRow) + ":" + "F" + QString::number(dataPosRow));
        xlsx.mergeCells(range, format);
        item = this->model()->index(row, curDataCol);
        if (item.isValid()) {
            xlsx.write(QString("E") + QString::number(dataPosRow), this->model()->data(item), format);
        }

        range = QXlsx::CellRange("G" + QString::number(dataPosRow) + ":" + "I" + QString::number(dataPosRow));
        xlsx.mergeCells(range, format);

        dataPosRow += 1;
    }

    // Save the Excel file
    return xlsx.saveAs(fileName);
}

DisturbNoiseTableView::~DisturbNoiseTableView()
{

}
