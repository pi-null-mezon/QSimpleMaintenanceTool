#include "qfiledownloader.h"

QFileDownloader::QFileDownloader(const QUrl &_url, QObject *_parent): QThread(_parent), url(_url)
{
}

void QFileDownloader::run()
{
    QNetworkAccessManager _manager;
    qDebug("GET %s",url.toString().toUtf8().constData());
    QNetworkReply *_reply = _manager.get(QNetworkRequest(url));
    connect(_reply,SIGNAL(finished()),this,SLOT(quit()));
    connect(_reply,SIGNAL(downloadProgress(qint64,qint64)),this,SIGNAL(downloadProgress(qint64,qint64)));
    exec();
    emit replyReady(_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(),
                    _reply->error(),
                    _reply->errorString(),
                    _reply->readAll(),
                    url.toString().section('/',-1));
    _reply->deleteLater();
}
