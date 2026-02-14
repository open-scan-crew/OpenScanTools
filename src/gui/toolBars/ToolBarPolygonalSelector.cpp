#include "gui/toolBars/ToolBarPolygonalSelector.h"

#include "controller/controls/ControlFunction.h"
#include "controller/controls/ControlFunctionClipping.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "models/graph/CameraNode.h"

#include <algorithm>
#include <string>

namespace
{
uint32_t getPolygonSuffix(const std::string& name)
{
    if (name.rfind("polygon_", 0) != 0)
        return 0;

    bool ok = false;
    int suffix = QString::fromStdString(name.substr(8)).toInt(&ok);
    return (ok && suffix > 0) ? static_cast<uint32_t>(suffix) : 0;
}

uint32_t computeNextPolygonId(const PolygonalSelectorSettings& settings)
{
    uint32_t maxSuffix = 0;
    for (const PolygonalSelectorPolygon& polygon : settings.polygons)
        maxSuffix = std::max<uint32_t>(maxSuffix, getPolygonSuffix(polygon.name));

    return std::max<uint32_t>(std::max<uint32_t>(settings.nextPolygonId, maxSuffix + 1), 1u);
}
}

ToolBarPolygonalSelector::ToolBarPolygonalSelector(IDataDispatcher& dataDispatcher, QWidget* parent, float guiScale)
    : QWidget(parent)
    , m_dataDispatcher(dataDispatcher)
{
    m_ui.setupUi(this);
    setEnabled(false);
    m_ui.toolButton_polygonalSelector->setIconSize(QSize(20, 20) * guiScale);
    m_ui.comboBox_polygonList->setEnabled(false);
    m_ui.deletePolygonButton->setEnabled(false);

    connect(m_ui.toolButton_polygonalSelector, &QToolButton::clicked, this, &ToolBarPolygonalSelector::activateSelector);
    connect(m_ui.pushButtonApply, &QPushButton::clicked, [this]() { applySettings(true); });
    connect(m_ui.pushButtonDeactivate, &QPushButton::clicked, [this]() { applySettings(false); });
    connect(m_ui.pushButtonReset, &QPushButton::clicked, [this]() { resetSettings(); });
    connect(m_ui.checkBox_managePolygon, &QCheckBox::toggled, this, &ToolBarPolygonalSelector::setManageMode);
    connect(m_ui.comboBox_polygonList, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ToolBarPolygonalSelector::onPolygonSelectionChanged);
    connect(m_ui.deletePolygonButton, &QPushButton::clicked, this, &ToolBarPolygonalSelector::deleteSelectedPolygon);

    connect(m_ui.radioButtonShowSelected, &QRadioButton::toggled, [this](bool) {
        m_settings.showSelected = m_ui.radioButtonShowSelected->isChecked();
        sendSettingsUpdate();
    });

    m_dataDispatcher.registerObserverOnKey(this, guiDType::projectLoaded);
    m_dataDispatcher.registerObserverOnKey(this, guiDType::renderActiveCamera);
    m_dataDispatcher.registerObserverOnKey(this, guiDType::focusViewport);
    m_dataDispatcher.registerObserverOnKey(this, guiDType::activatedFunctions);
    m_dataDispatcher.registerObserverOnKey(this, guiDType::renderPolygonalSelector);

    m_methods.insert({ guiDType::projectLoaded, &ToolBarPolygonalSelector::onProjectLoad });
    m_methods.insert({ guiDType::renderActiveCamera, &ToolBarPolygonalSelector::onActiveCamera });
    m_methods.insert({ guiDType::focusViewport, &ToolBarPolygonalSelector::onFocusViewport });
    m_methods.insert({ guiDType::activatedFunctions, &ToolBarPolygonalSelector::onActivatedFunctions });
    m_methods.insert({ guiDType::renderPolygonalSelector, &ToolBarPolygonalSelector::onRenderSettings });
}

ToolBarPolygonalSelector::~ToolBarPolygonalSelector()
{
    m_dataDispatcher.unregisterObserver(this);
}

void ToolBarPolygonalSelector::informData(IGuiData* data)
{
    auto it = m_methods.find(data->getType());
    if (it != m_methods.end())
        (this->*(it->second))(data);
}

void ToolBarPolygonalSelector::onProjectLoad(IGuiData* data)
{
    m_isProjectLoaded = static_cast<GuiDataProjectLoaded*>(data)->m_isProjectLoad;
    setEnabled(m_isProjectLoaded);

    if (!m_isProjectLoaded)
    {
        m_focusCamera = SafePtr<CameraNode>();
        m_settings = PolygonalSelectorSettings{};
        m_ui.radioButtonShowSelected->setChecked(true);
        m_ui.checkBox_managePolygon->blockSignals(true);
        m_ui.checkBox_managePolygon->setChecked(false);
        m_ui.checkBox_managePolygon->blockSignals(false);
        resetManageSelection();
        refreshPolygonList();
        return;
    }

    m_settings = PolygonalSelectorSettings{};
    m_ui.radioButtonShowSelected->setChecked(true);
    m_ui.checkBox_managePolygon->blockSignals(true);
    m_ui.checkBox_managePolygon->setChecked(false);
    m_ui.checkBox_managePolygon->blockSignals(false);
    resetManageSelection();
    refreshPolygonList();

    syncFromFocusedCamera();
}

