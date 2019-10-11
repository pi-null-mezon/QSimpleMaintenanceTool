#include <iostream>

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QFileInfo>

#include "qsimplemaintenancetool.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName(APP_NAME);
    QCoreApplication::setApplicationVersion(APP_VERSION);

    QCommandLineParser cmdparser;
    cmdparser.setApplicationDescription("Sample of how to use QSimpleMaintenanceTool");
    cmdparser.addHelpOption();
    cmdparser.addVersionOption();
    QCommandLineOption checkurlOption(QStringList() << "u" << "url", "url of the server wher maintenance info resides", "http:/...");
    cmdparser.addOption(checkurlOption);
    cmdparser.process(a);

    QSimpleMaintenanceTool smt;
    if(cmdparser.isSet(checkurlOption)) {
        QObject::connect(&smt,&QSimpleMaintenanceTool::checked,[&smt,&a](const QString &_openurl, const QString &_latestversion, const QString &_downloadurl, const QString &_changelog, bool _mandatory){
            qDebug() << "Maintenance check has been performed: latest-version in storage "
                     << _latestversion
                     << " ( download-url: " << _downloadurl << ")";
            if(_latestversion > APP_VERSION) {
                qDebug("Local version (%s) is lower than latest (%s) in storage. Update needed", APP_VERSION,_latestversion.toUtf8().constData());
                smt.download(_downloadurl);
                QObject::connect(&smt,&QSimpleMaintenanceTool::downloadProgress,[](qint64 bytesReceived, qint64 bytesTotal){
                    static uint8_t progress = 0;
                    const uint8_t _maxbins = 20;
                    const uint8_t _tmp = _maxbins * static_cast<float>(bytesReceived) / bytesTotal;
                    if(_tmp > progress) {
                        for(uint8_t i = 0; i < (_tmp - progress); ++i)
                            std::cout << '.';
                        progress = _tmp;
                    }
                    if(_tmp == _maxbins)
                        std::cout << std::endl;
                });
            } else {
                qDebug("Local version (%s) is greater than latest (%s) in storage. No update needed", APP_VERSION,_latestversion.toUtf8().constData());
                a.quit();
            }
        });
        QObject::connect(&smt,&QSimpleMaintenanceTool::downloaded,[&a](const QString &_filename){
            QFileInfo _finfo(_filename);
            qDebug() << "Downloads has been received: " << _filename
                     << " (" << _finfo.size() << " bytes)";
            a.quit();
        });
        QObject::connect(&smt,&QSimpleMaintenanceTool::error,[&a](const QString &_error){
            qDebug() << _error;
            a.quit();
        });
        smt.check(cmdparser.value(checkurlOption));
    } else {
        cmdparser.showHelp();
        a.quit();
    }

    return a.exec();
}
