#include "gui/toolBars/ToolBarColorimetricFilter.h"

#include "controller/controls/ControlPicking.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/widgets/ColorimetricFilterPreview.h"
#include "models/graph/CameraNode.h"
#include "utils/ColorimetricFilterUtils.h"

#include <QApplication>
#include <QPalette>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QIntValidator>
#include <QSignalBlocker>

namespace
{
    constexpr int kColorCount = 4;

    std::array<QLineEdit*, kColorCount> gatherFields(Ui::toolbar_colorimetricFilter& ui)
    {
        return { ui.lineEdit_color1, ui.lineEdit_color2, ui.lineEdit_color3, ui.lineEdit_color4 };
    }

    std::array<QToolButton*, kColorCount> gatherButtons(Ui::toolbar_colorimetricFilter& ui)
    {
        return { ui.toolButton_pickColor1, ui.toolButton_pickColor2, ui.toolButton_pickColor3, ui.toolButton_pickColor4 };
    }
}

ToolBarColorimetricFilter::ToolBarColorimetricFilter(IDataDispatcher& dataDispatcher, QWidget* parent, float guiScale)
    : QWidget(parent)
    , m_dataDispatcher(dataDispatcher)
{
    Q_UNUSED(guiScale);
    m_ui.setupUi(this);
    setEnabled(false);

    m_ui.spinBox_tolerance->setSingleStep(0.1);
    m_ui.spinBox_tolerance->setValue(0.0);

    QRegularExpression rgbRegex(QStringLiteral("^\\s*(?:[0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\\s*,\\s*(?:[0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\\s*,\\s*(?:[0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\\s*$"));
    m_rgbValidator = new QRegularExpressionValidator(rgbRegex, this);
    m_intensityValidator = new QIntValidator(0, 255, this);

    auto fields = gatherFields(m_ui);
    for (int i = 0; i < kColorCount; ++i)
    {
        fields[i]->setValidator(m_rgbValidator);
        connect(fields[i], &QLineEdit::editingFinished, [this, i]() { applyFieldValue(i); });
    }

    auto buttons = gatherButtons(m_ui);
    for (int i = 0; i < kColorCount; ++i)
    {
        connect(buttons[i], &QToolButton::clicked, [this]() { startPicking(); });
    }

    connect(m_ui.pushButton_apply, &QPushButton::clicked, this, &ToolBarColorimetricFilter::applyFilter);
    connect(m_ui.pushButton_deactivate, &QPushButton::clicked, this, &ToolBarColorimetricFilter::deactivateFilter);
    connect(m_ui.pushButton_reset, &QPushButton::clicked, this, &ToolBarColorimetricFilter::resetFilter);

    connect(m_ui.spinBox_tolerance, qOverload<double>(&QDoubleSpinBox::valueChanged), [this](double value)
    {
        if (m_blockUpdates)
            return;
        m_settings.tolerance = static_cast<float>(value);
        updatePreview();
    });

    connect(m_ui.radio_showColors, &QRadioButton::toggled, [this](bool checked)
    {
        if (m_blockUpdates)
            return;
        if (checked)
            m_settings.showColors = true;
    });

    connect(m_ui.radio_hideColors, &QRadioButton::toggled, [this](bool checked)
    {
        if (m_blockUpdates)
            return;
        if (checked)
            m_settings.showColors = false;
    });

    registerGuiDataFunction(guiDType::projectLoaded, &ToolBarColorimetricFilter::onProjectLoad);
    registerGuiDataFunction(guiDType::renderActiveCamera, &ToolBarColorimetricFilter::onActiveCamera);
    registerGuiDataFunction(guiDType::focusViewport, &ToolBarColorimetricFilter::onFocusViewport);
    registerGuiDataFunction(guiDType::colorimetricFilterPickValue, &ToolBarColorimetricFilter::onPickValue);

    updatePreview();
    updateModeState(m_currentMode);
}

