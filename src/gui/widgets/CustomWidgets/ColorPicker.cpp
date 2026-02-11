#include "gui/widgets/CustomWidgets/ColorPicker.h"

#include "utils/ProjectColor.hpp"
#include "utils/QtLogStream.hpp"

ColorPicker::ColorPicker(QWidget *parent, float pixelRatio)
    : QWidget(parent)
{
    m_ui.setupUi(this);
	m_autoUnselect = true;

    QSize buttonSize = QSize(22 * pixelRatio, 22 * pixelRatio);
    std::vector<std::string> colorName = { "GREEN", "RED", "ORANGE", "YELLOW", "BLUE", "PURPLE", "LIGHT GREY", "BROWN" };

    for (int i = 0; i < m_ui.horizontalLayout->count(); i++)
    {
        QWidget* button = m_ui.horizontalLayout->itemAt(i)->widget();
        button->setMaximumSize(buttonSize);
        button->setMinimumSize(buttonSize);

        // Style sheet
        Color32 color = ProjectColor::getColor(colorName[i]);
        QColor bg = QColorFromColor32(color);
        QColor border;
        //float grey = (float)(bg.red() + bg.green() + bg.blue()) / (3 * 255.f);
        float grey = color.greyEquivalent();

        if (grey > 0.5f)
            border = QColor(0, 0, 0);
        else
            border = QColor(255, 255, 255);
        int borderSize = 2 * pixelRatio;

        QString styleSheetText = QString(
            // the native border overlap the background-color so we must disable it
            "QPushButton {"
            "   background-color: rgb(%1, %2, %3);"
            "   border: none;"
            "}"
            ""
            "QPushButton:checked {"
            "   border: %7px solid rgb(%4, %5, %6);"
            //"   margin: 1px;"
            //"   image: url(:/icons/100x100/check.png);"
            "}"
        ).arg(bg.red()).arg(bg.green()).arg(bg.blue()).arg(border.red()).arg(border.green()).arg(border.blue()).arg(borderSize);

        button->setStyleSheet(styleSheetText);
    }

    QObject::connect(m_ui.button1, &QPushButton::clicked, this, [this]() { selectColor(m_ui.button1); });
    QObject::connect(m_ui.button2, &QPushButton::clicked, this, [this]() { selectColor(m_ui.button2); });
    QObject::connect(m_ui.button3, &QPushButton::clicked, this, [this]() { selectColor(m_ui.button3); });
    QObject::connect(m_ui.button4, &QPushButton::clicked, this, [this]() { selectColor(m_ui.button4); });
    QObject::connect(m_ui.button5, &QPushButton::clicked, this, [this]() { selectColor(m_ui.button5); });
    QObject::connect(m_ui.button6, &QPushButton::clicked, this, [this]() { selectColor(m_ui.button6); });
    QObject::connect(m_ui.button7, &QPushButton::clicked, this, [this]() { selectColor(m_ui.button7); });
    QObject::connect(m_ui.button8, &QPushButton::clicked, this, [this]() { selectColor(m_ui.button8); });
}


ColorPicker::~ColorPicker()
{}

void ColorPicker::setAutoUnselect(bool value)
{
	m_autoUnselect = value;
}

void ColorPicker::clearSelection()
{
    m_ui.button1->blockSignals(true);
    m_ui.button2->blockSignals(true);
    m_ui.button3->blockSignals(true);
    m_ui.button4->blockSignals(true);
    m_ui.button5->blockSignals(true);
    m_ui.button6->blockSignals(true);
    m_ui.button7->blockSignals(true);
    m_ui.button8->blockSignals(true);

    m_ui.button1->setChecked(false);
    m_ui.button2->setChecked(false);
    m_ui.button3->setChecked(false);
    m_ui.button4->setChecked(false);
    m_ui.button5->setChecked(false);
    m_ui.button6->setChecked(false);
    m_ui.button7->setChecked(false);
    m_ui.button8->setChecked(false);

    m_ui.button1->blockSignals(false);
    m_ui.button2->blockSignals(false);
    m_ui.button3->blockSignals(false);
    m_ui.button4->blockSignals(false);
    m_ui.button5->blockSignals(false);
    m_ui.button6->blockSignals(false);
    m_ui.button7->blockSignals(false);
    m_ui.button8->blockSignals(false);
}

void ColorPicker::setColorChecked(Color32 color)
{
    for (int i = 0; i < m_ui.horizontalLayout->count(); i++)
    {
        QWidget* widget = m_ui.horizontalLayout->itemAt(i)->widget();

        QPushButton* button = dynamic_cast<QPushButton*>(widget);
        if (button->palette().color(QPalette::Button) == color)
            button->setChecked(true);
        else
            button->setChecked(false);
    }
}

void ColorPicker::selectColor(QPushButton *qbutton)
{
	if (m_autoUnselect == true)
	{
        clearSelection();

		qbutton->setChecked(true);

		QColor color = qbutton->palette().color(QPalette::Button);

		emit pickedColor(Color32(color.red(), color.green(), color.blue(), color.alpha()));
	}
	else
	{
		if (qbutton->isChecked() == true)
		{
			qbutton->setChecked(true);

			QColor color = qbutton->palette().color(QPalette::Button);

			emit pickedColor(Color32(color.red(), color.green(), color.blue(), color.alpha()));
		}
		else
		{
			qbutton->setChecked(false);

			QColor color = qbutton->palette().color(QPalette::Button);

			emit unPickedColor(Color32(color.red(), color.green(), color.blue(), color.alpha()));
		}
	}
}