#include "gui/toolBars/ToolBarColorimetricFilter.h"

#include "controller/controls/ControlPicking.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "gui/widgets/CustomWidgets/ColorimetricFilterPreview.h"
#include "models/graph/CameraNode.h"
#include "utils/ColorimetricFilterUtils.h"

#include <QRegularExpression>
#include <QIcon>
#include <algorithm>

namespace
{
    const QRegularExpression kNumberRegex(QStringLiteral("(\\d{1,3})"));

    bool parseRgb(const QString& text, int& r, int& g, int& b)
    {
        QRegularExpressionMatchIterator it = kNumberRegex.globalMatch(text);
        if (!it.hasNext())
            return false;

        QList<int> values;
        while (it.hasNext())
        {
            QRegularExpressionMatch match = it.next();
            values.append(match.captured(1).toInt());
            if (values.size() >= 3)
                break;
        }

        if (values.size() < 3)
            return false;

        r = std::clamp(values[0], 0, 255);
        g = std::clamp(values[1], 0, 255);
        b = std::clamp(values[2], 0, 255);
        return true;
    }

    bool parseIntensity(const QString& text, int& intensity)
    {
        QRegularExpressionMatch match = kNumberRegex.match(text);
        if (!match.hasMatch())
            return false;
        intensity = std::clamp(match.captured(1).toInt(), 0, 255);
        return true;
    }
}

ToolBarColorimetricFilter::ToolBarColorimetricFilter(IDataDispatcher& dataDispatcher, QWidget* parent, float /*guiScale*/)
    : QWidget(parent)
    , m_dataDispatcher(dataDispatcher)
{
    m_ui.setupUi(this);
    setEnabled(false);

    QIcon pickIcon(QStringLiteral(":/icons/100x100/icons8-color-wheel-30.png"));
    m_ui.toolButtonPick1->setIcon(pickIcon);
    m_ui.toolButtonPick2->setIcon(pickIcon);
    m_ui.toolButtonPick3->setIcon(pickIcon);
    m_ui.toolButtonPick4->setIcon(pickIcon);

    connect(m_ui.pushButtonApply, &QPushButton::clicked, [this]() { applySettings(true); });
    connect(m_ui.pushButtonDeactivate, &QPushButton::clicked, [this]() { applySettings(false); });
    connect(m_ui.pushButtonReset, &QPushButton::clicked, [this]() { resetSettings(); });

    connect(m_ui.doubleSpinBoxTolerance, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            [this](double) { if (m_settings.enabled) applySettings(true); });
    connect(m_ui.radioShowColors, &QRadioButton::toggled, [this](bool) { if (m_settings.enabled) applySettings(true); });
    connect(m_ui.radioHideColors, &QRadioButton::toggled, [this](bool) { if (m_settings.enabled) applySettings(true); });

    auto connectColorEdit = [this](QLineEdit* edit)
    {
        connect(edit, &QLineEdit::editingFinished, [this]() { if (m_settings.enabled) applySettings(true); updatePreview(); });
    };

    connectColorEdit(m_ui.lineEditColor1);
    connectColorEdit(m_ui.lineEditColor2);
    connectColorEdit(m_ui.lineEditColor3);
    connectColorEdit(m_ui.lineEditColor4);

    auto connectPick = [this](QToolButton* button, int index)
    {
        connect(button, &QToolButton::clicked, [this, index]() { startPick(index); });
    };

    connectPick(m_ui.toolButtonPick1, 0);
    connectPick(m_ui.toolButtonPick2, 1);
    connectPick(m_ui.toolButtonPick3, 2);
    connectPick(m_ui.toolButtonPick4, 3);

    registerGuiDataFunction(guiDType::projectLoaded, &ToolBarColorimetricFilter::onProjectLoad);
    registerGuiDataFunction(guiDType::renderActiveCamera, &ToolBarColorimetricFilter::onActiveCamera);
    registerGuiDataFunction(guiDType::focusViewport, &ToolBarColorimetricFilter::onFocusViewport);
    registerGuiDataFunction(guiDType::renderColorMode, &ToolBarColorimetricFilter::onRenderMode);
    registerGuiDataFunction(guiDType::renderColorimetricFilter, &ToolBarColorimetricFilter::onRenderColorimetricFilter);
    registerGuiDataFunction(guiDType::colorimetricFilterPickValue, &ToolBarColorimetricFilter::onColorPickValue);
}

void ToolBarColorimetricFilter::informData(IGuiData* data)
{
    if (m_methods.find(data->getType()) != m_methods.end())
    {
        GuiDataFunction method = m_methods.at(data->getType());
        (this->*method)(data);
    }
}

