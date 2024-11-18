#include "gui/widgets/PropertyClippingSettings.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataMeasure.h"
#include "gui/GuiData/GuiDataUserOrientation.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "controller/controls/ControlSMeasureEdition.h"
#include "controller/controls/ControlMeasure.h"
#include "controller/controls/ControlFunctionClipping.h"
#include "gui/widgets/FocusWatcher.h"
#include "utils/Logger.h"

#include <cctype>
#include <glm/gtx/vector_angle.hpp>

#include <QtGui/QHideEvent>

PropertyClippingSettings::PropertyClippingSettings(IDataDispatcher& dataDispatcher, QWidget *parent, float guiScale)
	: QWidget(parent)
	, m_ui(new Ui::PropertyClippingSettings)
	, m_dataDispatcher(dataDispatcher)
	, m_bpoints(false)
	, m_point(0)
	, m_angle(0)
	, m_userAngle(0)
{
	m_ui->setupUi(this);

	// Link action
	m_ui->CenterRadio->setChecked(true);
	QObject::connect(m_ui->XInfield, &QLineEdit::editingFinished, this, &PropertyClippingSettings::onXSizeEdit);
	QObject::connect(m_ui->YInfield, &QLineEdit::editingFinished, this, &PropertyClippingSettings::onYSizeEdit);
	QObject::connect(m_ui->ZInfield, &QLineEdit::editingFinished, this, &PropertyClippingSettings::onZSizeEdit);
	QObject::connect(m_ui->TopFaceRadio, SIGNAL(pressed()), this, SLOT(onTopClick()));
	QObject::connect(m_ui->CenterRadio, SIGNAL(pressed()), this, SLOT(onCenterClick()));
	QObject::connect(m_ui->BottomFaceRadio, SIGNAL(pressed()), this, SLOT(onBottomClick()));


	QObject::connect(m_ui->pointsRadioButton, SIGNAL(pressed()), this, SLOT(on2Points()));
	QObject::connect(m_ui->projectAxisRadioButton, SIGNAL(pressed()), this, SLOT(onProjectAxis()));
	QObject::connect(m_ui->userRadioButton, SIGNAL(pressed()), this, SLOT(onUserOrientation()));

	QObject::connect(m_ui->point1Button, SIGNAL(pressed()), this, SLOT(onPoint1Click()));
	QObject::connect(m_ui->point2Button, SIGNAL(pressed()), this, SLOT(onPoint2Click()));

	m_dataDispatcher.registerObserverOnKey(this, guiDType::point);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::abortEvent);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::userOrientation);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectOrientation);
	
	m_xStored = 1.f;
	m_yStored = 1.f;
	m_zStored = 1.f;

	m_ui->XInfield->setType(NumericType::DISTANCE);
	m_ui->YInfield->setType(NumericType::DISTANCE);
	m_ui->ZInfield->setType(NumericType::DISTANCE);

	m_ui->duplicationWidget->initialise(&dataDispatcher, guiScale);
}

PropertyClippingSettings::~PropertyClippingSettings()
{
	m_dataDispatcher.unregisterObserver(this);
}

void PropertyClippingSettings::hideEvent(QHideEvent* event)
{

	m_ui->XInfield->blockSignals(true);
	m_ui->YInfield->blockSignals(true);
	m_ui->ZInfield->blockSignals(true);
	m_ui->XP1->blockSignals(true);
	m_ui->XP1->blockSignals(true);
	m_ui->XP1->blockSignals(true);
	m_ui->XP2->blockSignals(true);
	m_ui->XP2->blockSignals(true);
	m_ui->XP2->blockSignals(true); 
	QWidget::hideEvent(event);
	m_ui->XP1->blockSignals(false);
	m_ui->XP1->blockSignals(false);
	m_ui->XP1->blockSignals(false);
	m_ui->XP2->blockSignals(false);
	m_ui->XP2->blockSignals(false);
	m_ui->XP2->blockSignals(false);
	m_ui->XInfield->blockSignals(false);
	m_ui->YInfield->blockSignals(false);
	m_ui->ZInfield->blockSignals(false);
    // QUESTION(robin) - Doit-on accepter l'event ou le transmettre à la méthode parent ?
    //QWidget::hideEvent(event);
    //event->accept();
}

