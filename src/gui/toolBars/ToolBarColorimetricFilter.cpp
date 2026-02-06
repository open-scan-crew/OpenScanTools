#include "gui/toolBars/ToolBarColorimetricFilter.h"

#include "controller/controls/ControlPicking.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "models/graph/CameraNode.h"
#include "utils/QtLogStream.hpp"

#include <QApplication>
#include <QCursor>
#include <QGuiApplication>
#include <QMouseEvent>
#include <QScreen>
#include <QRegExp>

#include <algorithm>
#include <cmath>

namespace
{
    constexpr float kLabLRange = 100.0f;
    constexpr float kLabABRange = 255.0f;

    bool isIntensityMode(UiRenderMode mode)
    {
        return mode == UiRenderMode::Intensity || mode == UiRenderMode::Fake_Color;
    }
}

ToolBarColorimetricFilter::ToolBarColorimetricFilter(IDataDispatcher& dataDispatcher, QWidget* parent, float guiScale)
    : QWidget(parent)
    , m_dataDispatcher(dataDispatcher)
{
    m_ui.setupUi(this);

    m_ui.combo_sourceColor->addItem(tr("Point RGB"), static_cast<int>(ColorimetricFilterSource::PointRgb));
    m_ui.combo_sourceColor->addItem(tr("Pixel RGB"), static_cast<int>(ColorimetricFilterSource::PixelRgb));
    m_ui.combo_colorSpace->addItem(tr("RGB"), static_cast<int>(ColorimetricFilterSpace::RGB));
    m_ui.combo_colorSpace->addItem(tr("LAB"), static_cast<int>(ColorimetricFilterSpace::LAB));

    m_ui.spin_tolerance->setDecimals(1);
    m_ui.spin_tolerance->setMinimum(0.0);
    m_ui.spin_tolerance->setMaximum(100.0);
    m_ui.spin_tolerance->setValue(0.0);

    registerGuiDataFunction(guiDType::renderActiveCamera, &ToolBarColorimetricFilter::onActiveCamera);
    registerGuiDataFunction(guiDType::focusViewport, &ToolBarColorimetricFilter::onFocusViewport);
    registerGuiDataFunction(guiDType::renderColorMode, &ToolBarColorimetricFilter::onRenderColorMode);
    registerGuiDataFunction(guiDType::colorimetricFilterPickedColor, &ToolBarColorimetricFilter::onPickedColor);

    connect(m_ui.button_apply, &QPushButton::clicked, this, &ToolBarColorimetricFilter::slotApply);
    connect(m_ui.button_reset, &QPushButton::clicked, this, &ToolBarColorimetricFilter::slotReset);
    connect(m_ui.button_pickColor1, &QToolButton::clicked, this, &ToolBarColorimetricFilter::slotPickColor);
    connect(m_ui.button_pickColor2, &QToolButton::clicked, this, &ToolBarColorimetricFilter::slotPickColor);
    connect(m_ui.button_pickColor3, &QToolButton::clicked, this, &ToolBarColorimetricFilter::slotPickColor);
    connect(m_ui.button_pickColor4, &QToolButton::clicked, this, &ToolBarColorimetricFilter::slotPickColor);
    connect(m_ui.combo_colorSpace, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ToolBarColorimetricFilter::slotColorSpaceChanged);

    auto connectEditing = [this](QLineEdit* lineEdit, int index)
    {
        connect(lineEdit, &QLineEdit::editingFinished, this, [this, index]() {
            glm::vec3 value;
            QLineEdit* fields[] = { m_ui.lineEdit_color1, m_ui.lineEdit_color2, m_ui.lineEdit_color3, m_ui.lineEdit_color4 };
            bool active = parseColorText(fields[index]->text(), value);
            updateColorField(index, value, active);
            updatePreview();
        });
    };

    connectEditing(m_ui.lineEdit_color1, 0);
    connectEditing(m_ui.lineEdit_color2, 1);
    connectEditing(m_ui.lineEdit_color3, 2);
    connectEditing(m_ui.lineEdit_color4, 3);

    Q_UNUSED(guiScale);
    updatePreview();
}

