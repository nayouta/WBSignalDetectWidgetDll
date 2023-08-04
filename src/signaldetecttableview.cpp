#include "signaldetecttableview.h"

#include <QHeaderView>


SignalDetectTableView::SignalDetectTableView(QWidget *parent)
    : QTableView{parent}
{
    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    setSortingEnabled(true);

}

SignalDetectTableView::~SignalDetectTableView(){
}

