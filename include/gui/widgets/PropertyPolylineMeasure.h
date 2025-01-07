#ifndef PROPERTY_POLYLINeMEASURE_H_
#define PROPERTY_POLYLINeMEASURE_H_

#include "ui_Property_Measure_Polyline.h"
#include "gui/widgets/APropertyGeneral.h"

class Controller;

class PolylineMeasureNode;

class PropertyPolylineMeasure : public APropertyGeneral
{
	Q_OBJECT

public:
	explicit PropertyPolylineMeasure(Controller& controller, QWidget* parent = nullptr, float guiScale = 1.f);
	~PropertyPolylineMeasure();

	void resizeEvent(QResizeEvent* event);

	virtual bool actualizeProperty(SafePtr<AGraphNode> object );
private:
	void informData(IGuiData* keyValue);

	void onRenderUnitUsage(IGuiData* data);

	bool updateMeasure();
	void updateUI();

private:
	Ui::property_PolylineMeasure m_ui;
	typedef void (PropertyPolylineMeasure::* PropertyPolylineMethod)(IGuiData*);
	std::unordered_map<guiDType, PropertyPolylineMethod> m_polylineMethods;
	UnitUsage m_unitUsage;

	SafePtr<PolylineMeasureNode> m_measure;

};

#endif //PROPERTY_POLYLINeMEASURE_H_