ToolBarColorimetricFilter::~ToolBarColorimetricFilter()
{
    stopPixelPicking();
}

void ToolBarColorimetricFilter::informData(IGuiData* data)
{
    if (m_methods.find(data->getType()) != m_methods.end())
    {
        GuiDataFunction fct = m_methods.at(data->getType());
        (this->*fct)(data);
    }
}

void ToolBarColorimetricFilter::onActiveCamera(IGuiData* data)
{
    auto castData = static_cast<GuiDataCameraInfo*>(data);
    m_focusCamera = castData->m_camera;
    if (ReadPtr<CameraNode> rCam = m_focusCamera.cget())
    {
        m_renderMode = rCam->getDisplayParameters().m_mode;
        updateFromDisplayParameters(rCam->getDisplayParameters());
    }
}

void ToolBarColorimetricFilter::onFocusViewport(IGuiData* data)
{
    GuiDataFocusViewport* castData = static_cast<GuiDataFocusViewport*>(data);
    if (castData->m_forceFocus && castData->m_camera)
        m_focusCamera = castData->m_camera;
    if (ReadPtr<CameraNode> rCam = m_focusCamera.cget())
    {
        m_renderMode = rCam->getDisplayParameters().m_mode;
        updateFromDisplayParameters(rCam->getDisplayParameters());
    }
}

void ToolBarColorimetricFilter::onRenderColorMode(IGuiData* data)
{
    auto castData = static_cast<GuiDataRenderColorMode*>(data);
    m_renderMode = castData->m_mode;
    updateIntensityMode();
    updatePreview();
}

void ToolBarColorimetricFilter::onPickedColor(IGuiData* data)
{
    auto castData = static_cast<GuiDataColorimetricFilterPickedColor*>(data);
    if (castData->m_picked)
        applyPickedColor(castData->m_color);
}

void ToolBarColorimetricFilter::updateFromDisplayParameters(const DisplayParameters& params)
{
    const ColorimetricFilterParameters& filter = params.m_colorimetricFilter;
    m_ui.combo_sourceColor->setCurrentIndex(m_ui.combo_sourceColor->findData(static_cast<int>(filter.source)));
    m_ui.combo_colorSpace->setCurrentIndex(m_ui.combo_colorSpace->findData(static_cast<int>(filter.space)));
    m_ui.spin_tolerance->setValue(filter.tolerance);
    m_ui.radio_show->setChecked(filter.action == ColorimetricFilterAction::Show);
    m_ui.radio_hide->setChecked(filter.action == ColorimetricFilterAction::Hide);

    m_colors = filter.colors;
    for (size_t i = 0; i < m_colors.size(); ++i)
    {
        bool active = static_cast<int>(i) < filter.colorCount;
        updateColorField(static_cast<int>(i), m_colors[i], active);
    }

    updateIntensityMode();
    updatePreview();
}

void ToolBarColorimetricFilter::updateIntensityMode()
{
    bool intensity = isIntensityMode(m_renderMode);
    m_ui.combo_colorSpace->setEnabled(!intensity);

    QLineEdit* fields[] = { m_ui.lineEdit_color1, m_ui.lineEdit_color2, m_ui.lineEdit_color3, m_ui.lineEdit_color4 };
    QToolButton* buttons[] = { m_ui.button_pickColor1, m_ui.button_pickColor2, m_ui.button_pickColor3, m_ui.button_pickColor4 };
    for (int i = 1; i < 4; ++i)
    {
        fields[i]->setEnabled(!intensity);
        buttons[i]->setEnabled(!intensity);
    }

    const QString placeholder = intensity ? tr("I") : (m_ui.combo_colorSpace->currentData().toInt() == static_cast<int>(ColorimetricFilterSpace::RGB)
        ? tr("R, G, B")
        : tr("L, a, b"));
    for (int i = 0; i < 4; ++i)
        fields[i]->setPlaceholderText(placeholder);
}

