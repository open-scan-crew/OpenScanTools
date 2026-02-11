#include "gui/widgets/CustomWidgets/ColorimetricFilterPreview.h"

#include <QPainter>
#include <QImage>

namespace
{
	QColor toQColor(const Color32& color)
	{
		return QColor(color.Red(), color.Green(), color.Blue());
	}

	QColor mixColor(const QColor& a, const QColor& b, float t)
	{
		float it = 1.0f - t;
		return QColor(static_cast<int>(a.red() * it + b.red() * t),
					  static_cast<int>(a.green() * it + b.green() * t),
					  static_cast<int>(a.blue() * it + b.blue() * t));
	}
}

ColorimetricFilterPreview::ColorimetricFilterPreview(QWidget* parent)
	: QWidget(parent)
{
	setMinimumSize(QSize(60, 40));
}

void ColorimetricFilterPreview::setColors(const std::vector<Color32>& colors)
{
	m_colors = colors;
	update();
}

void ColorimetricFilterPreview::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event);
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, false);

	QImage image = buildPreviewImage(size());
	painter.drawImage(rect(), image);
}

QImage ColorimetricFilterPreview::buildPreviewImage(const QSize& size) const
{
	QImage image(size, QImage::Format_RGB32);
	image.fill(Qt::black);

	if (size.width() <= 0 || size.height() <= 0)
		return image;

	if (m_colors.empty())
		return image;

	if (m_colors.size() == 1)
	{
		image.fill(toQColor(m_colors[0]));
		return image;
	}

	const float width = static_cast<float>(size.width() - 1);
	const float height = static_cast<float>(size.height() - 1);

	QColor c0 = toQColor(m_colors[0]);
	QColor c1 = toQColor(m_colors.size() > 1 ? m_colors[1] : m_colors[0]);
	QColor c2 = toQColor(m_colors.size() > 2 ? m_colors[2] : m_colors[0]);
	QColor c3 = toQColor(m_colors.size() > 3 ? m_colors[3] : m_colors[0]);

	for (int y = 0; y < size.height(); ++y)
	{
		float v = height > 0.0f ? static_cast<float>(y) / height : 0.0f;
		for (int x = 0; x < size.width(); ++x)
		{
			float u = width > 0.0f ? static_cast<float>(x) / width : 0.0f;

			if (m_colors.size() == 2)
			{
				image.setPixelColor(x, y, mixColor(c0, c1, u));
				continue;
			}

			if (m_colors.size() == 3)
			{
				float w0 = 1.0f - u - v;
				float w1 = u;
				float w2 = v;
				if (w0 < 0.0f)
				{
					w0 = 0.0f;
					float sum = w1 + w2;
					if (sum > 0.0f)
					{
						w1 /= sum;
						w2 /= sum;
					}
				}
				QColor color(static_cast<int>(c0.red() * w0 + c1.red() * w1 + c2.red() * w2),
							 static_cast<int>(c0.green() * w0 + c1.green() * w1 + c2.green() * w2),
							 static_cast<int>(c0.blue() * w0 + c1.blue() * w1 + c2.blue() * w2));
				image.setPixelColor(x, y, color);
				continue;
			}

			float u0 = 1.0f - u;
			float v0 = 1.0f - v;
			QColor color(static_cast<int>(c0.red() * u0 * v0 + c1.red() * u * v0 + c2.red() * u0 * v + c3.red() * u * v),
						 static_cast<int>(c0.green() * u0 * v0 + c1.green() * u * v0 + c2.green() * u0 * v + c3.green() * u * v),
						 static_cast<int>(c0.blue() * u0 * v0 + c1.blue() * u * v0 + c2.blue() * u0 * v + c3.blue() * u * v));
			image.setPixelColor(x, y, color);
		}
	}

	return image;
}
