#ifndef TOOLBAR_TEXTPROPERTIES_H
#define TOOLBAR_TEXTPROPERTIES_H

#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"
#include "pointCloudEngine/ShowTypes.h"
#include "utils/safe_ptr.h"

#include <QtWidgets/qwidget.h>

#include "ui_toolbar_textdisplay.h"

class ToolBarTextDisplay;

typedef void (ToolBarTextDisplay::*TextDisplayMethod)(IGuiData*);
class CameraNode;

class ToolBarTextDisplay : public QWidget, public IPanel
{
	Q_OBJECT

public :
	explicit ToolBarTextDisplay(IDataDispatcher &dataDispatcher, QWidget *parent, const float& guiScale);

	void informData(IGuiData *data) override;

	void onProjectLoad(IGuiData *data);
	void onActiveCamera(IGuiData* data);
	void onFocusViewport(IGuiData* data);

protected :


private:
	~ToolBarTextDisplay();

	TextFilter m_textFilter;
	inline void addRemoveFilter(TextFilter& textFilter, bool checked, int filter);
	void updateUiFromCamera();
	void blockAllSignals(bool block);

	void toggleRenderParameter(bool checked, int parameter);
	void changeTheme(int theme);
	void changeTextFontSize(double font);

	std::unordered_map<guiDType, TextDisplayMethod> m_methods;

	Ui::ToolBarTextDisplay m_ui;
	IDataDispatcher &m_dataDispatcher;
	SafePtr<CameraNode> m_focusCamera;
};

#endif // TOOLBAR_TAGGROUP_H
