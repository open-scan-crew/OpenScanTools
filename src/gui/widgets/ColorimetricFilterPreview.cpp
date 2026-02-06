#include "gui/widgets/ColorimetricFilterPreview.h"

#include <QPainter>
#include <QPaintEvent>

#include <cmath>

namespace
{
    QColor toQColor(const Color32& color)
    {
        return QColor(color.r, color.g, color.b);
    }
}

ColorimetricFilterPreview::ColorimetricFilterPreview(QWidget* parent)
    : QWidget(parent)
{
    setMinimumSize(80, 40);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}

void ColorimetricFilterPreview::setColors(const std::array<Color32, 4>& colors, int count)
{
    m_colors = colors;
    m_count = count;
    update();
}

void ColorimetricFilterPreview::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    QRect rect = this->rect();

    if (m_count <= 0)
    {
        painter.fillRect(rect, palette().window());
        return;
    }

    if (m_count == 1)
    {
        painter.fillRect(rect, toQColor(m_colors[0]));
        return;
    }

    if (m_count == 2)
    {
        QLinearGradient gradient(rect.topLeft(), rect.topRight());
        gradient.setColorAt(0.0, toQColor(m_colors[0]));
        gradient.setColorAt(1.0, toQColor(m_colors[1]));
        painter.fillRect(rect, gradient);
        return;
    }

    QImage image(rect.size(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    int width = rect.width();
    int height = rect.height();

    if (m_count == 3)
    {
        QPointF a(0.0, 0.0);
        QPointF b(width - 1.0, 0.0);
        QPointF c(0.0, height - 1.0);
        QColor colorA = toQColor(m_colors[0]);
        QColor colorB = toQColor(m_colors[1]);
        QColor colorC = toQColor(m_colors[2]);

        float denom = float((b.y() - c.y()) * (a.x() - c.x()) + (c.x() - b.x()) * (a.y() - c.y()));
        if (std::abs(denom) < 1e-5f)
        {
            painter.fillRect(rect, palette().window());
            return;
        }

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                float w1 = float((b.y() - c.y()) * (x - c.x()) + (c.x() - b.x()) * (y - c.y())) / denom;
                float w2 = float((c.y() - a.y()) * (x - c.x()) + (a.x() - c.x()) * (y - c.y())) / denom;
                float w3 = 1.0f - w1 - w2;
                if (w1 < 0.0f || w2 < 0.0f || w3 < 0.0f)
                    continue;

                int r = int(w1 * colorA.red() + w2 * colorB.red() + w3 * colorC.red());
                int g = int(w1 * colorA.green() + w2 * colorB.green() + w3 * colorC.green());
                int bcol = int(w1 * colorA.blue() + w2 * colorB.blue() + w3 * colorC.blue());
                image.setPixelColor(x, y, QColor(r, g, bcol));
            }
        }

        painter.drawImage(rect.topLeft(), image);
        return;
    }

    QColor colorTL = toQColor(m_colors[0]);
    QColor colorTR = toQColor(m_colors[1]);
    QColor colorBL = toQColor(m_colors[2]);
    QColor colorBR = toQColor(m_colors[3]);

    for (int y = 0; y < height; ++y)
    {
        float fy = height > 1 ? float(y) / float(height - 1) : 0.0f;
        for (int x = 0; x < width; ++x)
        {
            float fx = width > 1 ? float(x) / float(width - 1) : 0.0f;
            float r = (1.0f - fx) * (1.0f - fy) * colorTL.red() + fx * (1.0f - fy) * colorTR.red() +
                (1.0f - fx) * fy * colorBL.red() + fx * fy * colorBR.red();
            float g = (1.0f - fx) * (1.0f - fy) * colorTL.green() + fx * (1.0f - fy) * colorTR.green() +
                (1.0f - fx) * fy * colorBL.green() + fx * fy * colorBR.green();
            float bcol = (1.0f - fx) * (1.0f - fy) * colorTL.blue() + fx * (1.0f - fy) * colorTR.blue() +
                (1.0f - fx) * fy * colorBL.blue() + fx * fy * colorBR.blue();
            image.setPixelColor(x, y, QColor(int(r), int(g), int(bcol)));
        }
    }

    painter.drawImage(rect.topLeft(), image);
}
