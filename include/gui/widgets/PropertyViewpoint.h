#ifndef PROPERTY_VIEWPOINT_H_
#define PROPERTY_VIEWPOINT_H_

#include "gui/widgets/APropertyGeneral.h"
#include "ui_Property_Viewpoint.h"

class ViewPointNode;
class TransformationModule;
class Controller;

class PropertyViewpoint : public APropertyGeneral
{
	Q_OBJECT

public:
	explicit PropertyViewpoint(Controller& controller, QWidget* parent = nullptr, float guiScale = 1.f);
	~PropertyViewpoint();

	virtual bool actualizeProperty(SafePtr<AGraphNode> object );

private:
	void hideEvent(QHideEvent* event);
	void informData(IGuiData* keyValue);

	void setObject3DParameters(const TransformationModule& data);
	bool setViewpointParameters();

	void changeOrientation();
	void changeCenter();

public slots:
	void onOrientXEdit();
	void onOrientYEdit();
	void onOrientZEdit();
	void onCenterXEdit();
	void onCenterYEdit();
	void onCenterZEdit();
	void onUpdateToolButton();

private:
	Ui::PropertyViewpoint m_ui;
	typedef void (PropertyViewpoint::* PropertyViewpointMethod)(IGuiData*);
	std::unordered_map<guiDType, PropertyViewpointMethod> m_viewpointMethods;

	SafePtr<ViewPointNode> m_object;
	glm::dvec3 m_eulers;
};

#endif //PROPERTY_PC_OBJECT_H_