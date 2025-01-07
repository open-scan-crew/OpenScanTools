#ifndef PROPERTY_WAVEFRONT_H_
#define PROPERTY_WAVEFRONT_H_

#include "gui/widgets/APropertyGeneral.h"
#include "ui_Property_MeshObject.h"

class Controller;

class MeshObjectNode;

enum class ManipulationMode;

class PropertyMeshObject : public APropertyGeneral
{
	Q_OBJECT

public:
	explicit PropertyMeshObject(Controller& controller, QWidget* parent = nullptr, float guiScale = 1.f);
	~PropertyMeshObject();

	void hideEvent(QHideEvent* event);
	virtual bool actualizeProperty(SafePtr<AGraphNode> object );

private:
	void informData(IGuiData* keyValue);

	bool updateMeshObject();

	void changeOrientation();
	void changeCenter();

public slots:
	void onOrientXEdit();
	void onOrientYEdit();
	void onOrientZEdit();
	void onCenterXEdit();
	void onCenterYEdit();
	void onCenterZEdit();
	void onScaleEdit();

private:
	Ui::PropertyMeshObject m_ui;
	typedef void (PropertyMeshObject::* PropertyMeshObjectMethod)(IGuiData*);
	std::unordered_map<guiDType, PropertyMeshObjectMethod> m_meshObjMethods;

	SafePtr<MeshObjectNode> m_object;
};

#endif //PROPERTY_WAVEFRONT_H_