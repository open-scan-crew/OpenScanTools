#ifndef COLORIMETRIC_FILTER_PREVIEW_H
#define COLORIMETRIC_FILTER_PREVIEW_H

#include "utils/Color32.hpp"

#include <QWidget>

#include <array>
#include <vector>

class ColorimetricFilterPreview : public QWidget
{
	Q_OBJECT

public:
	explicit ColorimetricFilterPreview(QWidget* parent = nullptr);

	void setColors(const std::vector<Color32>& colors);

protected:
	void paintEvent(QPaintEvent* event) override;

private:
	QImage buildPreviewImage(const QSize& size) const;
	std::vector<Color32> m_colors;
};

#endif // COLORIMETRIC_FILTER_PREVIEW_H
