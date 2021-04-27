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
    QFileDownloader(const QUrl &_url, const QString &_targetpath, const QString &_rcname, QObject *_parent = nullptr);

    QString getUrl() const;

signals:
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void replyReady(int _httpcode,
                    QNetworkReply::NetworkError _err,
                    const QString &_errstring,
                    const QByteArray &_bindata,
                    const QString &_rcname,
                    const QString &_targetpath,
                    const QString &_filename);

protected:
    void run();

private:
    QUrl url;
    QString targetpath;
    QString rcname;
};

#endif // QFILEDOWNLOADER_H
