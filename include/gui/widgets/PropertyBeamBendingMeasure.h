#ifndef PROPERTY_BEAMBENDINGMEASURE_H_
#define PROPERTY_BEAMBENDINGMEASURE_H_

#include "gui/widgets/APropertyGeneral.h"
#include "ui_Property_Measure_BeamBending.h"

class Controller;

class BeamBendingMeasureNode;

class PropertyBeamBendingMeasure : public APropertyGeneral
{
	Q_OBJECT

public:
	explicit PropertyBeamBendingMeasure(Controller& controller, QWidget *parent = nullptr, float guiScale = 1.f);
	~PropertyBeamBendingMeasure();

	virtual bool actualizeProperty(SafePtr<AGraphNode> object );

private:
	void informData(IGuiData *keyValue);

	bool updateMeasure();

private:
	Ui::property_BeamBendingMeasure m_ui;
	typedef void (PropertyBeamBendingMeasure::* PropertyBeamBendingMethod)(IGuiData*);
	std::unordered_map<guiDType, PropertyBeamBendingMethod> m_methods;

	SafePtr<BeamBendingMeasureNode> m_measure;
};

#endif //PROPERTY_BEAMBENDINGMEASURE_H_