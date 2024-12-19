#ifndef TOOLBAR_RENDER_NORMALS_H
#define TOOLBAR_RENDER_NORMALS_H

#include "ui_toolbar_rendernormalsgroup.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"
#include "pointCloudEngine/RenderingTypes.h"

#include "utils/safe_ptr.h"

class CameraNode;

class ToolBarRenderNormals : public QWidget, public IPanel
{
	Q_OBJECT

public:
	explicit ToolBarRenderNormals(IDataDispatcher& dataDispatcher, QWidget* parent = 0, float guiScale = 1.f);
	~ToolBarRenderNormals() {};


private:
	void informData(IGuiData* data) override;
	void onProjectLoad(IGuiData* data);
	void onActiveCamera(IGuiData* data);
	void onFocusViewport(IGuiData* data);

	void updateNormals(const PostRenderingNormals& normalsParams);
	void blockAllSignals(bool block);

private slots:
	void slotNormalsChanged();
	void slotSharpnessChanged(double value);

private:
	typedef void (ToolBarRenderNormals::*GuiDataFunction)(IGuiData*);
	std::unordered_map<guiDType, GuiDataFunction> m_methods;

	inline void registerGuiDataFunction(guiDType type, GuiDataFunction fct)
	{
		m_dataDispatcher.registerObserverOnKey(this, type);
		m_methods.insert({ type, fct });
	};

	Ui::toolbar_rendernormalsgroup m_ui;
	IDataDispatcher& m_dataDispatcher;
	SafePtr<CameraNode>	m_focusCamera;

};

#endif // TOOLBAR_RENDERSETTINGS_H
