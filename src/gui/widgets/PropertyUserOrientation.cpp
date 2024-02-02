#include "gui/widgets/PropertyUserOrientation.h"

#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "gui/GuiData/GuiDataMeasure.h"
#include "gui/GuiData/GuiDataUserOrientation.h"

#include "controller/controls/ControlUserOrientation.h"
#include "controller/controls/ControlMeasure.h"
#include "gui/Texts.hpp"

#include <cctype>
#include <glm/gtx/vector_angle.hpp>
#include "utils/math/trigo.h"

#include <QMessageBox>
#include <QtGui/QHideEvent>

PropertyUserOrientation::PropertyUserOrientation(IDataDispatcher& dataDispatcher, QWidget* parent, float guiScale)
	: APropertyGeneral(dataDispatcher, parent)
	, m_currentEditPoint(nullptr)
	, m_empty(true)
	, m_oldPoint(0.)
	, m_newPoint(0.)
	, m_axisPoints({ glm::dvec3(0.,0.,0.), glm::dvec3(1.,0.,0.) })
	, m_customAxis({ glm::dvec3(0.,0.,0.), glm::dvec3(1.,0.,0.) })
{
	m_ui.setupUi(this);

	m_ui.customAxisFrame->hide();
	m_ui.setTranslationGroupBox->hide();

	QObject::connect(m_ui.p1Button, &QPushButton::released, this, &PropertyUserOrientation::onPoint1Click);
	QObject::connect(m_ui.p2Button, &QPushButton::released, this, &PropertyUserOrientation::onPoint2Click);

	QObject::connect(m_ui.setCoorPickPointXButton, &QPushButton::released, this, &PropertyUserOrientation::onPointXClick);
	QObject::connect(m_ui.setCoorPickPointYButton, &QPushButton::released, this, &PropertyUserOrientation::onPointYClick);
	QObject::connect(m_ui.setCoorPickPointZButton, &QPushButton::released, this, &PropertyUserOrientation::onPointZClick);

	QObject::connect(m_ui.XRadioButton, &QRadioButton::released, this, &PropertyUserOrientation::updateCustomAxisFrame);
	QObject::connect(m_ui.YRadioButton, &QRadioButton::released, this, &PropertyUserOrientation::updateCustomAxisFrame);
	QObject::connect(m_ui.customAxisRadioButton, &QRadioButton::released, this, &PropertyUserOrientation::updateCustomAxisFrame);

	QObject::connect(m_ui.setCoorNewXValue, &QLineEdit::editingFinished, this, &PropertyUserOrientation::onNewPointEdit);
	QObject::connect(m_ui.setCoorNewYValue, &QLineEdit::editingFinished, this, &PropertyUserOrientation::onNewPointEdit);
	QObject::connect(m_ui.setCoorNewZValue, &QLineEdit::editingFinished, this, &PropertyUserOrientation::onNewPointEdit);

	QObject::connect(m_ui.customAxisX1, &QLineEdit::editingFinished, this, &PropertyUserOrientation::onCustomAxisUpdate);
	QObject::connect(m_ui.customAxisY1, &QLineEdit::editingFinished, this, &PropertyUserOrientation::onCustomAxisUpdate);
	QObject::connect(m_ui.customAxisX2, &QLineEdit::editingFinished, this, &PropertyUserOrientation::onCustomAxisUpdate);
	QObject::connect(m_ui.customAxisY2, &QLineEdit::editingFinished, this, &PropertyUserOrientation::onCustomAxisUpdate);

	QObject::connect(m_ui.okButton, &QPushButton::released, this, &PropertyUserOrientation::onOkButton);
	QObject::connect(m_ui.resetButton, &QPushButton::released, this, &PropertyUserOrientation::onResetButton);
	QObject::connect(m_ui.deleteButton, &QPushButton::released, this, &PropertyUserOrientation::onDeleteButton);
	QObject::connect(m_ui.cancelButton, &QPushButton::released, this, &PropertyUserOrientation::Close);

	m_dataDispatcher.registerObserverOnKey(this, guiDType::point);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::abortEvent);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::userOrientationProperties);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::closeUOProperties);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::renderValueDisplay);

	m_UOmethods.insert({ guiDType::point, &PropertyUserOrientation::onPointReceived });
	m_UOmethods.insert({ guiDType::abortEvent, &PropertyUserOrientation::onAbort });
	m_UOmethods.insert({ guiDType::userOrientationProperties, &PropertyUserOrientation::onUserOrientation });
	m_UOmethods.insert({ guiDType::closeUOProperties, &PropertyUserOrientation::onClose });

	m_ui.x_p1Edit->setType(NumericType::DISTANCE);
	m_ui.y_p1Edit->setType(NumericType::DISTANCE);
	m_ui.z_p1Edit->setType(NumericType::DISTANCE);

	m_ui.x_p2Edit->setType(NumericType::DISTANCE);
	m_ui.y_p2Edit->setType(NumericType::DISTANCE);
	m_ui.z_p2Edit->setType(NumericType::DISTANCE);

	m_ui.setCoorOldXValue->setType(NumericType::DISTANCE);
	m_ui.setCoorOldYValue->setType(NumericType::DISTANCE);
	m_ui.setCoorOldZValue->setType(NumericType::DISTANCE);

	m_ui.setCoorNewXValue->setType(NumericType::DISTANCE);
	m_ui.setCoorNewYValue->setType(NumericType::DISTANCE);
	m_ui.setCoorNewZValue->setType(NumericType::DISTANCE);

	adjustSize();
}

