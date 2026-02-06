#ifndef COLORIMETRIC_FILTER_PREVIEW_H
#define COLORIMETRIC_FILTER_PREVIEW_H

#include <QWidget>

#include "utils/Color32.hpp"

#include <array>

class ColorimetricFilterPreview : public QWidget
{
    Q_OBJECT

public:
    explicit ColorimetricFilterPreview(QWidget* parent = nullptr);

    void setColors(const std::array<Color32, 4>& colors, int count);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    std::array<Color32, 4> m_colors;
    int m_count = 0;
};

#endif
