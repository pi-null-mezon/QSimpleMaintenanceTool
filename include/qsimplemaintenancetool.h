#ifndef QSIMPLEMAINTENANCETOOL_H
#define QSIMPLEMAINTENANCETOOL_H

#include <QCoreApplication>
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
     * @brief checked
     * @param _versions - list of available versions ordered descending
     */
    void checked(const QList<smt::Version> &_versions);
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

private:
    QString appname, platform, cpuarch;
};

#endif // QSIMPLEMAINTENANCETOOL_H
