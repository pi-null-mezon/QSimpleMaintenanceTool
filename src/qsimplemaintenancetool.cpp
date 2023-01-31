#include "qsimplemaintenancetool.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>

#include <QDir>
#include <QFile>
#include <QSysInfo>
#include <QFileInfo>

#include "qfiledownloader.h"

QSimpleMaintenanceTool::QSimpleMaintenanceTool(const QString &_appname, QObject *parent) : QObject(parent),
    appname(_appname)
{
    platform = "unknown";
#if defined Q_OS_WIN
    platform = "windows";
#elif defined Q_OS_MAC
    platform = "osx";
#elif defined Q_OS_ANDROID
    platform = "android";
#elif defined Q_OS_LINUX
    platform = "linux";
#elif defined Q_OS_IOS
    platform = "ios";
#endif
    cpuarch = QSysInfo::buildCpuArchitecture();
    qInfo("QSimpleMaintenanceTool target platform: '%s' and cpuarch: '%s'",
          platform.toUtf8().constData(), cpuarch.toUtf8().constData());
}

void QSimpleMaintenanceTool::check(const QString &_url, const QString &_rcname)
{
    QFileDownloader *_thread = new QFileDownloader(QUrl::fromUserInput(_url),QString(),_rcname);
    connect(_thread,SIGNAL(downloadProgress(qint64,qint64)),this,SIGNAL(checkProgress(qint64,qint64)));
    connect(_thread,SIGNAL(replyReady(int,QNetworkReply::NetworkError,QString,QByteArray,QString,QString,QString)),
               this,SLOT(__check(int,QNetworkReply::NetworkError,QString,QByteArray,QString,QString,QString)));
    connect(_thread,SIGNAL(finished()),_thread,SLOT(deleteLater()));
    _thread->start();
}

void QSimpleMaintenanceTool::__check(int _httpcode, QNetworkReply::NetworkError _err, const QString &_errstring, const QByteArray &_jsondata, const QString &_rcname, const QString &_targetpath, const QString &_filename)
{
    Q_UNUSED(_targetpath)
    Q_UNUSED(_filename)
    Q_UNUSED(_httpcode)
    //qDebug("HTTP code [%d] has been recieved in QSimpleMaintenanceTool::__check()", _httpcode);
    if(_err == QNetworkReply::NetworkError::NoError) {
        if(_jsondata.size() > 0) {
            QJsonParseError _jperr;
            QJsonObject _json = QJsonDocument::fromJson(_jsondata,&_jperr).object();
            if(_jperr.error != QJsonParseError::NoError)
                emit error(UpdatesFileParsingError,tr("Maintenance check failed: '%1'").arg(_jperr.errorString()));
            else {
                if(_rcname.isEmpty()) {
                    if(_json.contains(appname)) {
                        _json = _json.value(appname).toObject();
                        if(_json.contains(platform)) {
                            QJsonObject _jtmp = _json.value(platform).toObject();
                            QJsonArray _jsonarray;
                            if(_jtmp.contains(cpuarch))
                                _jsonarray = _jtmp.value(cpuarch).toArray();
                            else
                                _jsonarray = _json.value(platform).toArray();
                            if(_jsonarray.size() > 0) {
                                QList<smt::Version> _versions;
                                _versions.reserve(_jsonarray.size());
                                for(int i = 0; i < _jsonarray.size(); ++i) {
                                    _json = _jsonarray.at(i).toObject();
                                    _versions.push_back(smt::Version(_json.value("version").toString(),
                                                                     _json.value("url").toString(),
                                                                     _json.value("changelog").toString()));
                                }
                                std::sort(_versions.begin(),_versions.end(),[](const smt::Version &_l,const smt::Version &_r){
                                    return _l.version > _r.version;
                                });
                                emit checked(_versions);
                            } else emit error(NoUpdatesFound,tr("Maintenance check failed: no available versions found"));
                        } else emit error(NoUpdatesFound,tr("Maintenance check failed: '%1' section not found").arg(platform));
                    } else emit error(NoUpdatesFound,tr("Maintenance check failed: '%1' section not found").arg(appname));
                } else { // branch for files without platform and version
                    if(_json.contains(_rcname)) {
                        QJsonArray _jsonarray = _json.value(_rcname).toArray();
                        QStringList _urls;
                        _urls.reserve(_jsonarray.size());
                        for(int i = 0; i < _jsonarray.size(); ++i)
                            _urls.push_back(_jsonarray.at(i).toString());
                        emit files(_urls);
                    } else emit error(NoResourcesFound,tr("Maintenance check failed: '%1' section not found").arg(_rcname));
                }
            }
        } else emit error(NoUpdatesFound,tr("Maintenance check failed: empty maintenance file"));
    } else emit error(NetworkError,tr("Maintenance check failed: '%1'").arg(_errstring));
}

void QSimpleMaintenanceTool::download(const QString &_url, const QString &_targetpath, const bool _forcedownload)
{
    QDir _dir(_targetpath);
    if(!_dir.exists())
        _dir.mkpath(_dir.absolutePath());
    const QString _targetname = QString("%1/%2").arg(_targetpath,_url.section('/',-1));
    if(!_forcedownload && QFileInfo(_targetname).exists())
        emit downloaded(_targetname);
    else {
        QFileDownloader *_thread = new QFileDownloader(QUrl::fromUserInput(_url),_targetpath,QString());
        connect(_thread,SIGNAL(downloadProgress(qint64,qint64)),this,SLOT(__updateDownloadProgress(qint64, qint64)));
        connect(_thread,SIGNAL(replyReady(int,QNetworkReply::NetworkError,QString,QByteArray,QString,QString,QString)),
                   this,SLOT(__download(int,QNetworkReply::NetworkError,QString,QByteArray,QString,QString,QString)));
        connect(_thread,SIGNAL(finished()),_thread,SLOT(deleteLater()));
        _thread->start();
    }
}

void QSimpleMaintenanceTool::__updateDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    emit downloadProgress(qobject_cast<QFileDownloader*>(sender())->getUrl(),bytesReceived,bytesTotal);
}

void QSimpleMaintenanceTool::__download(int _httpcode, QNetworkReply::NetworkError _err, const QString &_errstring, const QByteArray &_downloads, const QString &_rcname, const QString &_targetpath, const QString &_filename)
{
    Q_UNUSED(_httpcode)
    Q_UNUSED(_rcname)
    //qDebug("HTTP code [%d] has been recieved in QSimpleMaintenanceTool::__download()", _httpcode);
    if(_err == QNetworkReply::NetworkError::NoError) {
        if(_downloads.size() > 0) {
            const QString _targetname = QString(_targetpath).append("/%1").arg(_filename);
            QFile _file(_targetname);
            if(_file.exists())
                if(QFile::remove(_targetname) == false) {
                    emit error(RemoveFileError,tr("Maintenance download failed: file already exists on local machine and can not be removed"));
                    return;
                }
            if(_file.open(QIODevice::WriteOnly)) {
                _file.write(_downloads);
                _file.close();
                emit downloaded(_targetname);
            } else
                emit error(WriteToFileError,tr("Maintenance download failed: can not open file for write"));
        } else
            emit error(ZeroSizeDownload,tr("Maintenance download failed: zero size downloads"));
    } else
        emit error(NetworkError,tr("Maintenance download failed: '%1'").arg(_errstring));
}


