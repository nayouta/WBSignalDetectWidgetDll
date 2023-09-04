#ifndef _TINY_CONFIG_H
#define _TINY_CONFIG_H

#include <QApplication>
#include <QSettings>

class TinyConfig
{
public:
    TinyConfig();
    TinyConfig(const TinyConfig&) = default;
    TinyConfig& operator=(const TinyConfig&) = default;
    TinyConfig(const QString&);
    QString Get_DataIP();
    unsigned short Get_DataPort();
    void Set_Data(QString&, int&);

private:
    static constexpr char const* DEFAULT_SERVER_DATA_IP = "127.0.0.1";
    static constexpr int DEFAULT_SERVER_DATA_PORT = 5021;
    static constexpr char const* Data_Area = "Data";

    QSettings* m_psetting = nullptr;
    QString DataIP;
    unsigned short DataPort = 0;
    QVariant GetConfigData(QString, QString);
    void SetConfigData(QString, QString, QVariant);
    bool isIpAddr(const QString&);
};

#endif
