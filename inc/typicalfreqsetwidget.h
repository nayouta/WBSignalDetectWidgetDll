#ifndef TYPICALFREQSETWIDGET_H
#define TYPICALFREQSETWIDGET_H

#include <QWidget>
#include <QMap>
#include <QDialog>

namespace Ui {
class TypicalFreqSetWidget;
}

class TypicalFreqSetWidget : public QDialog
{
    Q_OBJECT

public:
    explicit TypicalFreqSetWidget(QWidget *parent = nullptr);
    ~TypicalFreqSetWidget();

signals:
    void sigHaveTypicalFreq(const QMap<int, int>& mapValue);

private slots:
    void on_pushButton_Confirm_clicked();

    void on_pushButton_Cancel_clicked();
//TODO:读写ini文件，设置值合理性判断等

private:
    Ui::TypicalFreqSetWidget *ui;

    QMap<int, int> m_mapValue;

};

#endif // TYPICALFREQSETWIDGET_H
