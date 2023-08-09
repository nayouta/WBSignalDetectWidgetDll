#include "../inc/manmadenoisetableview.h"
#include <QHeaderView>

ManMadeNoiseTableView::ManMadeNoiseTableView(QWidget *parent)
    : QTableView{parent}
{
    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    setSortingEnabled(true);
    this->verticalHeader()->hide();

}

bool ManMadeNoiseTableView::GenerateExcelTable(QString folderName)
{
    return true;
}

ManMadeNoiseTableView::~ManMadeNoiseTableView()
{

}


