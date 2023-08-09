#ifndef SIGNALDETECTTABLEVIEW_H
#define SIGNALDETECTTABLEVIEW_H

#include <QWidget>
#include <QTableView>
#include <QItemDelegate>
#include <QComboBox>

class ComboBoxDelegate : public QItemDelegate {
    Q_OBJECT

public:
    ComboBoxDelegate(QObject *parent = nullptr) : QItemDelegate(parent) {}

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

//    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
private:
    QFont m_Font;
};

class SignalDetectTableView : public QTableView
{
    Q_OBJECT
public:
    explicit SignalDetectTableView(QWidget *parent = nullptr);
    bool GenerateExcelTable(QString folderName);

protected:
    virtual ~SignalDetectTableView();


signals:

};

#endif // SIGNALDETECTTABLEVIEW_H