PropertyUserOrientation::~PropertyUserOrientation()
{
	PANELLOG << "destructor UserOrientations" << LOGENDL;
	m_dataDispatcher.unregisterObserver(this);
}

bool PropertyUserOrientation::actualizeProperty(SafePtr<AGraphNode> object)
{ 
	return true;
}

void PropertyUserOrientation::hideEvent(QHideEvent* event)
{
	m_ui.nameEdit->blockSignals(true);
	m_ui.XRadioButton->blockSignals(true);
	m_ui.YRadioButton->blockSignals(true);
	m_ui.x_p1Edit->blockSignals(true);
	m_ui.y_p1Edit->blockSignals(true);
	m_ui.z_p1Edit->blockSignals(true);
	m_ui.x_p2Edit->blockSignals(true);
	m_ui.y_p2Edit->blockSignals(true);
	m_ui.z_p2Edit->blockSignals(true);

	QWidget::hideEvent(event);

	m_ui.x_p1Edit->blockSignals(false);
	m_ui.y_p1Edit->blockSignals(false);
	m_ui.z_p1Edit->blockSignals(false);
	m_ui.x_p2Edit->blockSignals(false);
	m_ui.y_p2Edit->blockSignals(false);
	m_ui.z_p2Edit->blockSignals(false);
	m_ui.XRadioButton->blockSignals(false);
	m_ui.YRadioButton->blockSignals(false);
	m_ui.nameEdit->blockSignals(false);
}

void PropertyUserOrientation::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
}

void PropertyUserOrientation::informData(IGuiData *data)
{ 
	APropertyGeneral::informData(data);
	if (m_UOmethods.find(data->getType()) != m_UOmethods.end())
	{
		PropertyUserOrientationMethod method = m_UOmethods.at(data->getType());
		(this->*method)(data);
	}
}

void PropertyUserOrientation::updateUO()
{
	m_ui.x_p1Edit->setValue(m_axisPoints[0].x);
	m_ui.y_p1Edit->setValue(m_axisPoints[0].y);
	m_ui.z_p1Edit->setValue(m_axisPoints[0].z);

	m_ui.x_p2Edit->setValue(m_axisPoints[1].x);
	m_ui.y_p2Edit->setValue(m_axisPoints[1].y);
	m_ui.z_p2Edit->setValue(m_axisPoints[1].z);

	m_ui.setCoorOldXValue->setValue(m_oldPoint.x);
	m_ui.setCoorOldYValue->setValue(m_oldPoint.y);
	m_ui.setCoorOldZValue->setValue(m_oldPoint.z);

	m_ui.setCoorNewXValue->setValue(m_newPoint.x);
	m_ui.setCoorNewYValue->setValue(m_newPoint.y);
	m_ui.setCoorNewZValue->setValue(m_newPoint.z);

	m_ui.customAxisX1->setValue(m_customAxis[0].x);
	m_ui.customAxisY1->setValue(m_customAxis[0].y);
	m_ui.customAxisX2->setValue(m_customAxis[1].x);
	m_ui.customAxisY2->setValue(m_customAxis[1].y);
}

void PropertyUserOrientation::updateUI()
{
	updateUO();
}

