#ifndef IMPORT_FILEOBJECT_DIALOG_H_
#define IMPORT_FILEOBJECT_DIALOG_H_

#include "gui/Dialog/ADialog.h"
#include "ui_DialogImportFileObject.h"

#include "gui/IDataDispatcher.h"
#include "models/graph/AGraphNode.h"

#include <unordered_set>

class DialogImportFileObject : public ADialog
{
	Q_OBJECT

public:
	explicit DialogImportFileObject(IDataDispatcher& dataDispatcher, QWidget* parent = 0);
	~DialogImportFileObject();

	void setInfoFileObjects(const std::unordered_set<SafePtr<AGraphNode>>& infoFileObjects, bool isOnlyLink);

	virtual void informData(IGuiData* keyValue);

public slots:
	void ImportFiles();
	void Ignore();
	void Cancel();

private:
	std::unordered_set<xg::Guid> m_toImportFileObjects;
	QString m_openPath;

	Ui::ImportFileObject m_ui;
};

#endif // !DELETE_TYPE_DIALOG_H_