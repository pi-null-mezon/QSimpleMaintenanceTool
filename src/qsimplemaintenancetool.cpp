#include "qsimplemaintenancetool.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>

#include <QDir>
#include <QFile>
#include <QFileInfo>

#include "qfiledownloader.h"

QSimpleMaintenanceTool::QSimpleMaintenanceTool(QObject *parent) : QObject(parent)
{
}

void QSimpleMaintenanceTool::check(const QString &_url)
{
    QFileDownloader *_thread = new QFileDownloader(QUrl::fromUserInput(_url));
    connect(_thread,SIGNAL(downloadProgress(qint64,qint64)),this,SIGNAL(checkProgress(qint64,qint64)));
    connect(_thread,SIGNAL(replyReady(int,QNetworkReply::NetworkError,QString,QByteArray,QString)),
               this,SLOT(__check(int,QNetworkReply::NetworkError,QString,QByteArray,QString)));
    connect(_thread,SIGNAL(finished()),_thread,SLOT(deleteLater()));
    _thread->start();
}

void QSimpleMaintenanceTool::__check(int _httpcode, QNetworkReply::NetworkError _err, const QString &_errstring, const QByteArray &_jsondata, const QString &_filename)
{
    Q_UNUSED(_filename)
    //qDebug("HTTP code [%d] has been recieved in QSimpleMaintenanceTool::__check()", _httpcode);
    if(_err == QNetworkReply::NetworkError::NoError) {
        if(_jsondata.size() > 0) {
            QJsonParseError _jsonparseerr;
            QJsonObject _json = QJsonDocument::fromJson(_jsondata,&_jsonparseerr).object();
            if(_jsonparseerr.error != QJsonParseError::NoError)
                emit error(tr("Maintenance check failed: '%1'").arg(_jsonparseerr.errorString()));
            else {
                if(_json.contains("updates")) {
                    QString _platform = "unknown";
                    #if defined Q_OS_WIN
                        _platform = "windows";
                    #elif defined Q_OS_MAC
                        _platform = "osx";
                    #elif defined Q_OS_LINUX
                        _platform = "linux";
                    #elif defined Q_OS_ANDROID
                        _platform = "android";
                    #elif defined Q_OS_IOS
                        _platform = "ios";
                    #endif
                    _json = _json.value("updates").toObject();
                    if(_json.contains(_platform)) {
                        _json = _json.value(_platform).toObject();
                        emit checked(_json.value("open-url").toString(),
                                     _json.value("latest-version").toString(),
                                     _json.value("download-url").toString(),
                                     _json.value("changelog").toString(),
                                     _json.value("mandatory").toBool());
                    }
                    else
                        emit error(tr("Maintenance check failed: '%1' section not found").arg(_platform));
                } else
                    emit error(tr("Maintenance check failed: 'updates' section not found"));
            }
        } else
            emit error(tr("Maintenance check failed: empty maintenance info file"));
    } else
        emit error(tr("Maintenance check failed: '%1'").arg(_errstring));
}

void QSimpleMaintenanceTool::download(const QString &_url)
{
    QFileDownloader *_thread = new QFileDownloader(QUrl::fromUserInput(_url));
    connect(_thread,SIGNAL(downloadProgress(qint64,qint64)),this,SIGNAL(downloadProgress(qint64,qint64)));
    connect(_thread,SIGNAL(replyReady(int,QNetworkReply::NetworkError,QString,QByteArray,QString)),
               this,SLOT(__download(int,QNetworkReply::NetworkError,QString,QByteArray,QString)));
    connect(_thread,SIGNAL(finished()),_thread,SLOT(deleteLater()));
    _thread->start();
}

void QSimpleMaintenanceTool::__download(int _httpcode, QNetworkReply::NetworkError _err, const QString &_errstring, const QByteArray &_downloads, const QString &_filename)
{
    Q_UNUSED(_httpcode)
    //qDebug("HTTP code [%d] has been recieved in QSimpleMaintenanceTool::__download()", _httpcode);
    if(_err == QNetworkReply::NetworkError::NoError) {
        if(_downloads.size() > 0) {
            const QString _targetname = QDir::home().absolutePath().append("/%1").arg(_filename);
            QFile _file(_targetname);
            if(_file.exists())
                if(QFile::remove(_targetname) == false) {
                    emit error("Maintenance download failed: file already exists on local machine and can not be removed");
                    return;
                }
            if(_file.open(QIODevice::WriteOnly)) {
                _file.write(_downloads);
                _file.close();
                emit downloaded(_targetname);
            } else
                emit error("Maintenance download failed: can not open file for write");
        } else
            emit error("Maintenance download failed: zero size downloads");
    } else
        emit error(tr("Maintenance download failed: '%1'").arg(_errstring));
}