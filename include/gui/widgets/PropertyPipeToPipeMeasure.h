#ifndef PROPERTY_PIPETOPIPEMEASURE_H_
#define PROPERTY_PIPETOPIPEMEASURE_H_

#include "gui/widgets/APropertyGeneral.h"
#include "ui_Property_Measure_PipeToPipe.h"

#include <utility>
#include <vector>

class Controller;
class PipeToPipeMeasureNode;

class PropertyPipeToPipeMeasure;

typedef void (PropertyPipeToPipeMeasure::* PropertyPipeToPipeMethod)(IGuiData*);

class PropertyPipeToPipeMeasure : public APropertyGeneral
{
	Q_OBJECT

public:
	explicit PropertyPipeToPipeMeasure(Controller& controller, QWidget *parent = nullptr, float guiScale = 1.f);
	~PropertyPipeToPipeMeasure();
	virtual bool actualizeProperty(SafePtr<AGraphNode> object );

private:
	void informData(IGuiData *keyValue);

	void onMeasureData(IGuiData *data);
	bool updateMeasure();

private:
	Ui::property_PipeToPipeMeasure m_ui;
	std::unordered_map<guiDType, PropertyPipeToPipeMethod> m_measureMethods;

	SafePtr<PipeToPipeMeasureNode> m_measure;
};

#endif //PROPERTY_PIPETOPIPEMEASURE_H_