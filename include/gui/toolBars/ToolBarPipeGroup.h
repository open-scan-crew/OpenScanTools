#ifndef TOOLBAR_PIPEGROUP_H
#define TOOLBAR_PIPEGROUP_H

#include <QtWidgets/QWidget>
#include "ui_toolbar_pipegroup.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"
#include "models/application/List.h"
#include "controller/functionSystem/PipeDetectionOptions.h"
#include "models/OpenScanToolsModelEssentials.h"

class ToolBarPipeGroup;

typedef void (ToolBarPipeGroup::* PipeGroupMethod)(IGuiData*);

class ToolBarPipeGroup : public QWidget, public IPanel
{
	Q_OBJECT

public:
	explicit ToolBarPipeGroup(IDataDispatcher& dataDispatcher, QWidget* parent, float scale);

	void informData(IGuiData* data);

private:
	~ToolBarPipeGroup();
	void onProjectLoad(IGuiData* data);
	void onActivateFunction(IGuiData* data);
	void onStandardRecieved(IGuiData* data);

	void initPipeDetection();
	void initLargePipeDetection();
	void initPipeStandardManagement();
	void slotStandardChanged(const int& index);
	void extensionStateChanged(int state, QCheckBox* active);

	void updateDetectionOptions();

private:
	std::unordered_map<guiDType, PipeGroupMethod> m_methods;
	Ui::toolbar_pipegroup m_ui;
	IDataDispatcher& m_dataDispatcher;
	std::vector<SafePtr<StandardList>> m_standardsElems;
};

#endif // TOOLBAR_PIPEGROUP_H