ToolBarColorimetricFilter::~ToolBarColorimetricFilter()
{
    stopPicking();
    m_dataDispatcher.unregisterObserver(this);
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
    GuiDataProjectLoaded* plData = static_cast<GuiDataProjectLoaded*>(data);
    setEnabled(plData->m_isProjectLoad);
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
    if (infos->m_camera && m_focusCamera != infos->m_camera)
        return;

    ReadPtr<CameraNode> rCam = m_focusCamera.cget();
    if (!rCam)
        return;

    const DisplayParameters& displayParameters = rCam->getDisplayParameters();
    m_blockUpdates = true;
    m_settings = displayParameters.m_colorimetricFilter;
    m_ui.spinBox_tolerance->setValue(displayParameters.m_colorimetricFilter.tolerance);
    m_ui.radio_showColors->setChecked(displayParameters.m_colorimetricFilter.showColors);
    m_ui.radio_hideColors->setChecked(!displayParameters.m_colorimetricFilter.showColors);
    updateModeState(displayParameters.m_mode);

    auto fields = gatherFields(m_ui);
    for (int i = 0; i < kColorCount; ++i)
    {
        if (m_settings.colorsEnabled[i])
        {
            if (m_intensityMode && i == 0)
                fields[i]->setText(QString::number(m_settings.colors[i].r));
            else if (!m_intensityMode)
                fields[i]->setText(QStringLiteral("%1, %2, %3").arg(m_settings.colors[i].r).arg(m_settings.colors[i].g).arg(m_settings.colors[i].b));
        }
        else
        {
            fields[i]->clear();
        }
        updateFieldState(i);
    }

    updatePreview();
    m_blockUpdates = false;
}

void ToolBarColorimetricFilter::onPickValue(IGuiData* data)
{
    GuiDataColorimetricFilterPickValue* pickData = static_cast<GuiDataColorimetricFilterPickValue*>(data);
    auto fields = gatherFields(m_ui);
    int index = 0;
    if (!m_intensityMode)
    {
        index = findFirstEmptyColor();
        if (index < 0)
            index = 0;
    }

    Color32 pickedColor = pickData->m_color;
    if (m_intensityMode)
    {
        pickedColor = Color32(pickData->m_intensity, pickData->m_intensity, pickData->m_intensity);
    }

    m_settings.colors[index] = pickedColor;
    m_settings.colorsEnabled[index] = true;

    if (m_intensityMode)
    {
        fields[index]->setText(QString::number(pickedColor.r));
    }
    else
    {
        fields[index]->setText(QStringLiteral("%1, %2, %3").arg(pickedColor.r).arg(pickedColor.g).arg(pickedColor.b));
    }

    updateFieldState(index);
    updatePreview();
    stopPicking();
}

void ToolBarColorimetricFilter::applyFilter()
{
    ReadPtr<CameraNode> rCam = m_focusCamera.cget();
    if (!rCam)
        return;

    m_settings.enabled = true;
    m_dataDispatcher.updateInformation(new GuiDataColorimetricFilterSettings(m_settings, m_focusCamera), this);
}

void ToolBarColorimetricFilter::deactivateFilter()
{
    ReadPtr<CameraNode> rCam = m_focusCamera.cget();
    if (!rCam)
        return;

    ColorimetricFilterSettings settings = m_settings;
    settings.enabled = false;
    m_dataDispatcher.updateInformation(new GuiDataColorimetricFilterSettings(settings, m_focusCamera), this);
}

void ToolBarColorimetricFilter::resetFilter()
{
    m_blockUpdates = true;
    m_settings = {};
    m_ui.spinBox_tolerance->setValue(0.0);
    m_ui.radio_showColors->setChecked(true);
    m_ui.radio_hideColors->setChecked(false);

    auto fields = gatherFields(m_ui);
    for (int i = 0; i < kColorCount; ++i)
    {
        if (m_intensityMode && i == 0)
            fields[i]->setText(QStringLiteral("0"));
        else if (!m_intensityMode)
            fields[i]->setText(QStringLiteral("0, 0, 0"));
        else
            fields[i]->clear();
        updateFieldState(i);
    }

    updatePreview();
    m_blockUpdates = false;
    deactivateFilter();
}

void ToolBarColorimetricFilter::updatePreview()
{
    if (m_intensityMode)
    {
        std::array<Color32, 4> colors = { m_settings.colors[0], Color32(0, 0, 0), Color32(0, 0, 0), Color32(0, 0, 0) };
        int count = m_settings.colorsEnabled[0] ? 1 : 0;
        m_ui.previewWidget->setColors(colors, count);
        return;
    }

    ColorimetricFilterUtils::OrderedColorSet ordered = ColorimetricFilterUtils::buildOrderedColorSet(m_settings);
    m_ui.previewWidget->setColors(ordered.colors, ordered.count);
}

