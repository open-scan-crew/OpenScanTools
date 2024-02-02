#ifndef PROPERTY_SIMPLEMEASURE_H_
#define PROPERTY_SIMPLEMEASURE_H_

#include "gui/widgets/APropertyGeneral.h"
#include "ui_Property_SimpleMeasure.h"

class Controller;

class SimpleMeasureNode;

class PropertySimpleMeasure;

typedef void (PropertySimpleMeasure::* PropertySimpleMethod)(IGuiData*);

class PropertySimpleMeasure : public APropertyGeneral
{
	Q_OBJECT

public:
	explicit PropertySimpleMeasure(Controller &controller, QWidget *parent = nullptr, float guiScale = 1.f);
	~PropertySimpleMeasure();

	virtual bool actualizeProperty(SafePtr<AGraphNode> object );
private:
	void informData(IGuiData *keyValue);

	bool updateMeasure();

private:
	Ui::property_SimpleMeasure m_ui;
	std::unordered_map<guiDType, PropertySimpleMethod> m_simpleMethods;

	SafePtr<SimpleMeasureNode> m_measure;
};

#endif //PROPERTY_SIMPLEMEASURE_H_