void ToolBarPolygonalSelector::onActiveCamera(IGuiData* data)
{
    auto infos = static_cast<GuiDataCameraInfo*>(data);
    if (infos->m_camera && m_focusCamera && m_focusCamera != infos->m_camera)
        return;

    if (infos->m_camera)
        m_focusCamera = infos->m_camera;

    if (m_isProjectLoaded)
        syncFromFocusedCamera();
}

void ToolBarPolygonalSelector::onFocusViewport(IGuiData* data)
{
    auto* castData = static_cast<GuiDataFocusViewport*>(data);
    if (castData->m_forceFocus && castData->m_camera)
        m_focusCamera = castData->m_camera;

    if (m_isProjectLoaded)
        syncFromFocusedCamera();
}

void ToolBarPolygonalSelector::onActivatedFunctions(IGuiData* data)
{
    auto* functionData = static_cast<GuiDataActivatedFunctions*>(data);

    m_ui.toolButton_polygonalSelector->blockSignals(true);
    m_ui.toolButton_polygonalSelector->setChecked(functionData->type == ContextType::polygonalSelector);
    m_ui.toolButton_polygonalSelector->blockSignals(false);
}

void ToolBarPolygonalSelector::onRenderSettings(IGuiData* data)
{
    auto* castData = static_cast<GuiDataRenderPolygonalSelector*>(data);
    if (castData->m_camera)
        m_focusCamera = castData->m_camera;

    m_settings = castData->m_settings;
    for (size_t i = 0; i < m_settings.polygons.size(); ++i)
    {
        if (m_settings.polygons[i].name.empty())
            m_settings.polygons[i].name = std::string("polygon_") + std::to_string(i + 1);
    }
    m_settings.nextPolygonId = computeNextPolygonId(m_settings);

    if (m_settings.polygons.empty())
    {
        m_settings.manageMode = false;
        m_settings.highlightedPolygonIndex = -1;
    }

    m_ui.radioButtonShowSelected->blockSignals(true);
    m_ui.radioButtonHideSelected->blockSignals(true);
    m_ui.checkBox_managePolygon->blockSignals(true);

    m_ui.radioButtonShowSelected->setChecked(m_settings.showSelected);
    m_ui.radioButtonHideSelected->setChecked(!m_settings.showSelected);
    m_ui.checkBox_managePolygon->setChecked(m_settings.manageMode);

    m_ui.radioButtonShowSelected->blockSignals(false);
    m_ui.radioButtonHideSelected->blockSignals(false);
    m_ui.checkBox_managePolygon->blockSignals(false);

    refreshPolygonList();
}

void ToolBarPolygonalSelector::activateSelector()
{
    if (m_ui.toolButton_polygonalSelector->isChecked())
        m_dataDispatcher.sendControl(new control::function::clipping::ActivatePolygonalSelector());
    else
        m_dataDispatcher.sendControl(new control::function::Abort());
}

void ToolBarPolygonalSelector::applySettings(bool enabled)
{
    if (!syncFromFocusedCamera())
        return;

    m_settings.showSelected = m_ui.radioButtonShowSelected->isChecked();

    if (enabled)
    {
        m_settings.enabled = true;
        m_settings.active = true;
        m_settings.pendingApply = false;
        m_settings.appliedPolygonCount = static_cast<uint32_t>(m_settings.polygons.size());
    }
    else
    {
        m_settings.enabled = false;
        m_settings.active = false;
    }

    sendSettingsUpdate();
}

void ToolBarPolygonalSelector::resetSettings()
{
    if (!syncFromFocusedCamera())
        return;

    const uint32_t nextPolygonId = m_settings.nextPolygonId;
    m_settings = PolygonalSelectorSettings{};
    m_settings.nextPolygonId = std::max<uint32_t>(nextPolygonId, 1u);
    m_settings.appliedPolygonCount = 0;
    m_settings.pendingApply = false;
    m_ui.radioButtonShowSelected->setChecked(true);
    m_ui.checkBox_managePolygon->setChecked(false);
    sendSettingsUpdate();
}

void ToolBarPolygonalSelector::setManageMode(bool enabled)
{
    if (enabled && !syncFromFocusedCamera())
        return;

    m_settings.manageMode = enabled;

    if (enabled)
    {
        m_ui.toolButton_polygonalSelector->setChecked(false);
        m_dataDispatcher.sendControl(new control::function::Abort());
    }

    m_ui.toolButton_polygonalSelector->setEnabled(!enabled);
    m_ui.comboBox_polygonList->setEnabled(enabled && m_ui.comboBox_polygonList->count() > 0);

    if (!enabled)
        resetManageSelection();
    else
        onPolygonSelectionChanged(m_ui.comboBox_polygonList->currentIndex());

    refreshPolygonList();
    sendSettingsUpdate();
}

