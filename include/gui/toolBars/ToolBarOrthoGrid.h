#ifndef TOOLBAR_ORTHOGRID_H
#define TOOLBAR_ORTHOGRID_H

#include <QtWidgets/qwidget.h>
#include "ui_toolbar_orthoGrid.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"
#include "utils/safe_ptr.h"

class CameraNode;

class ToolBarOrthoGrid : public QWidget, public IPanel
{
	Q_OBJECT

public:
	explicit ToolBarOrthoGrid(IDataDispatcher &dataDispatcher, QWidget *parent, const float& guiScale);

	void informData(IGuiData *data) override;

private:
    ~ToolBarOrthoGrid();
	void onProjectLoad(IGuiData* data);
	void blockAllSignals(bool block);
	void onActiveCamera(IGuiData* data);
	void onFocusViewport(IGuiData* data);

	typedef void (ToolBarOrthoGrid::* GuiDataFunction)(IGuiData*);
	inline void registerGuiDataFunction(guiDType type, GuiDataFunction fct)
	{
		m_dataDispatcher.registerObserverOnKey(this, type);
		m_methods.insert({ type, fct });
	};

	void updateGrid();

private slots:
	void slotColorPicking();

private:
	std::unordered_map<guiDType, GuiDataFunction> m_methods;
	Ui::ToolBarOrthoGrid m_ui;
    IDataDispatcher &m_dataDispatcher;
	QString m_openPath;

	QColor m_selectedColor;
	SafePtr<CameraNode>	m_focusCamera;
};

#endif // TOOLBAR_IMPORTOBJECTS_H

