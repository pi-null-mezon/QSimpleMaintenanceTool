#ifndef QSIMPLEMAINTENANCETOOL_H
#define QSIMPLEMAINTENANCETOOL_H

#include <QCoreApplication>
#include <QStandardPaths>
#include <QNetworkReply>
#include <QObject>
#include <QThread>
#include <QUrl>

namespace smt {
    struct Version {
        Version(const QString &_version, const QString &_url, const QString &_changelog):
            version(_version), url(_url), changelog(_changelog) {}

        QString version, url, changelog;
    };
}

/**
 * @brief The QSimpleMaintenanceTool class should be used to check and download updates
 */
class QSimpleMaintenanceTool : public QObject
{
    Q_OBJECT
public:
    explicit QSimpleMaintenanceTool(const QString &_appname=QCoreApplication::applicationName(), QObject *parent = nullptr);

signals:
    void error(const QString &_message);
    void checkProgress(qint64 bytesReceived, qint64 bytesTotal);
    /**
     * @brief checked, this signal is emitted when version/s for target product is found
     * @param _versions - list of available versions ordered descending
     */
    void checked(const QList<smt::Version> &_versions);
    /**
     * @brief files, this signal is emitted when version/s for target product is found
     * @param _urls - list of urls to download platform- and version-less files such as music, pictures, images etc.
     */
    void files(const QStringList &_urls);
    void downloadProgress(const QString _url, qint64 bytesReceived, qint64 bytesTotal);
    void downloaded(const QString &_filename);

public slots:
    /**
     * @brief run to check maintenance info for particular resource
     * @param _url (example http://mypublichosting.org/myappinfo.json)
     * @param _rcname aka resource name, if not empty _rcname will be checked instead of appname
     */
    void check(const QString &_url, const QString &_rcname=QString());
    /**
     * @brief run to download update file
     * @note use in conjunction with 'downloadLink(...)' signal
     * @param _url (example http://mypublichosting.org/)
     * @param _forcedownload - perform downloading even if file already exists in _targetpath
     */
    void download(const QString &_url, const QString &_targetpath=QStandardPaths::writableLocation(QStandardPaths::DownloadLocation), const bool _forcedownload=false);

private slots:
    void __check(int _httpcode, QNetworkReply::NetworkError _err, const QString &_errstring, const QByteArray &_jsondata, const QString &_filename);
    void __download(int _httpcode, QNetworkReply::NetworkError _err, const QString &_errstring, const QByteArray &_downloads, const QString &_filename);
    void __updateDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);

private:
    QString appname, platform, cpuarch, targetpath, rcname;
};

#endif // QSIMPLEMAINTENANCETOOL_H
