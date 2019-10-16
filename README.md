QSimpleMaintenanceTool
===

This repo contains simple Qt-based tool to install and update your applications on remote and local clients.

Original idea has been stolen from [alex-spataru](https://github.com/alex-spataru/QSimpleUpdater). In fact in first version .json files that both projects are used were the same. !But not now!, because it was not very convenient for me ) The major difference from [QSimpleUpdater](https://github.com/alex-spataru/QSimpleUpdater)
in smaller and lighter code without any GUI (cause you know, you can always draw your own, right). Also in QSimpleMaintenanceTool all file downloads are performed in separate threads (one thread per download), so QSimpleMaintenanceTool should not cause any freezes.  

**How to use**

To integrate maintenance tool into your app, just clone this repo and include "qsimplemaintenance.pri" into your project's .pro file.
 
Now, let's suppose that we host our software distribution server on *https://mysoftware.org* and want to update *myapp* application.
Then we should share on *https://mysoftware.org* access for maintenance file. Let's give it name "updates.json". it should contain maintenance info about our application/s:

```json
{
  "myapp": {
    "windows": [
      {
        "version": "1.0.1.0",
        "url": "https://mysoftware.org/setup_myapp_msvc14_x86.msi",
        "changelog": "This is an example changelog for Windows. Go on..."
      },
      {
       "version": "2.0.0.0",
        "url": "ftp://mysoftware.org/setup_myapp_mingw_x86_last.msi",
        "changelog": "This is an example changelog for Windows. Go on..."
      }
    ],
    "linux": {
      "i386": [
        {
          "version": "1.0.0.0",
          "url": "ftp://mysoftware.org/setup_myapp_gcc_x86_last.deb",
          "changelog": "This is an example changelog for Linux. Go on..."
        }
      ],
      "x86_64": [
        {
          "version": "1.0.0.0",
          "url": "https://mysoftware.org/setup_myapp_gcc_x86_64_last.deb",
          "changelog": "This is an example changelog for Linux. Go on..."
        }
      ]
    }
  },
  "otherapp": {
    "android": [
      {
        "version": "1.0.0.0",
        "url": "https://mysoftware.org/simplestappever.apk",
        "changelog": "This is an example changelog for Android. Go on..."
      }
    ]
  }
}
```

That is all. Now from our app we should just *check* this file and optionally perform *download* of desired update. Following code can be used as reference:

```C++
#include "qsimplemaintenancetool.h"
...
QSimpleMaintenanceTool smt;
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
    }
});
QObject::connect(&smt,&QSimpleMaintenanceTool::downloaded,[&a](const QString &_filename){
	qDebug() << "Downloads have been saved: " << _filename 
	         << " (" << QFileInfo(_filename).size() << " bytes)";
});
QObject::connect(&smt,&QSimpleMaintenanceTool::error,[&a](const QString &_error){
	qDebug() << _error;
});
smt.check("https://mysoftware.org/updates.json");
...
```

Check full code of how QSimpleMaintenanceTool suppose to be used in [samples](https://github.com/pi-null-mezon/QSimpleMaintenanceTool/tree/master/samples/ConsoleApp).

Not only resources with explicit version/platform are supported. QSimpleMaintenanceTool also handles following format:  

```
{
	"music": [
		"http://mediastorage.com/Muse/Aftermath.mp3",
		"http://mediastorage.com/TheRasmus/HolyGrail.mp3"
	  ],
	"images": [
		"ftp://mediastorage.com/Leonardo/MonaLisa.jpg"
	  ]
 }
```

Be cool and enjoy! 
