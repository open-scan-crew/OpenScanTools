#include "gui/widgets/PropertyBeamBendingMeasure.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "controller/controls/ControlDataEdition.h"
#include "models/graph/BeamBendingMeasureNode.h"
#include "gui/widgets/FocusWatcher.h"
#include "gui/Texts.hpp"

#include "controller/Controller.h"

PropertyBeamBendingMeasure::PropertyBeamBendingMeasure(Controller& controller, QWidget *parent, float guiScale)
	: APropertyGeneral(controller.getDataDispatcher(), parent)
{
	m_ui.setupUi(this);
	m_ui.genericPropsHeadWidget->setControllerInfo(controller);
	m_ui.genericPropsFeetWidget->setDataDispatcher(controller.getDataDispatcher());

	m_dataDispatcher.registerObserverOnKey(this, guiDType::renderValueDisplay);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::sendAuthorsList);

	m_ui.BendingValueInfield->setType(NumericType::DIAMETER);

	m_ui.XP1Infield->setType(NumericType::DISTANCE);
	m_ui.YP1Infield->setType(NumericType::DISTANCE);
	m_ui.ZP1Infield->setType(NumericType::DISTANCE);

	m_ui.XP2Infield->setType(NumericType::DISTANCE);
	m_ui.YP2Infield->setType(NumericType::DISTANCE);
	m_ui.ZP2Infield->setType(NumericType::DISTANCE);

	m_ui.XMBInfield->setType(NumericType::DISTANCE);
	m_ui.YMBInfield->setType(NumericType::DISTANCE);
	m_ui.ZMBInfield->setType(NumericType::DISTANCE);

	m_ui.LengthInfield->setType(NumericType::DISTANCE);
}

PropertyBeamBendingMeasure::~PropertyBeamBendingMeasure()
{
	m_dataDispatcher.unregisterObserver(this);
}

void PropertyBeamBendingMeasure::informData(IGuiData *data)
{
	APropertyGeneral::informData(data);
	if (m_methods.find(data->getType()) != m_methods.end())
	{
		PropertyBeamBendingMethod method = m_methods.at(data->getType());
		(this->*method)(data);
	}
}

bool PropertyBeamBendingMeasure::updateMeasure()
{
	ReadPtr<BeamBendingMeasureNode> bm = m_measure.cget();
	if (!bm)
		return false;

	m_ui.BendingValueInfield->setValue(bm->getBendingValue());

	m_ui.LengthInfield->setValue(bm->getLength());

	m_ui.RatioInfield->setValue(bm->getRatio());
	m_ui.ValueMaxRatioInfield->setValue(bm->getMaxRatio());

	m_ui.OverRatioInfield->setText(bm->getRatioSup() == RatioSup::NA ? TEXT_NA : bm->getRatioSup() == RatioSup::yes ? TEXT_RATIOSUP_YES : TEXT_RATIOSUP_NO);
	m_ui.ResultInfield->setText(bm->getResult() == Reliability::NA ? TEXT_NA : bm->getResult() == Reliability::reliable ? TEXT_RELIABILITY_RELIABLE : TEXT_RELIABILITY_UNRELIABLE);

	m_ui.XP1Infield->setValue(bm->getPoint1Pos().x);
	m_ui.YP1Infield->setValue(bm->getPoint1Pos().y);
	m_ui.ZP1Infield->setValue(bm->getPoint1Pos().z);

	m_ui.XP2Infield->setValue(bm->getPoint2Pos().x);
	m_ui.YP2Infield->setValue(bm->getPoint2Pos().y);
	m_ui.ZP2Infield->setValue(bm->getPoint2Pos().z);

	m_ui.XMBInfield->setValue(bm->getMaxBendingPos().x);
	m_ui.YMBInfield->setValue(bm->getMaxBendingPos().y);
	m_ui.ZMBInfield->setValue(bm->getMaxBendingPos().z);

	return true;
}

bool PropertyBeamBendingMeasure::actualizeProperty(SafePtr<AGraphNode> object)
{
	if (object)
		m_measure = static_pointer_cast<BeamBendingMeasureNode>(object);

	m_ui.genericPropsHeadWidget->setObject(m_measure);
	m_ui.genericPropsFeetWidget->setObject(m_measure);

	return updateMeasure();
}