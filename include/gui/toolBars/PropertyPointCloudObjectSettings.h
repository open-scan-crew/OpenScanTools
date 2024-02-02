#ifndef PROPERTY_POINT_CLOUD_OBJECT_SETTINGS_H_
#define PROPERTY_POINT_CLOUD_OBJECT_SETTINGS_H_

#include <QtWidgets/QWidget>
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"
#include "models/data/SimpleMeasure.h"
#include "ui_Property_Clipping_Settings.h"

#include <utility>
#include <vector>

class PropertyPointCloudObjectSettings : public QWidget, public IPanel
{
	Q_OBJECT

public:
	explicit PropertyPointCloudObjectSettings(IDataDispatcher& dataDispatcher, QWidget* parent = nullptr, float guiScale = 1.f);
	~PropertyPointCloudObjectSettings();

public:
	void informData(IGuiData* keyValue);
	std::string getName() const override;

private:
	void CleanP1();
	void CleanP2();
	bool sendSize();
	bool sendOffset();
	int getStepX();
	int getStepY();
	int getStepZ();

public slots:

	void onXSizeEdit();
	void onYSizeEdit();
	void onZSizeEdit();
	void onXOffsetEdit();
	void onYOffsetEdit();
	void onZOffsetEdit();
	void onCenterClick();
	void onBottomClick();
	void onTopClick();
	void onXStepSizeClick();
	void onYStepSizeClick();
	void onZStepSizeClick();
	void onXNStepSizeClick();
	void onYNStepSizeClick();
	void onZNStepSizeClick();

	void duplicationOnClick();
	void duplicationOnStepSize();
	void duplicationOnOffset();
	void onOffsetLocal();
	void onOffsetGlobal();
	void onProjectAxis();
	void on2Points();
	void onPoint1Click();
	void onPoint2Click();

private:
	QString m_xStored;
	QString m_yStored;
	QString m_zStored;
	QString m_xOffsetStored;
	QString m_yOffsetStored;
	QString m_zOffsetStored;
	int m_point;
	glm::bvec2 m_bpoints;
	glm::vec2 m_points[2];
	double m_angle;
	Ui::PropertyClippingSettings m_ui;
	IDataDispatcher& m_dataDispatcher;
};

#endif //PROPERTY_POINT_CLOUD_OBJECT_SETTINGS_H_