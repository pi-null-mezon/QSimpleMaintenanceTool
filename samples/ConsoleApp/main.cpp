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
        qDebug("Application version: %s\n-----", APP_VERSION);
        QObject::connect(&smt,&QSimpleMaintenanceTool::checked,[&smt,&a](const QList<smt::Version> &_versions){
            // If this slot is called then _versions list is not empty
            qDebug() << "Available versions in storage: ";
            for(const auto &_version : _versions)
                qDebug() << _version.version << " - "
                         << _version.changelog
                         << "(" << _version.url << ")";
            const smt::Version &_lastversion = _versions.at(0); // greatest available
            if(_lastversion.version > APP_VERSION) {
                smt.download(_lastversion.url);
                // optional part
                QObject::connect(&smt,&QSimpleMaintenanceTool::downloadProgress,[](const QString &_url,
                                                                                   qint64 bytesReceived,
                                                                                   qint64 bytesTotal){
                    Q_UNUSED(_url) // _url is reserved for the cases when multiple files are downloaded simulateneously
                    static uint8_t progress = 0, _maxbins = 20;
                    const uint8_t _tmp = static_cast<uint8_t>(_maxbins * static_cast<float>(bytesReceived) / bytesTotal);
                    if(_tmp > progress) {
                        for(uint8_t i = 0; i < (_tmp - progress); ++i)
                            std::cout << '.' << std::flush;
                        progress = _tmp;
                    }
                    if(_tmp == _maxbins)
                        std::cout << std::endl;
                });
                // end of optional part
            } else {
                qDebug("Current version greater or equal to the latest");
                a.quit();
            }
        });
        QObject::connect(&smt,&QSimpleMaintenanceTool::downloaded,[&a](const QString &_filename){
            qDebug() << "Downloads have been saved: " << _filename
                     << " (" << QFileInfo(_filename).size() << " bytes )";
            a.quit();
        });
        QObject::connect(&smt,&QSimpleMaintenanceTool::error,[&a](const QString &_error){
            qDebug() << _error;
            a.quit();
        });
        smt.check(cmdparser.value(checkurlOption));
    } else {
        cmdparser.showHelp();
    }

    return a.exec();
}
