#ifndef PROPERTY_POINTTOPIPEMEASURE_H_
#define PROPERTY_POINTTOPIPEMEASURE_H_

#include "gui/widgets/APropertyGeneral.h"
#include "ui_Property_Measure_PointToPipe.h"

class Controller;
class PointToPipeMeasureNode;

class PropertyPointToPipeMeasure;

typedef void (PropertyPointToPipeMeasure::* PropertyPointToPipeMethod)(IGuiData*);

class PropertyPointToPipeMeasure : public APropertyGeneral
{
	Q_OBJECT

public:
	explicit PropertyPointToPipeMeasure(Controller& controller, QWidget *parent = nullptr, float guiScale = 1.f);
	~PropertyPointToPipeMeasure();

	virtual bool actualizeProperty(SafePtr<AGraphNode> object );

private:
	void informData(IGuiData *keyValue);

	bool updateMeasure();


private:
	Ui::property_PointToPipeMeasure m_ui;
	std::unordered_map<guiDType, PropertyPointToPipeMethod> m_measureMethods;

	SafePtr<PointToPipeMeasureNode> m_measure;
};

#endif //PROPERTY_POINTTOPIPEMEASURE_H_