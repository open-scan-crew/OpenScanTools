#ifndef TOOLBAR_TEXTPROPERTIES_H
#define TOOLBAR_TEXTPROPERTIES_H

#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"
#include "pointCloudEngine/ShowTypes.h"

#include <QtWidgets/qwidget.h>

#include "ui_toolbar_textdisplay.h"

class ToolBarTextDisplay;

typedef void (ToolBarTextDisplay::*TextDisplayMethod)(IGuiData*);

class ToolBarTextDisplay : public QWidget, public IPanel
{
	Q_OBJECT

public :
	explicit ToolBarTextDisplay(IDataDispatcher &dataDispatcher, QWidget *parent, const float& guiScale);

	void informData(IGuiData *data) override;

	void onProjectLoad(IGuiData *data);

protected :


private:
	~ToolBarTextDisplay();

	TextFilter m_textFilter;
	inline void addRemoveFilter(TextFilter& textFilter, bool checked, int filter);

	void toggleRenderParameter(bool checked, int parameter);
	void changeTheme(int theme);
	void changeTextFontSize(double font);

	std::unordered_map<guiDType, TextDisplayMethod> m_methods;

	Ui::ToolBarTextDisplay m_ui;
	IDataDispatcher &m_dataDispatcher;
};

#endif // TOOLBAR_TAGGROUP_H