void PropertyUserOrientation::Clean()
{
	m_ui.nameEdit->blockSignals(true);
	m_ui.XRadioButton->blockSignals(true);
	m_ui.YRadioButton->blockSignals(true);

	m_ui.nameEdit->setText("");
	m_ui.XRadioButton->setChecked(true);
	m_ui.YRadioButton->setChecked(false);
	m_ui.customAxisRadioButton->setChecked(false);
	updateCustomAxisFrame();

	m_axisPoints[0] = glm::dvec3();
	m_axisPoints[1] = glm::dvec3();

	m_customAxis[0] = glm::dvec3();
	m_customAxis[1] = glm::dvec3();

	m_newPoint = glm::dvec3();
	m_oldPoint = glm::dvec3();

	updateUO();

	m_ui.nameEdit->blockSignals(false);
	m_ui.XRadioButton->blockSignals(false);
	m_ui.YRadioButton->blockSignals(false);

}

void PropertyUserOrientation::updateCustomAxisFrame()
{
	if (m_ui.customAxisRadioButton->isChecked())
		m_ui.customAxisFrame->show();
	else
		m_ui.customAxisFrame->hide();
}

void PropertyUserOrientation::onPoint1Click()
{
	PANELLOG << "UserOrientation : Point 1 Click" << LOGENDL;
	m_currentEditPoint = &m_axisPoints[0];
	m_axisPoints[0] = glm::dvec3();
	updateUO();
	m_dataDispatcher.sendControl(new control::measure::ActivatePointMeasure());
}

void PropertyUserOrientation::onPoint2Click()
{
	PANELLOG << "UserOrientation : Point 2 Click" << LOGENDL;
	m_currentEditPoint = &m_axisPoints[1];
	m_axisPoints[1] = glm::dvec3();
	updateUO();
	m_dataDispatcher.sendControl(new control::measure::ActivatePointMeasure());
}

void PropertyUserOrientation::onCustomAxisUpdate()
{
	m_customAxis = { glm::dvec3(m_ui.customAxisX1->getValue(), m_ui.customAxisY1->getValue(), 0.),
					glm::dvec3(m_ui.customAxisX2->getValue(), m_ui.customAxisY2->getValue(), 0.) };
	updateUO();
}

void PropertyUserOrientation::onPointXClick()
{
	m_currentEditPoint = &m_oldPoint;
	m_oldPoint.x = NAN;
	m_dataDispatcher.sendControl(new control::measure::ActivatePointMeasure());
}

void PropertyUserOrientation::onPointYClick()
{
	m_currentEditPoint = &m_oldPoint;
	m_oldPoint.y = NAN;
	m_dataDispatcher.sendControl(new control::measure::ActivatePointMeasure());
}

void PropertyUserOrientation::onPointZClick()
{
	m_currentEditPoint = &m_oldPoint;
	m_oldPoint.z = NAN;
	m_dataDispatcher.sendControl(new control::measure::ActivatePointMeasure());
}

void PropertyUserOrientation::onNewPointEdit()
{

	m_newPoint = { m_ui.setCoorNewXValue->getValue(), m_ui.setCoorNewYValue->getValue(), m_ui.setCoorNewZValue->getValue() };
	updateUO();
}

void PropertyUserOrientation::onAbort(IGuiData* data)
{
	m_currentEditPoint = nullptr;
	return;
}

void PropertyUserOrientation::onPointReceived(IGuiData* data)
{
	if (m_currentEditPoint == nullptr)
		return;

	GuiDataPoint* point = static_cast<GuiDataPoint*>(data);

	if (std::isnan((*m_currentEditPoint).x))
		(*m_currentEditPoint).x = point->m_point.x;
	else if (std::isnan((*m_currentEditPoint).y))
		(*m_currentEditPoint).y = point->m_point.y;
	else if (std::isnan((*m_currentEditPoint).z))
		(*m_currentEditPoint).z = point->m_point.z;
	else
		(*m_currentEditPoint) = point->m_point;

	updateUO();
}

void PropertyUserOrientation::onUserOrientation(IGuiData* data)
{
	GuiDataUserOrientationProperties* userOrientation = static_cast<GuiDataUserOrientationProperties*>(data);
	m_empty = userOrientation->m_empty;
	if (!m_empty)
		m_uo = UserOrientation(userOrientation->m_userOrientation);
	else
		m_uo = UserOrientation();

	onResetButton();
}

void PropertyUserOrientation::onClose(IGuiData * data)
{
	Close();
}


