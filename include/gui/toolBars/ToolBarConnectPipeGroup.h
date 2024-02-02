#ifndef TOOLBAR_CONNECTPIPEGROUP_H
#define TOOLBAR_CONNECTPIPEGROUP_H

#include <QtWidgets/QWidget>
#include "ui_toolbar_connectpipegroup.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"
#include "models/application/List.h"
#include "controller/functionSystem/PipeDetectionOptions.h"

class ToolBarConnectPipeGroup;

typedef void (ToolBarConnectPipeGroup::* ConnectPipeGroupMethod)(IGuiData*);

class ToolBarConnectPipeGroup : public QWidget, public IPanel
{
	Q_OBJECT

public:
	explicit ToolBarConnectPipeGroup(IDataDispatcher& dataDispatcher, QWidget* parent, float scale);

	void informData(IGuiData* data);

private:
	~ToolBarConnectPipeGroup();
	void onProjectLoad(IGuiData* data);
	void onActivateFunction(IGuiData* data);
	void initConnectPipe();
	//void extensionStateChanged(int state, QCheckBox* active);

private:
	std::unordered_map<guiDType, ConnectPipeGroupMethod> m_methods;
	Ui::toolbar_connectpipegroup m_ui;
	IDataDispatcher& m_dataDispatcher;
};

#endif // TOOLBAR_CONNECTPIPEGROUP_H

