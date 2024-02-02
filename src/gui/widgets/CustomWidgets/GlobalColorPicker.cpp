#include "gui/widgets/CustomWidgets/GlobalColorPicker.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "ui_widget_colorpicker.h"
#include "qpushbutton.h"

GlobalColorPicker::GlobalColorPicker(IDataDispatcher* dataDispatcher, QWidget* parent, float pixelRatio)
	: ColorPicker(parent, pixelRatio)
	, m_dataDispatcher(dataDispatcher)
{
	m_dataDispatcher->registerObserverOnKey(this, guiDType::globalColorPickerValue);
}

GlobalColorPicker::GlobalColorPicker(QWidget* parent, float pixelRatio)
	: ColorPicker(parent, pixelRatio)
	, m_dataDispatcher(nullptr)
{}

void GlobalColorPicker::setDataDispatcher(IDataDispatcher* dataDispatcher)
{
	if (!m_dataDispatcher)
	{
		m_dataDispatcher = dataDispatcher;
		m_dataDispatcher->registerObserverOnKey(this, guiDType::globalColorPickerValue);
	}
}

GlobalColorPicker::~GlobalColorPicker()
{
	m_dataDispatcher->unregisterObserver(this);
}

void GlobalColorPicker::informData(IGuiData* keyValue)
{
	if (keyValue->getType() != guiDType::globalColorPickerValue)
		return;
	auto colorData = static_cast<GuiDataGlobalColorPickerValue*>(keyValue);
	if(colorData->m_isPicked)
		setColorChecked(colorData->m_color);
}

void GlobalColorPicker::selectColor(QPushButton* qbutton)
{
	if (m_autoUnselect == true)
	{
		m_ui.button1->setChecked(false);
		m_ui.button2->setChecked(false);
		m_ui.button3->setChecked(false);
		m_ui.button4->setChecked(false);
		m_ui.button5->setChecked(false);
		m_ui.button6->setChecked(false);
		m_ui.button7->setChecked(false);
		m_ui.button8->setChecked(false);

		qbutton->setChecked(true);

		QColor color = qbutton->palette().color(QPalette::Button);

		Color32 c32(color.red(), color.green(), color.blue(), color.alpha());
		emit pickedColor(c32);
		if (m_dataDispatcher)
			m_dataDispatcher->updateInformation(new GuiDataGlobalColorPickerValue(c32), this);
	}
	else
	{
		if (qbutton->isChecked() == true)
		{
			qbutton->setChecked(true);

			QColor color = qbutton->palette().color(QPalette::Button);
			Color32 c32(color.red(), color.green(), color.blue(), color.alpha());
			emit pickedColor(c32);
			if (m_dataDispatcher)
				m_dataDispatcher->updateInformation(new GuiDataGlobalColorPickerValue(c32), this);
		}
		else
		{
			qbutton->setChecked(false);

			QColor color = qbutton->palette().color(QPalette::Button);

			Color32 c32(color.red(), color.green(), color.blue(), color.alpha());
			emit unPickedColor(c32);
			if (m_dataDispatcher)
				m_dataDispatcher->updateInformation(new GuiDataGlobalColorPickerValue(c32,false), this);
		}
	}
}