void ToolBarColorimetricFilter::updatePreview()
{
    int width = std::max(1, m_ui.label_preview->width());
    int height = std::max(1, m_ui.label_preview->height());

    QImage image(width, height, QImage::Format_ARGB32);
    image.fill(Qt::transparent);

    int count = 0;
    for (int i = 0; i < 4; ++i)
    {
        if (m_colorActive[i])
            count++;
        else
            break;
    }

    auto toRgb = [this](const glm::vec3& value) {
        glm::vec3 rgb = (m_ui.combo_colorSpace->currentData().toInt() == static_cast<int>(ColorimetricFilterSpace::LAB)) ? labToRgb(value) : value;
        rgb = glm::clamp(rgb, glm::vec3(0.f), glm::vec3(255.f));
        return QColor(static_cast<int>(rgb.x), static_cast<int>(rgb.y), static_cast<int>(rgb.z));
    };

    if (count <= 0)
    {
        image.fill(QColor(30, 30, 30));
    }
    else if (count == 1)
    {
        image.fill(toRgb(m_colors[0]));
    }
    else if (count == 2)
    {
        for (int x = 0; x < width; ++x)
        {
            float t = (width == 1) ? 0.f : static_cast<float>(x) / static_cast<float>(width - 1);
            glm::vec3 color = (1.0f - t) * m_colors[0] + t * m_colors[1];
            QColor qcolor = toRgb(color);
            for (int y = 0; y < height; ++y)
                image.setPixelColor(x, y, qcolor);
        }
    }
    else if (count == 3)
    {
        glm::vec2 a(0.f, 0.f);
        glm::vec2 b(width - 1.f, 0.f);
        glm::vec2 c(width * 0.5f, height - 1.f);
        float denom = (b.y - c.y) * (a.x - c.x) + (c.x - b.x) * (a.y - c.y);
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                glm::vec2 p(static_cast<float>(x), static_cast<float>(y));
                float w1 = ((b.y - c.y) * (p.x - c.x) + (c.x - b.x) * (p.y - c.y)) / denom;
                float w2 = ((c.y - a.y) * (p.x - c.x) + (a.x - c.x) * (p.y - c.y)) / denom;
                float w3 = 1.0f - w1 - w2;
                if (w1 >= 0.f && w2 >= 0.f && w3 >= 0.f)
                {
                    glm::vec3 color = w1 * m_colors[0] + w2 * m_colors[1] + w3 * m_colors[2];
                    image.setPixelColor(x, y, toRgb(color));
                }
            }
        }
    }
    else
    {
        for (int y = 0; y < height; ++y)
        {
            float v = (height == 1) ? 0.f : static_cast<float>(y) / static_cast<float>(height - 1);
            for (int x = 0; x < width; ++x)
            {
                float u = (width == 1) ? 0.f : static_cast<float>(x) / static_cast<float>(width - 1);
                glm::vec3 color = (1.0f - u) * (1.0f - v) * m_colors[0]
                    + u * (1.0f - v) * m_colors[1]
                    + u * v * m_colors[2]
                    + (1.0f - u) * v * m_colors[3];
                image.setPixelColor(x, y, toRgb(color));
            }
        }
    }

    m_ui.label_preview->setPixmap(QPixmap::fromImage(image));
}

