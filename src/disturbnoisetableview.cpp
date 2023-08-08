#include "../inc/disturbnoisetableview.h"
#include <QHeaderView>

DisturbNoiseTableView::DisturbNoiseTableView(QWidget *parent)
    : QTableView (parent)
{
    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    setSortingEnabled(true);
    this->verticalHeader()->hide();
}

DisturbNoiseTableView::~DisturbNoiseTableView()
{

}
