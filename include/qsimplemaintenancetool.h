#ifndef QSIMPLEMAINTENANCETOOL_H
#define QSIMPLEMAINTENANCETOOL_H

#include <QNetworkReply>
#include <QObject>
#include <QThread>
#include <QUrl>

/**
 * @brief The QSimpleMaintenanceTool class should be used to check and download updates
 */
class QSimpleMaintenanceTool : public QObject
{
    Q_OBJECT
public:
    explicit QSimpleMaintenanceTool(QObject *parent = nullptr);

signals:
    void error(const QString &_message);
    void checkProgress(qint64 bytesReceived, qint64 bytesTotal);
    void checked(const QString &_openurl, const QString &_latestversion, const QString &_downloadurl, const QString &_changelog, bool _mandatory);
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void downloaded(const QString &_filename);

public slots:
    /**
     * @brief run to check maintenance info for particular resource
     * @param _url (example http://mypublichosting.org/myappinfo.json)
     */
    void check(const QString &_url);
    /**
     * @brief run to download update file
     * @note use in conjunction with 'downloadLink(...)' signal
     * @param _url (example http://mypublichosting.org/)
     */
    void download(const QString &_url);

private slots:
    void __check(int _httpcode, QNetworkReply::NetworkError _err, const QString &_errstring, const QByteArray &_jsondata, const QString &_filename);
    void __download(int _httpcode, QNetworkReply::NetworkError _err, const QString &_errstring, const QByteArray &_downloads, const QString &_filename);
};

#endif // QSIMPLEMAINTENANCETOOL_H
