#ifndef POPUPPARAMSET_H
#define POPUPPARAMSET_H

#include <QWidget>
#include <QMetaType>
#include <QDialog>

namespace Ui {
class PopupParamSet;
}

struct ParamSet{
    uint FreqPointThreshold = 0;
    uint BandwidthThreshold = 0;
    uint ActiveThreshold = 0;
};

Q_DECLARE_METATYPE(ParamSet);

class PopupParamSet : public QDialog
{
    Q_OBJECT

public:
    explicit PopupParamSet(QWidget *parent = nullptr);
    ~PopupParamSet();

signals:
    void sigUpdateParam(ParamSet param);

private slots:
    void on_pushButton_Confirm_clicked();

    void on_pushButton_Cancel_clicked();

private:
    Ui::PopupParamSet *ui;
};

#endif // POPUPPARAMSET_H
