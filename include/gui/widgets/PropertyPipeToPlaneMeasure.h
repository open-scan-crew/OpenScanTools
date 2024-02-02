#ifndef PROPERTY_PIPETOPLANEMEASURE_H_
#define PROPERTY_PIPETOPLANEMEASURE_H_

#include "gui/widgets/APropertyGeneral.h"
#include "ui_Property_Measure_PipeToPlane.h"

class Controller;

class PipeToPlaneMeasureNode;

class PropertyPipeToPlaneMeasure;

typedef void (PropertyPipeToPlaneMeasure::* PropertyPipeToPlaneMethod)(IGuiData*);

class PropertyPipeToPlaneMeasure : public APropertyGeneral
{
	Q_OBJECT

public:
	explicit PropertyPipeToPlaneMeasure(Controller& controller, QWidget *parent = nullptr, float guiScale = 1.f);
	~PropertyPipeToPlaneMeasure();

	virtual bool actualizeProperty(SafePtr<AGraphNode> object );

private:
	void informData(IGuiData *keyValue);

	bool updateMeasure();

private:
	Ui::property_PipeToPlaneMeasure m_ui;
	std::unordered_map<guiDType, PropertyPipeToPlaneMethod> m_measureMethods;

	SafePtr<PipeToPlaneMeasureNode> m_measure;
};

#endif //PROPERTY_PIPETOPLANEMEASURE_H_