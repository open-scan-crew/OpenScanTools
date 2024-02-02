#ifndef COLOR_PICKER_H
#define COLOR_PICKER_H

#include <QWidget>
#include "ui_widget_colorpicker.h"

#include "utils/Color32.hpp"

class QPushButton;

class ColorPicker : public QWidget
{
    Q_OBJECT

public:
    ColorPicker(QWidget *parent = Q_NULLPTR, float pixelRatio = 1.f);
    ~ColorPicker();

    void setColorChecked(Color32 color);
	void setAutoUnselect(bool value);
    void clearSelection();

signals:
    void pickedColor(Color32 color);
	void unPickedColor(Color32 color);

protected slots:
    virtual void selectColor(QPushButton *qbutton);

protected:
	bool m_autoUnselect;
    Ui::ColorPicker m_ui;
    Color32 m_autoSelectColor;
};

#endif