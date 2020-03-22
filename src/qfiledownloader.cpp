#include "qfiledownloader.h"

QFileDownloader::QFileDownloader(const QUrl &_url, QObject *_parent): QThread(_parent), url(_url)
{
}

void QFileDownloader::run()
{
    QNetworkAccessManager _manager;    
    QNetworkReply *_reply = _manager.get(QNetworkRequest(url));
    connect(_reply,SIGNAL(finished()),this,SLOT(quit()));
    connect(_reply,SIGNAL(downloadProgress(qint64,qint64)),this,SIGNAL(downloadProgress(qint64,qint64)));
    exec();
    const int _httpcode = _reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug("GET %s [reply %d]",url.toString().toUtf8().constData(),_httpcode);
    emit replyReady(_httpcode,
                    _reply->error(),
                    _reply->errorString(),
                    _reply->readAll(),
                    url.toString().section('/',-1));
    _reply->deleteLater();
}

QString QFileDownloader::getUrl() const
{
    return url.toString();
}
