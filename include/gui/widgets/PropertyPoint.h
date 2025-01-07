#ifndef PROPERTY_POINT_H_
#define PROPERTY_POINT_H_

#include "gui/widgets/APropertyGeneral.h"
#include "ui_Property_Point.h"

class Controller;

class PointNode;

class PropertyPoint : public APropertyGeneral
{
	Q_OBJECT

public:
	explicit PropertyPoint(Controller& controller, QWidget* parent = nullptr, float guiScale = 1.f);
	~PropertyPoint();

	virtual bool actualizeProperty(SafePtr<AGraphNode> object );
private:

	void hideEvent(QHideEvent* event);
	void informData(IGuiData* keyValue);

	bool updatePoint();

	void changePosition();

public slots:
	void onXEdit();
	void onYEdit();
	void onZEdit();

	void addAsAnimationViewPoint();

private:
	Ui::property_Point m_ui;
	typedef void (PropertyPoint::* PropertyPointMethod)(IGuiData*);
	std::unordered_map<guiDType, PropertyPointMethod> m_measureMethods;

	SafePtr<PointNode> m_point;
};

#endif //PropertyPoint