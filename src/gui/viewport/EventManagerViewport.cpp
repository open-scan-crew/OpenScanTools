#include "gui/viewport/EventManagerViewport.h"
#include "gui/Dialog/DialogImportAsciiPC.h"
#include <qmimedata.h>
#include <qevent.h>
#include <filesystem>
#include "controller/controls/ControlProject.h"


EventManagerViewport::EventManagerViewport(QWidget* viewW, IDataDispatcher& dispatcher)
	: m_dataDispatcher(dispatcher)
	, m_widget(viewW)
{
}

bool EventManagerViewport::eventFilter(QObject *object, QEvent *event)
{
	if (object == m_widget && event->type() == QEvent::Drop) 
	{
		QDropEvent *dropEvent = static_cast<QDropEvent *>(event);
		this->dropEvent(dropEvent);
		return true;
	}
	return false;
}

void EventManagerViewport::dropEvent(QDropEvent* event)
{
	const QMimeData* mimeData = event->mimeData();

	// check for our needed mime type, here a file or a list of files
	if (mimeData->hasUrls())
	{
		std::vector<std::filesystem::path> pathList;
		QList<QUrl> urlList = mimeData->urls();

		// extract the local paths of the files
		for (int i = 0; i < urlList.size(); i++)
		{
			std::filesystem::path file(urlList.at(i).toLocalFile().toStdWString());
			if (file.extension() == (".tlp"))
				m_dataDispatcher.sendControl(new control::project::DropLoad(file));
			else
				pathList.push_back(file);
		}

		DialogImportAsciiPC importAsciiDialog(m_widget);
		if (importAsciiDialog.setInfoAsciiPC(pathList))
			importAsciiDialog.exec();

		std::map<std::filesystem::path, Import::AsciiInfo> mapAsciiInfo;
		if (importAsciiDialog.isFinished())
			mapAsciiInfo = importAsciiDialog.getFileImportInfo();

		Import::ScanInfo info;
		info.asObject = false;
		info.paths = pathList;
		info.mapAsciiInfo = mapAsciiInfo;

		// call a function to open the files
		if(pathList.size())
			m_dataDispatcher.sendControl(new control::project::ImportScan(info));
	}
}