void ToolBarColorimetricFilter::updateFieldState(int index)
{
    auto fields = gatherFields(m_ui);
    bool active = m_settings.colorsEnabled[index];
    if (m_intensityMode && index > 0)
        active = false;
    setFieldPalette(fields[index], m_settings.colors[index], active);
}

void ToolBarColorimetricFilter::updateModeState(UiRenderMode mode)
{
    m_currentMode = mode;
    m_intensityMode = (mode == UiRenderMode::Intensity || mode == UiRenderMode::Fake_Color);

    auto fields = gatherFields(m_ui);
    auto buttons = gatherButtons(m_ui);

    for (int i = 0; i < kColorCount; ++i)
    {
        if (m_intensityMode)
        {
            fields[i]->setValidator(m_intensityValidator);
            if (i == 0)
            {
                fields[i]->setEnabled(true);
                fields[i]->setPlaceholderText(tr("0-255"));
                buttons[i]->setEnabled(true);
            }
            else
            {
                fields[i]->setEnabled(false);
                fields[i]->setPlaceholderText(tr("Disabled"));
                buttons[i]->setEnabled(false);
            }
        }
        else
        {
            fields[i]->setValidator(m_rgbValidator);
            fields[i]->setEnabled(true);
            fields[i]->setPlaceholderText(tr("R, G, B"));
            buttons[i]->setEnabled(true);
        }
    }

    updatePreview();
}

void ToolBarColorimetricFilter::applyFieldValue(int index)
{
    if (m_blockUpdates)
        return;

    auto fields = gatherFields(m_ui);
    QString text = fields[index]->text();

    if (text.isEmpty())
    {
        m_settings.colorsEnabled[index] = false;
        updateFieldState(index);
        updatePreview();
        return;
    }

    Color32 color;
    bool ok = m_intensityMode ? parseIntensity(text, color) : parseRgb(text, color);
    if (!ok)
        return;

    m_settings.colors[index] = color;
    m_settings.colorsEnabled[index] = true;
    updateFieldState(index);
    updatePreview();
}

void ToolBarColorimetricFilter::setFieldPalette(QLineEdit* field, const Color32& color, bool active)
{
    QPalette palette = field->palette();
    if (active)
    {
        QColor qcolor(color.r, color.g, color.b);
        palette.setColor(QPalette::Base, qcolor);
        palette.setColor(QPalette::Text, qcolor.lightness() < 128 ? QColor(255, 255, 255) : QColor(0, 0, 0));
    }
    else
    {
        palette.setColor(QPalette::Base, QApplication::palette().color(QPalette::Base));
        palette.setColor(QPalette::Text, QApplication::palette().color(QPalette::Text));
    }
    field->setPalette(palette);
}

int ToolBarColorimetricFilter::findFirstEmptyColor() const
{
    for (int i = 0; i < kColorCount; ++i)
    {
        if (!m_settings.colorsEnabled[i])
            return i;
    }
    return -1;
}

bool ToolBarColorimetricFilter::parseRgb(const QString& value, Color32& outColor) const
{
    QStringList parts = value.split(',', Qt::SkipEmptyParts);
    if (parts.size() != 3)
        return false;

    bool ok = false;
    int rgb[3] = { 0, 0, 0 };
    for (int i = 0; i < 3; ++i)
    {
        rgb[i] = parts[i].trimmed().toInt(&ok);
        if (!ok || rgb[i] < 0 || rgb[i] > 255)
            return false;
    }

    outColor = Color32(rgb[0], rgb[1], rgb[2]);
    return true;
}

bool ToolBarColorimetricFilter::parseIntensity(const QString& value, Color32& outColor) const
{
    bool ok = false;
    int intensity = value.trimmed().toInt(&ok);
    if (!ok || intensity < 0 || intensity > 255)
        return false;

    outColor = Color32(intensity, intensity, intensity);
    return true;
}

void ToolBarColorimetricFilter::startPicking()
{
    if (m_isPicking)
        return;

    m_isPicking = true;
    QApplication::setOverrideCursor(Qt::CrossCursor);
    m_dataDispatcher.sendControl(new control::picking::PickColorimetricFilterFromPick());
}

void ToolBarColorimetricFilter::stopPicking()
{
    if (!m_isPicking)
        return;

    QApplication::restoreOverrideCursor();
    m_isPicking = false;
}
