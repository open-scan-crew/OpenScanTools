#ifndef TOOLBAR_ATTRIBUTES_GROUP_H
#define TOOLBAR_ATTRIBUTES_GROUP_H

#include <QtWidgets/qwidget.h>
#include "ui_toolbar_attributesgroup.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"
#include "utils/Color32.hpp"
//#include "models/OpenScanToolsModelEssentials.h"
#include "utils/safe_ptr.h"
#include "models/application/List.h"

class Controller;
class ToolBarAttributesGroup;

typedef void (ToolBarAttributesGroup::* AttributesGroupMethod)(IGuiData*);

class ToolBarAttributesGroup : public QWidget, public IPanel
{
	Q_OBJECT

public:
	explicit ToolBarAttributesGroup(Controller& controller, QWidget* parent, const float& scale);

	void informData(IGuiData* data) override;

	void hideColorPicker(const bool& visible = false);

private:
	void onProjectLoad(IGuiData* data);
	void onList(IGuiData* data);
	void onDisciplineSelected(IGuiData* data);
	void onPhaseSelected(IGuiData* data);
	void onNameSelected(IGuiData* data);
	void onIdentifierSelected(IGuiData* data);

public slots:
	void onNameChange();
	void onIdentifierChange();
	void onPhaseChange(const int& index);
	void onDisciplineChange(const int& index);
	void onColorChange(const Color32& color);

private:
	~ToolBarAttributesGroup();
	void updateLists();

private:
	std::unordered_map<guiDType, AttributesGroupMethod> m_methods;
	QString   m_openPath;
	Ui::toolbar_attributesgroup m_ui;
	IDataDispatcher& m_dataDispatcher;

	SafePtr<UserList> m_disciplineList;
	SafePtr<UserList> m_phaseList;
};

#endif // TOOLBAR_ATTRIBUTES_GROUP_H