void PropertyUserOrientation::onOkButton()
{
	if (m_ui.x_p1Edit->text() == "" || m_ui.x_p2Edit->text() == "" 
		|| m_ui.y_p1Edit->text() == "" || m_ui.y_p2Edit->text() == ""
		|| m_ui.z_p1Edit->text() == "" || m_ui.z_p2Edit->text() == "") {
		QMessageBox missingPoint(QMessageBox::Icon::Warning, "", TEXT_USER_ORIENTATION_PROPERTIES_WARNING_MISSING_POINT);
		missingPoint.exec();
		return;
	}
	if (m_axisPoints[0].x == m_axisPoints[1].x && m_axisPoints[0].y == m_axisPoints[1].y)
	{
		QMessageBox samePoint(QMessageBox::Icon::Warning, "", TEXT_USER_ORIENTATION_PROPERTIES_SAME_POINT_AXIS.arg(tr("axis")));
		samePoint.exec();
		return;
	}

	if (glm::any(glm::isnan(m_oldPoint)))
	{
		QMessageBox nanVec(QMessageBox::Icon::Warning, "", TEXT_USER_ORIENTATION_PROPERTIES_NAN_VEC.arg("\"current values\""));
		nanVec.exec();
		return;
	}


	m_uo.setName(m_ui.nameEdit->text());
	if (m_ui.YRadioButton->isChecked())
	{
		m_uo.setAxisType(UOAxisType::YAxis);
		m_uo.setCustomAxis({ glm::dvec3({0., 0., 0.}), glm::dvec3({0., 1., 0.}) });
	}
	else if (m_ui.XRadioButton->isChecked())
	{
		m_uo.setAxisType(UOAxisType::XAxis);
		m_uo.setCustomAxis({ glm::dvec3({0., 0., 0.}), glm::dvec3({1., 0., 0.}) });
	}
	else
	{
		m_uo.setAxisType(UOAxisType::Custom);
		if (m_ui.customAxisX1->text() == "" || m_ui.customAxisY1->text() == ""
			|| m_ui.customAxisX2->text() == "" || m_ui.customAxisY2->text() == "")
		{
			QMessageBox missingPoint(QMessageBox::Icon::Warning, "", TEXT_USER_ORIENTATION_PROPERTIES_WARNING_MISSING_POINT);
			missingPoint.exec();
			return;
		}
		std::array<glm::dvec3, 2> customAxis;
		customAxis = { glm::dvec3(m_ui.customAxisX1->getValue(), m_ui.customAxisY1->getValue(), 0.),
						glm::dvec3(m_ui.customAxisX2->getValue(), m_ui.customAxisY2->getValue(), 0.) };
		if (customAxis[0] == customAxis[1])
		{
			QMessageBox samePoint(QMessageBox::Icon::Warning, "", TEXT_USER_ORIENTATION_PROPERTIES_SAME_POINT_AXIS.arg(tr("custom axis")));
			samePoint.exec();
			return;
		}
		m_uo.setCustomAxis(customAxis);
	}

	m_uo.setPoint1(m_axisPoints[0]);
	m_uo.setPoint2(m_axisPoints[1]);

	m_uo.setOldPoint(m_oldPoint);
	m_uo.setNewPoint(m_newPoint);

	m_dataDispatcher.sendControl(new control::userOrientation::CreateEditUserOrientation(m_uo));
	
	Close();
}

void PropertyUserOrientation::onResetButton()
{
	if (m_empty)
		Clean();
	else {
		 
		m_ui.nameEdit->setText(m_uo.getName());

		switch (m_uo.getAxisType())
		{
			case UOAxisType::XAxis:
			{
				m_ui.XRadioButton->setChecked(true);
				break;
			}
			case UOAxisType::YAxis:
			{
				m_ui.YRadioButton->setChecked(true);
				break;
			}
			case UOAxisType::Custom:
			{
				m_ui.customAxisRadioButton->setChecked(true);
				break;
			}
		}
		m_axisPoints = m_uo.getAxisPoints();
		m_customAxis = m_uo.getCustomAxis();


		m_newPoint = m_uo.getNewPoint();
		m_oldPoint = m_uo.getOldPoint();

		updateCustomAxisFrame();
		updateUO();
	}
}

void PropertyUserOrientation::onDeleteButton()
{
	if(!m_empty)
		m_dataDispatcher.sendControl(new control::userOrientation::DeleteUserOrientation(m_uo.getId()));

	Close();
}

void PropertyUserOrientation::Close()
{
	Clean();
	m_dataDispatcher.updateInformation(new GuiDataHidePropertyPanels());
}