void PropertyClippingSettings::showEvent(QShowEvent *event)
{
    m_ui->duplicationWidget->show();
    QWidget::showEvent(event);
}

void PropertyClippingSettings::showHideCreationSettings(bool hide)
{
	if (hide)
	{
		m_ui->alignementSettingsGroupBox->hide();
		m_ui->boxSizeGroupBox->hide();
	}
	else
	{
		m_ui->alignementSettingsGroupBox->show();
		m_ui->boxSizeGroupBox->show();
	}
}

void PropertyClippingSettings::informData(IGuiData *data)
{ 
	switch(data->getType()){
		case guiDType::abortEvent:
		{
			if (m_point <= 0)
				return;
			m_bpoints[m_point - 1] = false;
			m_point = 0;
			return;
		}
		break;
		case guiDType::point:
		{
			if (m_point <= 0)
				return;
			GuiDataPoint* point = static_cast<GuiDataPoint*>(data);
			switch (m_point)
			{
			case 1:
				m_ui->XP1->blockSignals(true);
				m_ui->XP1->blockSignals(true);
				m_ui->XP1->blockSignals(true);

				m_ui->XP1->setValue(point->m_point.x);
				m_ui->YP1->setValue(point->m_point.y);
				m_ui->ZP1->setValue(point->m_point.z);

				m_ui->XP1->blockSignals(false);
				m_ui->XP1->blockSignals(false);
				m_ui->XP1->blockSignals(false);
				break;
			case 2:
				m_ui->XP2->blockSignals(true);
				m_ui->XP2->blockSignals(true);
				m_ui->XP2->blockSignals(true);

				m_ui->XP2->setValue(point->m_point.x);
				m_ui->YP2->setValue(point->m_point.y);
				m_ui->ZP2->setValue(point->m_point.z);

				m_ui->XP2->blockSignals(false);
				m_ui->XP2->blockSignals(false);
				m_ui->XP2->blockSignals(false);
				break;
			}
			m_bpoints[m_point - 1] = true;
			m_points[m_point - 1] = point->m_point;
			if (m_bpoints.x && m_bpoints.y)
			{
				glm::vec2 prom(m_points[1] - m_points[0]);
				m_angle = atan2(prom.y, prom.x);
				if (std::isnan(m_angle))
				{
					m_angle = 0.0;
					if (m_point == 1)
						CleanP1();
					else
						CleanP2();
				}
				if (m_ui->pointsRadioButton->isChecked())
					m_dataDispatcher.sendControl(new control::function::clipping::SetAlignementValue(m_angle));
			}
			m_point = 0;
		}
		break;
		case guiDType::userOrientation:
		{
			auto userOrientation = static_cast<GuiDataSetUserOrientation*>(data);
			UserOrientation uo = userOrientation->m_userOrientation;
			m_userAngle = uo.getAngle();
			if (m_ui->userRadioButton->isChecked())
				m_dataDispatcher.sendControl(new control::function::clipping::SetAlignementValue(m_userAngle));
		}
		break;
		case guiDType::projectOrientation:
		{
			m_userAngle = 0.0;
			if (m_ui->userRadioButton->isChecked())
				m_dataDispatcher.sendControl(new control::function::clipping::SetAlignementValue(m_userAngle));
		}
		break;
	}
}

bool PropertyClippingSettings::sendSize()
{
	m_dataDispatcher.sendControl(new control::function::clipping::SetDefaultSize({m_xStored, m_yStored, m_zStored}));
	return true;
}

void PropertyClippingSettings::onXSizeEdit()
{
	GUI_LOG << "x edit" << LOGENDL;
	float xOffset = (float)m_ui->XInfield->getValue();
	if (m_xStored != xOffset) {
		m_xStored = xOffset;
		sendSize();
	}
}

void PropertyClippingSettings::onYSizeEdit()
{
	GUI_LOG << "y edit" << LOGENDL;
	float yOffset = (float)m_ui->YInfield->getValue();
	if (m_yStored != yOffset) {
		m_yStored = yOffset;
		sendSize();
	}
}

