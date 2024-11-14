#ifndef TOOLBAR_TAGGROUP_H
#define TOOLBAR_TAGGROUP_H

#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"
#include "gui/dialog/MarkerIconSelectionDialog.h"
#include "models/application/TagTemplate.h"
#include "utils/Color32.hpp"

#include <QtWidgets/qwidget.h>

#include "ui_toolbar_taggroup.h"

class ToolBarTagGroup;

typedef void (ToolBarTagGroup::*TagGroupMethod)(IGuiData*);

class ToolBarTagGroup : public QWidget, public IPanel
{
	Q_OBJECT

public:
	explicit ToolBarTagGroup(IDataDispatcher &dataDispatcher, QWidget *parent, const float& guiScale);

	void informData(IGuiData *data) override;

	void onProjectLoad(IGuiData *data);
	void onActivatedFunctions(IGuiData *data);

private:
    ~ToolBarTagGroup();

public slots:
	void clickCreatetag();
	void clickMoveTag();
	void clickDuplicateTag();

private:
	std::vector<std::pair<sma::tFieldId, std::string>> templateElems;

	std::unordered_map<guiDType, TagGroupMethod> methods;

	Ui::ToolBarTagGroup m_ui;
    IDataDispatcher &m_dataDispatcher;
};

#endif // TOOLBAR_TAGGROUP_H