void ToolBarColorimetricFilter::updateColorField(int index, const glm::vec3& value, bool active)
{
    QLineEdit* fields[] = { m_ui.lineEdit_color1, m_ui.lineEdit_color2, m_ui.lineEdit_color3, m_ui.lineEdit_color4 };
    if (index < 0 || index >= 4)
        return;

    m_colors[index] = value;
    m_colorActive[index] = active;
    fields[index]->setText(active ? formatColorText(value) : QString());

    if (!active)
    {
        fields[index]->setStyleSheet(QString());
        return;
    }

    glm::vec3 rgb = value;
    if (isIntensityMode(m_renderMode))
        rgb = glm::vec3(value.x);
    else if (m_ui.combo_colorSpace->currentData().toInt() == static_cast<int>(ColorimetricFilterSpace::LAB))
        rgb = labToRgb(value);
    rgb = glm::clamp(rgb, glm::vec3(0.f), glm::vec3(255.f));
    fields[index]->setStyleSheet(QString("QLineEdit { background-color: rgb(%1, %2, %3); }")
        .arg(static_cast<int>(rgb.x))
        .arg(static_cast<int>(rgb.y))
        .arg(static_cast<int>(rgb.z)));
}

int ToolBarColorimetricFilter::firstAvailableColorIndex() const
{
    for (int i = 0; i < 4; ++i)
    {
        if (!m_colorActive[i])
            return i;
    }
    return 0;
}

void ToolBarColorimetricFilter::applyPickedColor(const Color32& color)
{
    int index = firstAvailableColorIndex();
    glm::vec3 rgb(static_cast<float>(color.r), static_cast<float>(color.g), static_cast<float>(color.b));
    glm::vec3 value = (m_ui.combo_colorSpace->currentData().toInt() == static_cast<int>(ColorimetricFilterSpace::LAB)) ? rgbToLab(rgb) : rgb;
    updateColorField(index, value, true);
    updatePreview();
}

bool ToolBarColorimetricFilter::parseColorText(const QString& text, glm::vec3& outColor) const
{
    QString trimmed = text.trimmed();
    if (trimmed.isEmpty())
        return false;

    QStringList parts = trimmed.split(QRegExp("[,\\s]+"), Qt::SkipEmptyParts);
    if (isIntensityMode(m_renderMode))
    {
        bool ok = false;
        float value = parts.value(0).toFloat(&ok);
        if (!ok)
            return false;
        value = std::clamp(value, 0.0f, 255.0f);
        outColor = glm::vec3(value, 0.f, 0.f);
        return true;
    }

    if (parts.size() < 3)
        return false;

    bool ok1 = false;
    bool ok2 = false;
    bool ok3 = false;
    float v1 = parts[0].toFloat(&ok1);
    float v2 = parts[1].toFloat(&ok2);
    float v3 = parts[2].toFloat(&ok3);
    if (!ok1 || !ok2 || !ok3)
        return false;

    if (m_ui.combo_colorSpace->currentData().toInt() == static_cast<int>(ColorimetricFilterSpace::RGB))
    {
        v1 = std::clamp(v1, 0.0f, 255.0f);
        v2 = std::clamp(v2, 0.0f, 255.0f);
        v3 = std::clamp(v3, 0.0f, 255.0f);
    }
    else
    {
        v1 = std::clamp(v1, 0.0f, kLabLRange);
        v2 = std::clamp(v2, -kLabABRange / 2.0f, kLabABRange / 2.0f);
        v3 = std::clamp(v3, -kLabABRange / 2.0f, kLabABRange / 2.0f);
    }

    outColor = glm::vec3(v1, v2, v3);
    return true;
}

QString ToolBarColorimetricFilter::formatColorText(const glm::vec3& value) const
{
    if (isIntensityMode(m_renderMode))
        return QString::number(value.x, 'f', 0);

    if (m_ui.combo_colorSpace->currentData().toInt() == static_cast<int>(ColorimetricFilterSpace::RGB))
    {
        return QStringLiteral("%1, %2, %3")
            .arg(QString::number(static_cast<int>(std::round(value.x))))
            .arg(QString::number(static_cast<int>(std::round(value.y))))
            .arg(QString::number(static_cast<int>(std::round(value.z))));
    }

    return QStringLiteral("%1, %2, %3")
        .arg(QString::number(value.x, 'f', 1))
        .arg(QString::number(value.y, 'f', 1))
        .arg(QString::number(value.z, 'f', 1));
}

