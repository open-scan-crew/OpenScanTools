#include "gui/UrlHandler.h"
#include <QtCore/qdir.h>
#include <QtCore/qurl.h>
#include "qdesktopservices.h"

void FileUrlHandler::handleFile(const QUrl& fileUrl)
{
	QUrl url;
	if (!QDir::isAbsolutePath(fileUrl.path()))
	{
		QUrl relUrl;
		if (fileUrl.isLocalFile())
			relUrl = fileUrl.toLocalFile();
		url = QUrl::fromLocalFile(m_projectPath);
		url = url.resolved(relUrl);
	}
	else
		url = fileUrl;

	QDesktopServices::openUrl(url);
}