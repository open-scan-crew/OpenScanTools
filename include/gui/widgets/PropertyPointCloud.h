#ifndef PROPERTY_POINT_CLOUD_H
#define PROPERTY_POINT_CLOUD_H

#include "gui/widgets/APropertyGeneral.h"
#include "ui_Property_Scan.h"

class AGraphNode;
class APointCloudNode;
class TransformationModule;

class Controller;

class PropertyPointCloud : public APropertyGeneral
{
	Q_OBJECT

public:
	PropertyPointCloud(const Controller& controller, QWidget* parent);
	~PropertyPointCloud();

	void hideEvent(QHideEvent* event);

	virtual bool actualizeProperty(SafePtr<AGraphNode> object );

private:
	bool update();
	void setObject3DParameters(const TransformationModule& data);

	void changeOrientation();
	void changeCenter();
	void changeScale();
	
public slots:
	void onOrientXEdit();
	void onOrientYEdit();
	void onOrientZEdit();
	void onCenterXEdit();
	void onCenterYEdit();
	void onCenterZEdit();
	void onScaleXEdit();
	void onScaleYEdit();
	void onScaleZEdit();

	void slotSetScanColor();
	void slotClippableChanged(int value);
	void addAsAnimationViewPoint();


private:
	SafePtr<APointCloudNode> m_storedScan;
	Ui::PropertyScan m_ui;
};

#endif // _PROPERTIESSCAN_PANEL_H_