glm::vec3 ToolBarColorimetricFilter::rgbToLab(const glm::vec3& rgb) const
{
    glm::vec3 srgb = rgb / 255.0f;
    glm::vec3 linear;
    for (int i = 0; i < 3; ++i)
    {
        float c = srgb[i];
        linear[i] = (c > 0.04045f) ? std::pow((c + 0.055f) / 1.055f, 2.4f) : (c / 12.92f);
    }

    glm::vec3 xyz;
    xyz.x = linear.x * 0.4124f + linear.y * 0.3576f + linear.z * 0.1805f;
    xyz.y = linear.x * 0.2126f + linear.y * 0.7152f + linear.z * 0.0722f;
    xyz.z = linear.x * 0.0193f + linear.y * 0.1192f + linear.z * 0.9505f;

    glm::vec3 ref(0.95047f, 1.0f, 1.08883f);
    glm::vec3 v = xyz / ref;
    auto pivot = [](float t) { return (t > 0.008856f) ? std::pow(t, 1.0f / 3.0f) : (7.787f * t + 16.0f / 116.0f); };
    v = glm::vec3(pivot(v.x), pivot(v.y), pivot(v.z));

    float L = std::max(0.0f, 116.0f * v.y - 16.0f);
    float a = 500.0f * (v.x - v.y);
    float b = 200.0f * (v.y - v.z);
    return glm::vec3(L, a, b);
}

glm::vec3 ToolBarColorimetricFilter::labToRgb(const glm::vec3& lab) const
{
    float y = (lab.x + 16.0f) / 116.0f;
    float x = lab.y / 500.0f + y;
    float z = y - lab.z / 200.0f;

    auto pivot = [](float t) { return (t * t * t > 0.008856f) ? t * t * t : (t - 16.0f / 116.0f) / 7.787f; };
    glm::vec3 xyz(pivot(x), pivot(y), pivot(z));
    xyz *= glm::vec3(0.95047f, 1.0f, 1.08883f);

    glm::vec3 linear;
    linear.x = xyz.x * 3.2406f + xyz.y * -1.5372f + xyz.z * -0.4986f;
    linear.y = xyz.x * -0.9689f + xyz.y * 1.8758f + xyz.z * 0.0415f;
    linear.z = xyz.x * 0.0557f + xyz.y * -0.2040f + xyz.z * 1.0570f;

    glm::vec3 srgb;
    for (int i = 0; i < 3; ++i)
    {
        float c = linear[i];
        srgb[i] = (c > 0.0031308f) ? (1.055f * std::pow(c, 1.0f / 2.4f) - 0.055f) : (12.92f * c);
    }

    return glm::clamp(srgb * 255.0f, glm::vec3(0.0f), glm::vec3(255.0f));
}