void PropertyClippingSettings::onZSizeEdit()
{
	GUI_LOG << "z edit" << LOGENDL;
	float zOffset = (float)m_ui->ZInfield->getValue();
	if (m_zStored != zOffset) {
		m_zStored = zOffset;
		sendSize();
	}
}

void PropertyClippingSettings::onCenterClick()
{
	GUI_LOG << "center click" << LOGENDL;
	m_ui->BottomFaceRadio->setChecked(false);
	m_ui->TopFaceRadio->setChecked(false);
	m_ui->CenterRadio->setChecked(true);
	m_dataDispatcher.sendControl(new control::function::clipping::SetDefaultOffset(ClippingBoxOffset::CenterOnPoint));
}

void PropertyClippingSettings::onBottomClick()
{
	GUI_LOG << "bot click" << LOGENDL;
	m_ui->CenterRadio->setChecked(false);
	m_ui->TopFaceRadio->setChecked(false);
	m_ui->BottomFaceRadio->setChecked(true);
	m_dataDispatcher.sendControl(new control::function::clipping::SetDefaultOffset(ClippingBoxOffset::BottomFace));
}

void PropertyClippingSettings::onTopClick()
{
	GUI_LOG << "top click" << LOGENDL;
	m_ui->BottomFaceRadio->setChecked(false);
	m_ui->CenterRadio->setChecked(false);
	m_ui->TopFaceRadio->setChecked(true);
	m_dataDispatcher.sendControl(new control::function::clipping::SetDefaultOffset(ClippingBoxOffset::Topface));
}

void PropertyClippingSettings::onProjectAxis()
{
	GUI_LOG << "onProjectAxis" << LOGENDL;
	m_ui->pointsRadioButton->setChecked(false);
	m_ui->userRadioButton->setChecked(false);
	m_dataDispatcher.sendControl(new control::function::clipping::SetAlignementValue(0.0));
}

void PropertyClippingSettings::onUserOrientation() {
	GUI_LOG << "onUserOrientation" << LOGENDL;
	m_ui->pointsRadioButton->setChecked(false);
	m_ui->projectAxisRadioButton->setChecked(false);
	m_dataDispatcher.sendControl(new control::function::clipping::SetAlignementValue(m_userAngle));
}

void PropertyClippingSettings::on2Points()
{
	GUI_LOG << "on2Points" << LOGENDL;
	m_ui->projectAxisRadioButton->setChecked(false);
	m_ui->userRadioButton->setChecked(false);
	m_dataDispatcher.sendControl(new control::function::clipping::SetAlignementValue(m_angle));
}

void PropertyClippingSettings::CleanP1()
{
	m_ui->XP1->blockSignals(true);
	m_ui->YP1->blockSignals(true);
	m_ui->ZP1->blockSignals(true);
	m_ui->XP1->setText("");
	m_ui->YP1->setText("");
	m_ui->ZP1->setText("");
	m_ui->XP1->blockSignals(false);
	m_ui->YP1->blockSignals(false);
	m_ui->ZP1->blockSignals(false);
}

void PropertyClippingSettings::CleanP2()
{
	m_ui->XP2->blockSignals(true);
	m_ui->YP2->blockSignals(true);
	m_ui->ZP2->blockSignals(true);
	m_ui->XP2->setText("");
	m_ui->YP2->setText("");
	m_ui->ZP2->setText("");
	m_ui->XP2->blockSignals(false);
	m_ui->YP2->blockSignals(false);
	m_ui->ZP2->blockSignals(false);
}

void PropertyClippingSettings::onPoint1Click()
{
	GUI_LOG << "Point 1 Click" << LOGENDL;
	m_point = 1;
	CleanP1();
	m_dataDispatcher.sendControl(new control::measure::ActivatePointMeasure());
	m_dataDispatcher.sendControl(new control::function::clipping::SetAlignementValue(0));
}

void PropertyClippingSettings::onPoint2Click()
{
	GUI_LOG << "Point 2 Click" << LOGENDL;
	m_point = 2;
	CleanP2();
	m_dataDispatcher.sendControl(new control::measure::ActivatePointMeasure());
	m_dataDispatcher.sendControl(new control::function::clipping::SetAlignementValue(0));
}