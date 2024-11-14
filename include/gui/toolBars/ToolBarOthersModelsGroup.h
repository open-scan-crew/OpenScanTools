#ifndef TOOLBAR_OTHER_MODEL_H
#define TOOLBAR_OTHER_MODEL_H

#include <QtWidgets/qwidget.h>
#include "ui_toolbar_othersmodelsgroup.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"

class ToolBarOthersModelsGroup;

typedef void (ToolBarOthersModelsGroup::* OtherModelGroupMethod)(IGuiData*);

class ToolBarOthersModelsGroup : public QWidget, public IPanel
{
	Q_OBJECT

public:
	explicit ToolBarOthersModelsGroup(IDataDispatcher& dataDispatcher, QWidget* parent, float scale);

	void informData(IGuiData* data);

public slots:
	void onSphere();
	void onPoint();
	void onBeam();

private:
	~ToolBarOthersModelsGroup();

	void onProjectLoad(IGuiData* data);
	void onActivateFunction(IGuiData* data);


private:
	std::unordered_map<guiDType, OtherModelGroupMethod> m_methods;
	Ui::toolbar_othersmodelsgroup m_ui;
	IDataDispatcher& m_dataDispatcher;
};

#endif // TOOLBAR_OTHER_MODEL_H

