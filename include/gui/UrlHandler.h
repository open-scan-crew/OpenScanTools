#ifndef URL_HANDLER_H_
#define URL_HANDLER_H_

#include <QObject>
#include <filesystem>

class FileUrlHandler : public QObject
{
	Q_OBJECT
public slots:
	void handleFile(const QUrl& url);
public:
	QString m_projectPath;
};

#endif