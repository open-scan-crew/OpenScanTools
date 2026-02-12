#include "gui/toolBars/ToolBarSelector.h"

#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "models/graph/CameraNode.h"

ToolBarSelector::ToolBarSelector(IDataDispatcher& dataDispatcher, QWidget* parent, float guiScale)
    : QWidget(parent)
    , m_dataDispatcher(dataDispatcher)
{
    m_ui.setupUi(this);
    setEnabled(false);

    m_ui.toolButton_polygonalSelector->setIconSize(QSize(20, 20) * guiScale);

    connect(m_ui.toolButton_polygonalSelector, &QToolButton::clicked, [this]()
    {
        applySettings(m_ui.toolButton_polygonalSelector->isChecked());
    });

    connect(m_ui.radioButton_showInside, &QRadioButton::toggled, [this](bool)
    {
        if (m_settings.enabled)
            applySettings(true);
    });

    connect(m_ui.radioButton_showOutside, &QRadioButton::toggled, [this](bool)
    {
        if (m_settings.enabled)
            applySettings(true);
    });

    connect(m_ui.pushButton_apply, &QPushButton::clicked, [this]() { applySettings(true); });
    connect(m_ui.pushButton_deactivate, &QPushButton::clicked, [this]() { applySettings(false); });
    connect(m_ui.pushButton_reset, &QPushButton::clicked, [this]() { resetSettings(); });

    registerGuiDataFunction(guiDType::projectLoaded, &ToolBarSelector::onProjectLoad);
    registerGuiDataFunction(guiDType::focusViewport, &ToolBarSelector::onFocusViewport);
    registerGuiDataFunction(guiDType::renderActiveCamera, &ToolBarSelector::onActiveCamera);
    registerGuiDataFunction(guiDType::renderPolygonalSelector, &ToolBarSelector::onRenderSelector);

    refreshUiFromSettings();
}

ToolBarSelector::~ToolBarSelector()
{
    m_dataDispatcher.unregisterObserver(this);
}

void ToolBarSelector::informData(IGuiData* data)
{
    if (m_methods.find(data->getType()) != m_methods.end())
    {
        GuiDataFunction method = m_methods.at(data->getType());
        (this->*method)(data);
    }
}

void ToolBarSelector::onProjectLoad(IGuiData* data)
{
    GuiDataProjectLoaded* projectData = static_cast<GuiDataProjectLoaded*>(data);
    setEnabled(projectData->m_isProjectLoad);
}

void ToolBarSelector::onFocusViewport(IGuiData* data)
{
    GuiDataFocusViewport* castData = static_cast<GuiDataFocusViewport*>(data);
    if (castData->m_forceFocus && castData->m_camera)
        m_focusCamera = castData->m_camera;
}

void ToolBarSelector::onActiveCamera(IGuiData* data)
{
    auto infos = static_cast<GuiDataCameraInfo*>(data);
    if (infos->m_camera && !(m_focusCamera == infos->m_camera))
        return;

    ReadPtr<CameraNode> rCam = m_focusCamera.cget();
    if (!rCam)
        return;

    m_settings = rCam->getDisplayParameters().m_polygonalSelector;
    refreshUiFromSettings();
}

void ToolBarSelector::onRenderSelector(IGuiData* data)
{
    auto* selectorData = static_cast<GuiDataRenderPolygonalSelector*>(data);
    m_settings = selectorData->m_settings;
    refreshUiFromSettings();
}

void ToolBarSelector::applySettings(bool enabled)
{
    m_settings.enabled = enabled;
    m_settings.showInside = m_ui.radioButton_showInside->isChecked();
    refreshUiFromSettings();
    m_dataDispatcher.updateInformation(new GuiDataRenderPolygonalSelector(m_settings, m_focusCamera), this);
}

void ToolBarSelector::resetSettings()
{
    m_settings = PolygonalSelectorSettings{};
    refreshUiFromSettings();
    m_dataDispatcher.updateInformation(new GuiDataRenderPolygonalSelector(m_settings, m_focusCamera), this);
}

void ToolBarSelector::refreshUiFromSettings()
{
    m_ui.toolButton_polygonalSelector->blockSignals(true);
    m_ui.radioButton_showInside->blockSignals(true);
    m_ui.radioButton_showOutside->blockSignals(true);

    m_ui.toolButton_polygonalSelector->setChecked(m_settings.enabled);
    m_ui.radioButton_showInside->setChecked(m_settings.showInside);
    m_ui.radioButton_showOutside->setChecked(!m_settings.showInside);
    m_ui.label_polygonCount->setText(QString::number(m_settings.polygons.size()));

    m_ui.toolButton_polygonalSelector->blockSignals(false);
    m_ui.radioButton_showInside->blockSignals(false);
    m_ui.radioButton_showOutside->blockSignals(false);
}
