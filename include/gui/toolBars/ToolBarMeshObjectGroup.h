#ifndef TOOLBAR_WAVEFRONT_GROUP_H
#define TOOLBAR_WAVEFRONT_GROUP_H

#include <QtWidgets/QWidget>
#include "gui/Dialog/DialogImportMeshObject.h"
#include "ui_toolbar_meshobjectgroup.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"
#include "utils/Color32.hpp"

class ToolBarMeshObjectGroup;

typedef void (ToolBarMeshObjectGroup::* WavefrontGroupMethod)(IGuiData*);

class ToolBarMeshObjectGroup : public QWidget, public IPanel
{
	Q_OBJECT

public:
	explicit ToolBarMeshObjectGroup(IDataDispatcher& dataDispatcher, QWidget* parent, float scale);

	void informData(IGuiData* data) override;

private:
	void onFunctionActived(IGuiData* data);
	void onProjectLoad(IGuiData* data);

public slots:
	void clickWavefrontProperties();
	void clickFromFile();
	void clickCopy();

private:
	~ToolBarMeshObjectGroup();

private:
	std::unordered_map<guiDType, WavefrontGroupMethod> m_methods;
	QString   m_openPath;
	Ui::toolbar_meshObjectGroup m_ui;
	IDataDispatcher& m_dataDispatcher;

	DialogImportMeshObject m_dialog;
};

#endif // TOOLBAR_WAVEFRONT_GROUP_H

