QSimpleMaintenanceTool
===

This repo contains simple Qt-based tool to install and update your applications on remote and local clients.

Original idea has been stolen from [alex-spataru](https://github.com/alex-spataru/QSimpleUpdater). In fact .json files that both projects are used are the same. The major difference from [QSimpleUpdater](https://github.com/alex-spataru/QSimpleUpdater)
in smaller and lighter code without any GUI (cause you know, you can always draw your own, right) and
all network downloads in QSimpleMaintenanceTool are performed in separate threads.

**How to use**

To integrate maintenance tool into your app, just clone this repo and include "qsimplemaintenance.pri" into your project's .pro file.
 
Now, let's suppose that we host our software distribution server on *https://mysoftware.org* and want to update *myapp* application.
Then we should share on *https://mysoftware.org* access for maintenance file. Let's give it name "myapp.json". it should contain maintenance info about our application (all thanks goes to [alex-spataru](https://github.com/alex-spataru/QSimpleUpdater)):

```json
{
  "updates": {
    "windows": {
      "open-url": "",
      "latest-version": "1.0.1.0",
      "download-url": "https://mysoftware.org/setup_myapp_mingw_x86_last.msi",
      "changelog": "This is an example changelog for Windows. Go on...",
      "mandatory": true
    },
    "linux": {
      "open-url": "",
      "latest-version": "1.0.0.0",
      "download-url": "https://mysoftware.org/setup_myapp_gcc_x86_64_last.deb",
      "changelog": "This is an example changelog for Linux. Go on...",
      "mandatory": true
    }
}
```

Ok, that is all!

```C++
#include "qsimplemaintenancetool.h"
...
QSimpleMaintenanceTool smt;
QObject::connect(&smt,&QSimpleMaintenanceTool::checked,[&smt](const QString &_openurl,
																										const QString &_latestversion,
																										const QString &_downloadurl,
																										const QString &_changelog,
																										bool _mandatory){
	qDebug() << "Latest-version in storage " << _latestversion;
	if(_latestversion > APP_VERSION) { // here APP_VERSION is a macro defined at compile time
		smt.download(_downloadurl);
		// optional progress printing part
		QObject::connect(&smt,&QSimpleMaintenanceTool::downloadProgress,[](qint64 bytesReceived,
																														 qint64 bytesTotal){
			static uint8_t progress = 0, _maxbins = 20;
			const uint8_t _tmp = _maxbins * static_cast<float>(bytesReceived) / bytesTotal;
			if(_tmp > progress) {
				for(uint8_t i = 0; i < (_tmp - progress); ++i)
					std::cout << '.';
				progress = _tmp;
			}
			if(_tmp == _maxbins)
				std::cout << std::endl;
		});
		// end of optional part
	}
});
QObject::connect(&smt,&QSimpleMaintenanceTool::downloaded,[&a](const QString &_filename){
	QFileInfo _finfo(_filename);
	qDebug() << "Downloads has been saved: " << _filename  << " (" << _finfo.size() << " bytes)";
});
QObject::connect(&smt,&QSimpleMaintenanceTool::error,[&a](const QString &_error){
	qDebug() << _error;
});
smt.check("https://mysoftware.org/myapp.json");
...
```

Check full code of how QSimpleMaintenanceTool suppose to be used in [samples](https://github.com/pi-null-mezon/QSimpleMaintenanceTool/tree/master/samples/ConsoleApp). Enjoy!