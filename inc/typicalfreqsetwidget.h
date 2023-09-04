#ifndef TYPICALFREQSETWIDGET_H
#define TYPICALFREQSETWIDGET_H

#include <QWidget>
#include <QMap>
#include <QDialog>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QPushButton>

class TypicalFreqSetWidget : public QDialog
{
    Q_OBJECT
public:
    explicit TypicalFreqSetWidget(QWidget *parent = nullptr);
    void SetCurrentTypicalFreqFromTable(QList<int> lst);

signals:
    void sigHaveTypicalFreq(const QList<int>& mapValue);

private:
    void setupUi();
    QList<int> m_lstValue;
    static constexpr int SETTING_LINE = 7;
    QCheckBox *checkBox_Enable[SETTING_LINE];
    QDoubleSpinBox *lineEdit_TypicalFreq[SETTING_LINE];
    QPushButton *pushButton_Confirm;
    QPushButton *pushButton_Cancel;
};

#endif // TYPICALFREQSETWIDGET_H
