#ifndef QFILEDOWNLOADER_H
#define QFILEDOWNLOADER_H

#include <QNetworkReply>
#include <QThread>
#include <QUrl>

/**
 * @brief The QDownloader class should be used to perform GET requests in separate thread
 */
class QFileDownloader : public QThread
{
    Q_OBJECT
public:
    QFileDownloader(const QUrl &_url, QObject *_parent = nullptr);

    QString getUrl() const;

signals:
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void replyReady(int _httpcode, QNetworkReply::NetworkError _err, const QString &_errstring, const QByteArray &_downloads, const QString &_filename);

protected:
    void run();

private:
    QUrl url;
};

#endif // QFILEDOWNLOADER_H
