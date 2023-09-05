#include "../inc/TypicalFreqSetWidget.h"

#include <QBoxLayout>
#include <QLabel>
#include <QMessageBox>

TypicalFreqSetWidget::TypicalFreqSetWidget(QWidget *parent): QDialog(parent)
{
    setupUi();
}

void TypicalFreqSetWidget::SetCurrentTypicalFreqFromTable(QList<int> lst)
{
    if(lst.length() > SETTING_LINE){
        return;
    }
    for(int index = 0; index < SETTING_LINE; index++){
        if(index < lst.length()){
            lineEdit_TypicalFreq[index]->setValue(double(lst[index] / 1e6));
            checkBox_Enable[index]->setChecked(true);
        }else{
            lineEdit_TypicalFreq[index]->setValue(0);
            checkBox_Enable[index]->setChecked(false);
        }
    }
}

void TypicalFreqSetWidget::setupUi()
{
    auto vBoxLayout = new QVBoxLayout(this);
    for (auto i = 0; i < SETTING_LINE; ++i)
    {
        auto horizontalLayout = new QHBoxLayout;
        horizontalLayout->addWidget(checkBox_Enable[i] = new QCheckBox("启用"));
        horizontalLayout->addWidget(new QLabel("典型频率(MHz):"));
        horizontalLayout->addWidget(lineEdit_TypicalFreq[i] = new QDoubleSpinBox);
        lineEdit_TypicalFreq[i]->setMinimum(0 + 0.2);
        lineEdit_TypicalFreq[i]->setMaximum(30 - 0.2);
        vBoxLayout->addLayout(horizontalLayout);
    }
    auto horizontalLayout = new QHBoxLayout;
    horizontalLayout->addStretch();
    horizontalLayout->addWidget(pushButton_Confirm = new QPushButton("确认"));
    connect(pushButton_Confirm, &QPushButton::clicked, this, [this] {
        m_lstValue.clear();
        for (auto i = 0; i < SETTING_LINE; ++i)
        {
            if(checkBox_Enable[i]->isChecked())
                m_lstValue.append(lineEdit_TypicalFreq[i]->text().toDouble() * 1e6);
        }
        //检查设置的典型频点彼此间是否在0.4mhz以上
        for(int index = 0; index < m_lstValue.length(); ++index){
            for(int otherIndex = 0; otherIndex < m_lstValue.length(); ++otherIndex){
                if(otherIndex == index){
                    continue;
                }
                if(std::abs(m_lstValue[index] - m_lstValue[otherIndex]) < 0.4e6){
                    QMessageBox::information(nullptr, "典型频点设置", "典型频点间距需大于0.4MHz，设置失败！");
                    return;
                }
            }
        }

        if(!m_lstValue.isEmpty()){
            emit sigHaveTypicalFreq(m_lstValue);
        }
        QMessageBox::information(nullptr, "典型频点设置", !m_lstValue.isEmpty()? "设置成功！": "无有效频点，设置失败！");
    });
    horizontalLayout->addWidget(pushButton_Cancel = new QPushButton("取消"));
    connect(pushButton_Cancel, &QPushButton::clicked, this, [this] {
        close();
    });
    vBoxLayout->addLayout(horizontalLayout);
}
