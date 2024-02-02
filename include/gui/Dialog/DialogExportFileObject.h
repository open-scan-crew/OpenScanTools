#ifndef EXPORT_FILEOBJECT_DIALOG_H_
#define EXPORT_FILEOBJECT_DIALOG_H_

#include <QtWidgets/QListView>
#include <qdialog.h>
#include "utils/Logger.h"
#include "ui_DialogExportFileObject.h"
#include "models/data/Data.h"

#include "gui/IDataDispatcher.h"

#include <unordered_set>

#define GUILOG Logger::log(LoggerMode::GuiLog)

class DialogExportFileObject : public QDialog
{
	Q_OBJECT

public:
	explicit DialogExportFileObject(IDataDispatcher& dataDispatcher, std::unordered_map<std::wstring, std::wstring> infoFileObjects, QWidget *parent = 0);
	~DialogExportFileObject();


public slots:

	void Ok();
	void Cancel();

private:
	IDataDispatcher& m_dataDispatcher;

	Ui::ExportFileObject m_ui;
};

#endif // !DELETE_TYPE_DIALOG_H_