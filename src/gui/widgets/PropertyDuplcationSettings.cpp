#include "gui/widgets/PropertyDuplicationSettings.h"
#include "gui/GuiData/GuiDataMeasure.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "controller/controls/ControlSMeasureEdition.h"
#include "controller/controls/ControlMeasure.h"
#include "controller/controls/ControlDuplication.h"
#include "gui/widgets/FocusWatcher.h"
#include "models/3d/DuplicationTypes.h"

#include <cctype>
#include <glm/gtx/vector_angle.hpp>

PropertyDuplicationSettings::PropertyDuplicationSettings(IDataDispatcher* dataDispatcher, QWidget* parent, float guiScale)
	: QWidget(parent)
	, m_dataDispatcher(dataDispatcher)
{
	init();

	m_dataDispatcher->registerObserverOnKey(this, guiDType::renderValueDisplay);
}

PropertyDuplicationSettings::PropertyDuplicationSettings(QWidget* parent)
	: QWidget(parent)
{
	init();
}

PropertyDuplicationSettings::~PropertyDuplicationSettings()
{
	m_dataDispatcher->unregisterObserver(this);
}

void PropertyDuplicationSettings::init()
{
	m_ui.setupUi(this);

	QObject::connect(m_ui.duplicateClicked, SIGNAL(pressed()), this, SLOT(duplicationOnClick()));
	QObject::connect(m_ui.duplicateOffsetValue, SIGNAL(pressed()), this, SLOT(duplicationOnOffset()));
	QObject::connect(m_ui.duplicateSizeStep, SIGNAL(pressed()), this, SLOT(duplicationOnStepSize()));
	QObject::connect(m_ui.offsetValueGlobal, SIGNAL(pressed()), this, SLOT(onOffsetGlobal()));
	QObject::connect(m_ui.offsetValueLocal, SIGNAL(pressed()), this, SLOT(onOffsetLocal()));

	QObject::connect(m_ui.XOffsetfield, SIGNAL(editingFinished()), this, SLOT(onXOffsetEdit()));
	QObject::connect(m_ui.YOffsetfield, SIGNAL(editingFinished()), this, SLOT(onYOffsetEdit()));
	QObject::connect(m_ui.ZOffsetfield, SIGNAL(editingFinished()), this, SLOT(onZOffsetEdit()));
	QObject::connect(m_ui.stepSizeX, SIGNAL(pressed()), this, SLOT(onXStepSizeClick()));
	QObject::connect(m_ui.stepSizeY, SIGNAL(pressed()), this, SLOT(onYStepSizeClick()));
	QObject::connect(m_ui.stepSizeZ, SIGNAL(pressed()), this, SLOT(onZStepSizeClick()));
	QObject::connect(m_ui.stepSizeX_Neg, SIGNAL(pressed()), this, SLOT(onXNStepSizeClick()));
	QObject::connect(m_ui.stepSizeY_Neg, SIGNAL(pressed()), this, SLOT(onYNStepSizeClick()));
	QObject::connect(m_ui.stepSizeZ_Neg, SIGNAL(pressed()), this, SLOT(onZNStepSizeClick()));

	m_xOffsetStored = 1.f;
	m_yOffsetStored = 1.f;
	m_zOffsetStored = 1.f;

	updateUI();

}

void PropertyDuplicationSettings::initialise(IDataDispatcher* dataDispatcher, float guiScale)
{
	m_dataDispatcher = dataDispatcher;

	m_dataDispatcher->registerObserverOnKey(this, guiDType::renderValueDisplay);
}

void PropertyDuplicationSettings::informData(IGuiData* data)
{
}

void PropertyDuplicationSettings::hideEvent(QHideEvent* event)
{
	m_ui.XOffsetfield->blockSignals(true);
	m_ui.YOffsetfield->blockSignals(true);
	m_ui.ZOffsetfield->blockSignals(true);

	QWidget::hideEvent(event);

	m_ui.XOffsetfield->blockSignals(false);
	m_ui.YOffsetfield->blockSignals(false);
	m_ui.ZOffsetfield->blockSignals(false);
}

void PropertyDuplicationSettings::sendOffset()
{
	glm::vec3 vec;
	bool x, y, z;
	x = y = z = false;

	vec.x = m_ui.XOffsetfield->text().toFloat(&x);
	vec.y = m_ui.YOffsetfield->text().toFloat(&y);
	vec.z = m_ui.ZOffsetfield->text().toFloat(&z);
	if (!(x & y & z))
		return;

	m_dataDispatcher->sendControl(new control::duplication::SetDuplicationOffsetValue(glm::vec3(m_xOffsetStored, m_yOffsetStored, m_zOffsetStored)));
}

void PropertyDuplicationSettings::onXOffsetEdit()
{
	PANELLOG << "x edit" << LOGENDL;

	float xOffset = (float)m_ui.XOffsetfield->getValue();
	if (m_xOffsetStored != xOffset) {
		m_xOffsetStored = xOffset;
		sendOffset();
	}
}

void PropertyDuplicationSettings::onYOffsetEdit()
{
	PANELLOG << "y edit" << LOGENDL;
	float yOffset = (float)m_ui.YOffsetfield->getValue();
	if (m_yOffsetStored != yOffset) {
		m_yOffsetStored = yOffset;
		sendOffset();
	}
}