void ToolBarPolygonalSelector::onPolygonSelectionChanged(int index)
{
    if (!m_settings.manageMode)
    {
        resetManageSelection();
        return;
    }

    if (index >= 0 && index < static_cast<int>(m_settings.polygons.size()))
        m_settings.highlightedPolygonIndex = index;
    else
        m_settings.highlightedPolygonIndex = -1;

    m_ui.deletePolygonButton->setEnabled(m_settings.highlightedPolygonIndex >= 0);
    sendSettingsUpdate();
}

void ToolBarPolygonalSelector::deleteSelectedPolygon()
{
    if (!syncFromFocusedCamera())
        return;

    if (!m_settings.manageMode)
        return;

    const int selectedIndex = m_ui.comboBox_polygonList->currentIndex();
    if (selectedIndex < 0 || selectedIndex >= static_cast<int>(m_settings.polygons.size()))
        return;

    m_settings.polygons.erase(m_settings.polygons.begin() + selectedIndex);

    const uint32_t polygonCount = static_cast<uint32_t>(m_settings.polygons.size());
    m_settings.appliedPolygonCount = std::min<uint32_t>(m_settings.appliedPolygonCount, polygonCount);
    m_settings.pendingApply = (m_settings.appliedPolygonCount < polygonCount);
    if (polygonCount == 0)
    {
        m_settings.enabled = false;
        m_settings.active = false;
    }

    resetManageSelection();
    refreshPolygonList();

    if (m_settings.manageMode && m_ui.comboBox_polygonList->count() > 0)
    {
        m_ui.comboBox_polygonList->setCurrentIndex(std::min(selectedIndex, m_ui.comboBox_polygonList->count() - 1));
        m_settings.highlightedPolygonIndex = m_ui.comboBox_polygonList->currentIndex();
    }

    m_ui.deletePolygonButton->setEnabled(m_settings.highlightedPolygonIndex >= 0);
    sendSettingsUpdate();
}

void ToolBarPolygonalSelector::refreshPolygonList()
{
    m_ui.comboBox_polygonList->blockSignals(true);
    m_ui.comboBox_polygonList->clear();
    for (const PolygonalSelectorPolygon& polygon : m_settings.polygons)
        m_ui.comboBox_polygonList->addItem(QString::fromStdString(polygon.name));

    if (m_settings.highlightedPolygonIndex >= 0 && m_settings.highlightedPolygonIndex < m_ui.comboBox_polygonList->count())
        m_ui.comboBox_polygonList->setCurrentIndex(m_settings.highlightedPolygonIndex);

    m_ui.comboBox_polygonList->blockSignals(false);

    m_ui.comboBox_polygonList->setEnabled(m_settings.manageMode && m_ui.comboBox_polygonList->count() > 0);
    m_ui.deletePolygonButton->setEnabled(m_settings.manageMode && m_settings.highlightedPolygonIndex >= 0);
    m_ui.toolButton_polygonalSelector->setEnabled(!m_settings.manageMode);
}

void ToolBarPolygonalSelector::sendSettingsUpdate()
{
    m_dataDispatcher.updateInformation(new GuiDataRenderPolygonalSelector(m_settings, m_focusCamera), this);
}

void ToolBarPolygonalSelector::resetManageSelection()
{
    m_settings.highlightedPolygonIndex = -1;
    m_ui.deletePolygonButton->setEnabled(false);
}

bool ToolBarPolygonalSelector::syncFromFocusedCamera()
{
    ReadPtr<CameraNode> rCam = m_focusCamera.cget();
    if (!rCam)
        return false;

    m_settings = rCam->getDisplayParameters().m_polygonalSelector;
    for (size_t i = 0; i < m_settings.polygons.size(); ++i)
    {
        if (m_settings.polygons[i].name.empty())
            m_settings.polygons[i].name = std::string("polygon_") + std::to_string(i + 1);
    }
    m_settings.nextPolygonId = computeNextPolygonId(m_settings);

    if (m_settings.polygons.empty())
    {
        m_settings.manageMode = false;
        m_settings.highlightedPolygonIndex = -1;
    }

    m_ui.radioButtonShowSelected->blockSignals(true);
    m_ui.radioButtonHideSelected->blockSignals(true);
    m_ui.checkBox_managePolygon->blockSignals(true);

    m_ui.radioButtonShowSelected->setChecked(m_settings.showSelected);
    m_ui.radioButtonHideSelected->setChecked(!m_settings.showSelected);
    m_ui.checkBox_managePolygon->setChecked(m_settings.manageMode);

    m_ui.radioButtonShowSelected->blockSignals(false);
    m_ui.radioButtonHideSelected->blockSignals(false);
    m_ui.checkBox_managePolygon->blockSignals(false);

    refreshPolygonList();
    return true;
}
