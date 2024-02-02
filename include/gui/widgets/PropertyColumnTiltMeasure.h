#ifndef PROPERTY_COLUMNTILTMEASURE_H_
#define PROPERTY_COLUMNTILTMEASURE_H_

#include "gui/widgets/APropertyGeneral.h"
#include "ui_Property_Measure_ColumnTilt.h"


class Controller;

class ColumnTiltMeasureNode;

class PropertyColumnTiltMeasure;

typedef void (PropertyColumnTiltMeasure::* PropertyColumnTiltMethod)(IGuiData*);

class PropertyColumnTiltMeasure : public APropertyGeneral
{
	Q_OBJECT

public:
	explicit PropertyColumnTiltMeasure(Controller& controller, QWidget *parent = nullptr, float guiScale = 1.f);
	~PropertyColumnTiltMeasure();

	virtual bool actualizeProperty(SafePtr<AGraphNode> object );

private:
	void informData(IGuiData *keyValue);

	void onMeasureData(IGuiData *data);
	bool updateColumnTilt();

private:
	Ui::property_ColumnTiltMeasure m_ui;
	std::unordered_map<guiDType, PropertyColumnTiltMethod> m_columnTiltMethods;

	SafePtr<ColumnTiltMeasureNode> m_measure;
};

#endif //PROPERTY_COLUMNTILTMEASURE_H_