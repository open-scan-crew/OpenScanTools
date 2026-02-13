#include "gui/toolBars/ToolBarPolygonalSelector.h"

#include "controller/controls/ControlFunction.h"
#include "controller/controls/ControlFunctionClipping.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataRendering.h"

ToolBarPolygonalSelector::ToolBarPolygonalSelector(IDataDispatcher& dataDispatcher, QWidget* parent, float guiScale)
    : QWidget(parent)
    , m_dataDispatcher(dataDispatcher)
{
    m_ui.setupUi(this);
    setEnabled(false);
    m_ui.toolButton_polygonalSelector->setIconSize(QSize(20, 20) * guiScale);

    connect(m_ui.toolButton_polygonalSelector, &QToolButton::clicked, this, &ToolBarPolygonalSelector::activateSelector);
    connect(m_ui.pushButtonApply, &QPushButton::clicked, [this]() { applySettings(true); });
    connect(m_ui.pushButtonDeactivate, &QPushButton::clicked, [this]() { applySettings(false); });
    connect(m_ui.pushButtonReset, &QPushButton::clicked, [this]() { resetSettings(); });

    connect(m_ui.radioButtonShowSelected, &QRadioButton::toggled, [this](bool) {
        m_settings.showSelected = m_ui.radioButtonShowSelected->isChecked();
        m_dataDispatcher.updateInformation(new GuiDataRenderPolygonalSelector(m_settings, SafePtr<CameraNode>()), this);
    });

    m_dataDispatcher.registerObserverOnKey(this, guiDType::projectLoaded);
    m_dataDispatcher.registerObserverOnKey(this, guiDType::activatedFunctions);
    m_dataDispatcher.registerObserverOnKey(this, guiDType::renderPolygonalSelector);

    m_methods.insert({ guiDType::projectLoaded, &ToolBarPolygonalSelector::onProjectLoad });
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
    setEnabled(static_cast<GuiDataProjectLoaded*>(data)->m_isProjectLoad);
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
    m_settings = castData->m_settings;

    m_ui.radioButtonShowSelected->blockSignals(true);
    m_ui.radioButtonHideSelected->blockSignals(true);
    m_ui.radioButtonShowSelected->setChecked(m_settings.showSelected);
    m_ui.radioButtonHideSelected->setChecked(!m_settings.showSelected);
    m_ui.radioButtonShowSelected->blockSignals(false);
    m_ui.radioButtonHideSelected->blockSignals(false);
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

    m_dataDispatcher.updateInformation(new GuiDataRenderPolygonalSelector(m_settings, SafePtr<CameraNode>()), this);
}

void ToolBarPolygonalSelector::resetSettings()
{
    m_settings = PolygonalSelectorSettings{};
    m_settings.appliedPolygonCount = 0;
    m_settings.pendingApply = false;
    m_ui.radioButtonShowSelected->setChecked(true);
    m_dataDispatcher.updateInformation(new GuiDataRenderPolygonalSelector(m_settings, SafePtr<CameraNode>()), this);
}