void ToolBarColorimetricFilter::onProjectLoad(IGuiData* data)
{
    GuiDataProjectLoaded* projectData = static_cast<GuiDataProjectLoaded*>(data);
    setEnabled(projectData->m_isProjectLoad);
}

void ToolBarColorimetricFilter::onFocusViewport(IGuiData* data)
{
    GuiDataFocusViewport* castData = static_cast<GuiDataFocusViewport*>(data);
    if (castData->m_forceFocus && castData->m_camera)
        m_focusCamera = castData->m_camera;
}

void ToolBarColorimetricFilter::onActiveCamera(IGuiData* data)
{
    auto infos = static_cast<GuiDataCameraInfo*>(data);
    if (infos->m_camera && !(m_focusCamera == infos->m_camera))
        return;

    ReadPtr<CameraNode> rCam = m_focusCamera.cget();
    if (!rCam)
        return;

    m_currentRenderMode = rCam->getDisplayParameters().m_mode;
    updateUiFromSettings(rCam->getDisplayParameters().m_colorimetricFilter);
}

void ToolBarColorimetricFilter::onRenderMode(IGuiData* data)
{
    auto* modeData = static_cast<GuiDataRenderColorMode*>(data);
    m_currentRenderMode = modeData->m_mode;
    updateFieldStates();
    updatePreview();
}

void ToolBarColorimetricFilter::onRenderColorimetricFilter(IGuiData* data)
{
    auto* filterData = static_cast<GuiDataRenderColorimetricFilter*>(data);
    updateUiFromSettings(filterData->m_settings);
}

void ToolBarColorimetricFilter::onColorPickValue(IGuiData* data)
{
    auto* pickData = static_cast<GuiDataColorimetricFilterPickValue*>(data);
    Color32 color = pickData->m_color;

    if (m_currentRenderMode == UiRenderMode::Intensity || m_currentRenderMode == UiRenderMode::Fake_Color)
    {
        Color32 intensityColor(pickData->m_intensity, pickData->m_intensity, pickData->m_intensity);
        updateColorField(0, intensityColor, true);
    }
    else
    {
        ColorimetricFilterSettings settings = readSettingsFromUi(m_settings.enabled);
        int targetIndex = m_pendingPickIndex;
        if (targetIndex < 0 || targetIndex >= 4)
        {
            targetIndex = -1;
            for (int i = 0; i < 4; ++i)
            {
                if (!settings.colorsEnabled[i])
                {
                    targetIndex = i;
                    break;
                }
            }
            if (targetIndex == -1)
                targetIndex = 0;
        }
        updateColorField(targetIndex, color, true);
    }

    m_pendingPickIndex = -1;
    updatePreview();
    applySettings(m_settings.enabled);
}

void ToolBarColorimetricFilter::applySettings(bool enable)
{
    ColorimetricFilterSettings settings = readSettingsFromUi(enable);
    m_settings = settings;
    updatePreview();
    m_dataDispatcher.updateInformation(new GuiDataRenderColorimetricFilter(settings, m_focusCamera), this);
}

void ToolBarColorimetricFilter::resetSettings()
{
    m_settings = ColorimetricFilterSettings{};
    updateUiFromSettings(m_settings);
    m_dataDispatcher.updateInformation(new GuiDataRenderColorimetricFilter(m_settings, m_focusCamera), this);
}

void ToolBarColorimetricFilter::updateUiFromSettings(const ColorimetricFilterSettings& settings)
{
    m_settings = settings;
    m_ui.doubleSpinBoxTolerance->blockSignals(true);
    m_ui.doubleSpinBoxTolerance->setValue(settings.tolerance);
    m_ui.doubleSpinBoxTolerance->blockSignals(false);

    m_ui.radioShowColors->blockSignals(true);
    m_ui.radioHideColors->blockSignals(true);
    m_ui.radioShowColors->setChecked(settings.showColors);
    m_ui.radioHideColors->setChecked(!settings.showColors);
    m_ui.radioShowColors->blockSignals(false);
    m_ui.radioHideColors->blockSignals(false);

    for (int i = 0; i < 4; ++i)
        updateColorField(i, settings.colors[i], settings.colorsEnabled[i]);

    updateFieldStates();
    updatePreview();
}

void ToolBarColorimetricFilter::updatePreview()
{
    ColorimetricFilterSettings settings = readSettingsFromUi(m_settings.enabled);
    auto colors = ColorimetricFilterUtils::getOrderedActiveColors(settings, m_currentRenderMode);
    m_ui.previewWidget->setColors(colors);
}

