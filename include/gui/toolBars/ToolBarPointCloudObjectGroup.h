#ifndef TOOLBAR_POINT_CLOUD_OBJECT_GROUP_H
#define TOOLBAR_POINT_CLOUD_OBJECT_GROUP_H

#include <QtWidgets/QWidget>
#include "ui_toolbar_pcobjectgroup.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"
#include "utils/Color32.hpp"

#include "gui/Dialog/DialogImportPCObject.h"

class ToolBarPointCloudObjectGroup;

typedef void (ToolBarPointCloudObjectGroup::* PCOGroupMethod)(IGuiData*);

class ToolBarPointCloudObjectGroup : public QWidget, public IPanel
{
	Q_OBJECT

public:
	explicit ToolBarPointCloudObjectGroup(IDataDispatcher& dataDispatcher, QWidget* parent, float scale);

	void informData(IGuiData* data) override;

private:
	void onFunctionActived(IGuiData* data);
	void onProjectLoad(IGuiData* data);
	void onProjectPath(IGuiData* data);

public slots:
	void clickPointCloudObjectProperties();
	void clickFromFile();
	void clickFromBox();
	void clickCopy();

private:
	~ToolBarPointCloudObjectGroup();

private:
	std::unordered_map<guiDType, PCOGroupMethod> m_methods;
	QString   m_openPath;
	Ui::toolbar_pcObjectGroup m_ui;
	IDataDispatcher& m_dataDispatcher;

	DialogImportPCObject m_dialog;
};

#endif // TOOLBAR_POINT_CLOUD_OBJECT_GROUP_H

