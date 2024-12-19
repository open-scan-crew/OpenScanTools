#ifndef TOOLBAR_IMAGEGROUP_H
#define TOOLBAR_IMAGEGROUP_H

#include <QtWidgets/qwidget.h>
#include "ui_toolbar_imagegroup.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"
#include "pointCloudEngine/RenderingTypes.h"

#include "utils/safe_ptr.h"

#include <filesystem>

class CameraNode;

class ToolBarImageGroup;

typedef void (ToolBarImageGroup::* ImageGroupMethod)(IGuiData*);

class ToolBarImageGroup : public QWidget, public IPanel
{
	Q_OBJECT

public:
	explicit ToolBarImageGroup(IDataDispatcher &dataDispatcher, QWidget *parent, const float& guiScale);

	void informData(IGuiData *keyValue) override;

	void quickScreenshot(std::filesystem::path filepath);
	void imageFormat();

private:
	~ToolBarImageGroup();

	typedef void (ToolBarImageGroup::* GuiDataFunction)(IGuiData*);
	inline void registerGuiDataFunction(guiDType type, GuiDataFunction fct)
	{
		m_dataDispatcher.registerObserverOnKey(this, type);
		m_guiDFunctions.insert({ type, fct });
	};

	void onProjectLoad(IGuiData* data);
	void onFocusViewport(IGuiData* data);
	void onActiveCamera(IGuiData* data);
	void onCallHD(IGuiData* data);

	void refreshShowUI();
	void refreshImageSize();
	double getRatioWH();

	void setAreaWidthHeight(uint32_t area_w, uint32_t area_h);
	void setSilentWidthHeight(uint32_t w, uint32_t h);
	uint32_t resetWidth();
	uint32_t resetHeight();

private slots:
	void slotCreateImage(std::filesystem::path filepath, bool showProgressBar);
	void slotShowFrame();
	void slotRatio();
	void slotRatioChanged(int);
	void slotScaleChanged(int);
	void slotDPIChanged(int);
	void slotPortrait(bool checked);
	//void slotWidthChanged();
	void slotHeightChanged();

private:
	IDataDispatcher &m_dataDispatcher;
	ProjectionMode m_projection;
	double m_scale;
	double m_dpmm;
	glm::dvec2 m_cameraOrthoSize;
	glm::ivec2 m_viewportSize;
	glm::ivec2 m_storePerspImageSize;
	SafePtr<CameraNode> m_focusCamera;
	std::unordered_map<guiDType, ImageGroupMethod> m_guiDFunctions;

	Ui::ToolBarImageGroup m_ui;
};

#endif // TOOLBAR_IMAGEGROUP_H