void PropertyDuplicationSettings::onZOffsetEdit()
{
	PANELLOG << "z edit" << LOGENDL;
	float zOffset = m_ui.ZOffsetfield->getValue();
	if (m_zOffsetStored != zOffset) {
		m_zOffsetStored = zOffset;
		sendOffset();
	}
}

void PropertyDuplicationSettings::duplicationOnClick()
{
	PANELLOG << "duplication click" << LOGENDL;
	m_ui.duplicateClicked->setChecked(true);
	m_ui.duplicateOffsetValue->setChecked(false);
	m_ui.duplicateSizeStep->setChecked(false);
	m_dataDispatcher->sendControl(new control::duplication::SetDuplicationMode(DuplicationMode::Click));
}

void PropertyDuplicationSettings::duplicationOnStepSize()
{
	PANELLOG << "duplication step" << LOGENDL;
	m_ui.duplicateClicked->setChecked(false);
	m_ui.duplicateOffsetValue->setChecked(false);
	m_ui.duplicateSizeStep->setChecked(true);
	m_dataDispatcher->sendControl(new control::duplication::SetDuplicationMode(DuplicationMode::SizeStep));
}

void PropertyDuplicationSettings::duplicationOnOffset()
{
	PANELLOG << "duplication offset" << LOGENDL;
	m_ui.duplicateClicked->setChecked(false);
	m_ui.duplicateOffsetValue->setChecked(true);
	m_ui.duplicateSizeStep->setChecked(false);
	m_dataDispatcher->sendControl(new control::duplication::SetDuplicationMode(DuplicationMode::Offset));
}

void PropertyDuplicationSettings::onOffsetLocal()
{
	PANELLOG << "duplication local" << LOGENDL;
	m_ui.offsetValueGlobal->setChecked(false);
	m_dataDispatcher->sendControl(new control::duplication::SetDuplicationIsLocal(true));
}

void PropertyDuplicationSettings::onOffsetGlobal()
{
	PANELLOG << "duplication global" << LOGENDL;
	m_ui.offsetValueLocal->setChecked(false);
	m_dataDispatcher->sendControl(new control::duplication::SetDuplicationIsLocal(false));
}

int PropertyDuplicationSettings::getStepX()
{
	return m_ui.stepSizeX->isChecked() ? 2 : m_ui.stepSizeX_Neg->isChecked() ? -2 : 0;
}

int PropertyDuplicationSettings::getStepY()
{
	return m_ui.stepSizeY->isChecked() ? 2 : m_ui.stepSizeY_Neg->isChecked() ? -2 : 0;
}

int PropertyDuplicationSettings::getStepZ()
{
	return m_ui.stepSizeZ->isChecked() ? 2 : m_ui.stepSizeZ_Neg->isChecked() ? -2 : 0;
}

void PropertyDuplicationSettings::updateUI()
{
	m_ui.XOffsetfield->setValue(m_xOffsetStored);
	m_ui.YOffsetfield->setValue(m_yOffsetStored);
	m_ui.ZOffsetfield->setValue(m_zOffsetStored);
}

void PropertyDuplicationSettings::onXStepSizeClick()
{
	PANELLOG << "duplication step X" << LOGENDL;
	m_ui.stepSizeX_Neg->setChecked(false);
	m_dataDispatcher->sendControl(new control::duplication::SetDuplicationStepSize(glm::ivec3((!m_ui.stepSizeX->isChecked()) ? 2 : 0, getStepY(), getStepZ())));
}

void PropertyDuplicationSettings::onYStepSizeClick()
{
	PANELLOG << "duplication step Y" << LOGENDL;
	m_ui.stepSizeY_Neg->setChecked(false);
	m_dataDispatcher->sendControl(new control::duplication::SetDuplicationStepSize(glm::ivec3(getStepX(), (!m_ui.stepSizeY->isChecked()) ? 2 : 0, getStepZ())));
}

void PropertyDuplicationSettings::onZStepSizeClick()
{
	PANELLOG << "duplication step Z" << LOGENDL;
	m_ui.stepSizeZ_Neg->setChecked(false);
	m_dataDispatcher->sendControl(new control::duplication::SetDuplicationStepSize(glm::ivec3(getStepX(), getStepY(), (!m_ui.stepSizeZ->isChecked()) ? 2 : 0)));
}

void PropertyDuplicationSettings::onXNStepSizeClick()
{
	PANELLOG << "duplication step X" << LOGENDL;
	m_ui.stepSizeX->setChecked(false);
	m_dataDispatcher->sendControl(new control::duplication::SetDuplicationStepSize(glm::ivec3((!m_ui.stepSizeX_Neg->isChecked()) ? -2 : 0, getStepY(), getStepZ())));
}

void PropertyDuplicationSettings::onYNStepSizeClick()
{
	PANELLOG << "duplication step Y" << LOGENDL;
	m_ui.stepSizeY->setChecked(false);
	m_dataDispatcher->sendControl(new control::duplication::SetDuplicationStepSize(glm::ivec3(getStepX(), (!m_ui.stepSizeY_Neg->isChecked()) ? -2 : 0, getStepZ())));
}

void PropertyDuplicationSettings::onZNStepSizeClick()
{
	PANELLOG << "duplication step Z" << LOGENDL;
	m_ui.stepSizeZ->setChecked(false);
	m_dataDispatcher->sendControl(new control::duplication::SetDuplicationStepSize(glm::ivec3(getStepX(), getStepY(), (!m_ui.stepSizeZ_Neg->isChecked()) ? -2 : 0)));
}