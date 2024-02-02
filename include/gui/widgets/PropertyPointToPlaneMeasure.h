#ifndef PROPERTY_POINTTOPLANMEASURE_H_
#define PROPERTY_POINTTOPLANMEASURE_H_

#include "gui/widgets/APropertyGeneral.h"
#include "ui_Property_Measure_PointToPlane.h"

class Controller;

class PointToPlaneMeasureNode;

class PropertyPointToPlanMeasure;

typedef void (PropertyPointToPlanMeasure::* PropertyPointToPlanMethod)(IGuiData*);

class PropertyPointToPlanMeasure : public APropertyGeneral
{
	Q_OBJECT

public:
	explicit PropertyPointToPlanMeasure(Controller& controller, QWidget *parent = nullptr, float guiScale = 1.f);
	~PropertyPointToPlanMeasure();

	virtual bool actualizeProperty(SafePtr<AGraphNode> object );
private:
	void informData(IGuiData *keyValue);

	bool updateMeasure();

private:
	Ui::property_PointToPlanMeasure m_ui;
	std::unordered_map<guiDType, PropertyPointToPlanMethod> m_measureMethods;

	SafePtr<PointToPlaneMeasureNode> m_measure;
};

#endif //PROPERTY_POINTTOPLANMEASURE_H_