void ToolBarColorimetricFilter::reorderConvexColors(std::array<glm::vec3, 4>& colors, int count) const
{
    if (count != 4)
        return;

    glm::vec3 centroid(0.0f);
    for (int i = 0; i < 4; ++i)
        centroid += colors[i];
    centroid /= 4.0f;

    glm::vec3 normal = glm::cross(colors[1] - colors[0], colors[2] - colors[0]);
    if (glm::length(normal) < 1e-4f)
        return;

    glm::vec3 axisU = glm::normalize(colors[1] - colors[0]);
    glm::vec3 axisV = glm::normalize(glm::cross(normal, axisU));

    std::array<std::pair<float, glm::vec3>, 4> ordered;
    for (int i = 0; i < 4; ++i)
    {
        glm::vec3 vec = colors[i] - centroid;
        float angle = std::atan2(glm::dot(axisV, vec), glm::dot(axisU, vec));
        ordered[i] = { angle, colors[i] };
    }

    std::sort(ordered.begin(), ordered.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
    for (int i = 0; i < 4; ++i)
        colors[i] = ordered[i].second;
}

bool ToolBarColorimetricFilter::eventFilter(QObject* watched, QEvent* event)
{
    if (!m_pixelPickActive)
        return QWidget::eventFilter(watched, event);

    if (event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        QPoint globalPos = mouseEvent->globalPos();
        QScreen* screen = QGuiApplication::screenAt(globalPos);
        if (!screen)
            screen = QGuiApplication::primaryScreen();

        if (screen)
        {
            QPixmap pixmap = screen->grabWindow(0, globalPos.x(), globalPos.y(), 1, 1);
            QImage image = pixmap.toImage();
            QColor color = image.pixelColor(0, 0);
            applyPickedColor(Color32(color.red(), color.green(), color.blue(), color.alpha()));
        }

        stopPixelPicking();
        return true;
    }

    return QWidget::eventFilter(watched, event);
}

void ToolBarColorimetricFilter::startPixelPicking()
{
    if (m_pixelPickActive)
        return;
    m_pixelPickActive = true;
    QApplication::setOverrideCursor(Qt::CrossCursor);
    qApp->installEventFilter(this);
}

void ToolBarColorimetricFilter::stopPixelPicking()
{
    if (!m_pixelPickActive)
        return;
    m_pixelPickActive = false;
    qApp->removeEventFilter(this);
    QApplication::restoreOverrideCursor();
}

void ToolBarColorimetricFilter::slotApply()
{
    if (!m_focusCamera)
        return;

    ColorimetricFilterParameters filter;
    filter.source = static_cast<ColorimetricFilterSource>(m_ui.combo_sourceColor->currentData().toInt());
    filter.space = static_cast<ColorimetricFilterSpace>(m_ui.combo_colorSpace->currentData().toInt());
    filter.action = m_ui.radio_hide->isChecked() ? ColorimetricFilterAction::Hide : ColorimetricFilterAction::Show;
    filter.tolerance = static_cast<float>(m_ui.spin_tolerance->value());

    QLineEdit* fields[] = { m_ui.lineEdit_color1, m_ui.lineEdit_color2, m_ui.lineEdit_color3, m_ui.lineEdit_color4 };
    int count = 0;
    for (int i = 0; i < 4; ++i)
    {
        glm::vec3 value;
        bool active = parseColorText(fields[i]->text(), value);
        if (isIntensityMode(m_renderMode) && i > 0)
            active = false;

        m_colorActive[i] = active;
        m_colors[i] = value;

        if (active)
            count++;
        else
            break;
    }

    filter.colorCount = count;
    filter.enabled = count > 0;
    filter.colors = m_colors;

    reorderConvexColors(filter.colors, count);

    m_dataDispatcher.updateInformation(new GuiDataRenderColorimetricFilter(filter, m_focusCamera), this);
    for (int i = 0; i < 4; ++i)
    {
        bool active = i < count;
        updateColorField(i, filter.colors[i], active);
    }
    updatePreview();
}

void ToolBarColorimetricFilter::slotReset()
{
    QLineEdit* fields[] = { m_ui.lineEdit_color1, m_ui.lineEdit_color2, m_ui.lineEdit_color3, m_ui.lineEdit_color4 };
    for (int i = 0; i < 4; ++i)
    {
        m_colors[i] = glm::vec3(0.f);
        m_colorActive[i] = false;
        fields[i]->setText(formatColorText(glm::vec3(0.f)));
        fields[i]->setStyleSheet(QString());
    }

    updatePreview();
}

void ToolBarColorimetricFilter::slotPickColor()
{
    if (!m_focusCamera)
        return;

    auto source = static_cast<ColorimetricFilterSource>(m_ui.combo_sourceColor->currentData().toInt());
    if (source == ColorimetricFilterSource::PointRgb)
    {
        m_dataDispatcher.sendControl(new control::picking::PickColorFromPick());
        return;
    }

    startPixelPicking();
}

void ToolBarColorimetricFilter::slotColorSpaceChanged(int)
{
    updateIntensityMode();
    updatePreview();
}
