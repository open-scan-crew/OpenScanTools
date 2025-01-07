#ifndef PROPERTY_SPHERE_H_
#define PROPERTY_SPHERE_H_

#include "gui/widgets/APropertyGeneral.h"
#include "ui_Property_Sphere.h"

class TransformationModule;
class SphereNode;
class Controller;

class PropertySphere : public APropertyGeneral
{
	Q_OBJECT

public:
	explicit PropertySphere(Controller& controller, QWidget* parent = nullptr, float guiScale = 1.f);
	~PropertySphere();

	virtual bool actualizeProperty(SafePtr<AGraphNode> object );

private:
	void informData(IGuiData* keyValue);

	bool updateSphere();
	void setObject3DParameters(const TransformationModule& data);

	void updateStandard();


public slots:
	void onDiameterChange();

private:
	Ui::PropertySphere m_ui;
	typedef void (PropertySphere::* PropertySphereMethod)(IGuiData*);
	std::unordered_map<guiDType, PropertySphereMethod> m_sphereMethods;

	SafePtr<SphereNode> m_sphere;
};

#endif //PROPERTY_PIPE_H_