void ToolBarColorimetricFilter::updateFieldStates()
{
    bool intensityMode = (m_currentRenderMode == UiRenderMode::Intensity || m_currentRenderMode == UiRenderMode::Fake_Color);
    setFieldPlaceholder(intensityMode);

    auto setEnabled = [intensityMode](QWidget* widget, bool enabled)
    {
        widget->setEnabled(enabled && !intensityMode);
    };

    setEnabled(m_ui.lineEditColor2, true);
    setEnabled(m_ui.lineEditColor3, true);
    setEnabled(m_ui.lineEditColor4, true);
    setEnabled(m_ui.toolButtonPick2, true);
    setEnabled(m_ui.toolButtonPick3, true);
    setEnabled(m_ui.toolButtonPick4, true);

    m_ui.labelColor2->setEnabled(!intensityMode);
    m_ui.labelColor3->setEnabled(!intensityMode);
    m_ui.labelColor4->setEnabled(!intensityMode);
}

void ToolBarColorimetricFilter::updateColorField(int index, const Color32& color, bool enabled)
{
    QLineEdit* edit = nullptr;
    switch (index)
    {
    case 0: edit = m_ui.lineEditColor1; break;
    case 1: edit = m_ui.lineEditColor2; break;
    case 2: edit = m_ui.lineEditColor3; break;
    case 3: edit = m_ui.lineEditColor4; break;
    default: return;
    }

    QPalette pal = edit->palette();
    if (enabled)
    {
        QColor qColor(color.Red(), color.Green(), color.Blue());
        pal.setColor(QPalette::Base, qColor);
        if (index == 0 && (m_currentRenderMode == UiRenderMode::Intensity || m_currentRenderMode == UiRenderMode::Fake_Color))
            edit->setText(QString::number(color.Red()));
        else
            edit->setText(QStringLiteral("%1, %2, %3").arg(color.Red()).arg(color.Green()).arg(color.Blue()));
    }
    else
    {
        pal.setColor(QPalette::Base, palette().color(QPalette::Base));
        edit->clear();
    }
    edit->setPalette(pal);
    edit->setAutoFillBackground(true);
}

bool ToolBarColorimetricFilter::parseColorField(int index, Color32& outColor) const
{
    QLineEdit* edit = nullptr;
    switch (index)
    {
    case 0: edit = m_ui.lineEditColor1; break;
    case 1: edit = m_ui.lineEditColor2; break;
    case 2: edit = m_ui.lineEditColor3; break;
    case 3: edit = m_ui.lineEditColor4; break;
    default: return false;
    }

    int r = 0;
    int g = 0;
    int b = 0;
    if (!parseRgb(edit->text(), r, g, b))
        return false;
    outColor = Color32(r, g, b);
    return true;
}

bool ToolBarColorimetricFilter::parseIntensityField(Color32& outColor) const
{
    int intensity = 0;
    if (!parseIntensity(m_ui.lineEditColor1->text(), intensity))
        return false;
    outColor = Color32(intensity, intensity, intensity);
    return true;
}

ColorimetricFilterSettings ToolBarColorimetricFilter::readSettingsFromUi(bool keepEnabled) const
{
    ColorimetricFilterSettings settings = m_settings;
    settings.tolerance = static_cast<float>(m_ui.doubleSpinBoxTolerance->value());
    settings.showColors = m_ui.radioShowColors->isChecked();
    settings.enabled = keepEnabled;

    for (int i = 0; i < 4; ++i)
    {
        settings.colorsEnabled[i] = false;
    }

    if (m_currentRenderMode == UiRenderMode::Intensity || m_currentRenderMode == UiRenderMode::Fake_Color)
    {
        Color32 intensityColor;
        if (parseIntensityField(intensityColor))
        {
            settings.colors[0] = intensityColor;
            settings.colorsEnabled[0] = true;
        }
    }
    else
    {
        for (int i = 0; i < 4; ++i)
        {
            Color32 color;
            if (parseColorField(i, color))
            {
                settings.colors[i] = color;
                settings.colorsEnabled[i] = true;
            }
        }
    }

    return settings;
}

void ToolBarColorimetricFilter::setFieldPlaceholder(bool intensityMode)
{
    if (intensityMode)
        m_ui.lineEditColor1->setPlaceholderText(tr("Intensity (0-255)"));
    else
        m_ui.lineEditColor1->setPlaceholderText(tr("R, G, B"));
}

void ToolBarColorimetricFilter::startPick(int index)
{
    m_pendingPickIndex = index;
    m_dataDispatcher.sendControl(new control::picking::PickColorimetricFromPick());
}
