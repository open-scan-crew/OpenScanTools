#ifndef TOOLBAR_TEMPLATEGROUP_H
#define TOOLBAR_TEMPLATEGROUP_H

#include "models/application/TagTemplate.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"
#include "gui/dialog/MarkerIconSelectionDialog.h"
#include "utils/Color32.hpp"

#include <QtWidgets/qwidget.h>

#include "ui_toolbar_templategroup.h"

class ToolBarTemplateGroup;

typedef void (ToolBarTemplateGroup::*TemplateGroupMethod)(IGuiData*);

class ToolBarTemplateGroup : public QWidget, public IPanel
{
	Q_OBJECT

public:
	explicit ToolBarTemplateGroup(IDataDispatcher &dataDispatcher, QWidget *parent, const float& guiScale);

	void informData(IGuiData *data) override;

	void onProjectLoad(IGuiData *data);
	void onTemplateListReceive(IGuiData *data);
	void onTemplateSelected(IGuiData* data);
	void onIconTag(IGuiData *data);

private:
    ~ToolBarTemplateGroup();

public slots:
	void changeTemplateCombo(int);
    void changeTagColor(Color32 color);
    void changeMarkerIcon(scs::MarkerIcon icon);
    void clickIconTag();

private:
	std::vector<SafePtr<sma::TagTemplate>> templateElems;

	std::unordered_map<guiDType, TemplateGroupMethod> methods;

	Ui::ToolBarTemplateGroup m_ui;
    IDataDispatcher &m_dataDispatcher;
    MarkerIconSelectionDialog *m_iconSelectionDialog;
};

#endif // TOOLBAR_TEMPLATEGROUP_H

