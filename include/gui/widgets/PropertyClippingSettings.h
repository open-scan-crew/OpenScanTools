#ifndef PROPERTY_CLIPPING_SETTINGS_H
#define PROPERTY_CLIPPING_SETTINGS_H

#include <QtWidgets/qwidget.h>
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"
#include "ui_Property_Clipping_Settings.h"

#include <glm/glm.hpp>

class PropertyClippingSettings : public QWidget, public IPanel
{
	Q_OBJECT

public:
	explicit PropertyClippingSettings(IDataDispatcher &dataDispatcher, QWidget *parent = nullptr, float guiScale = 1.f);
	~PropertyClippingSettings();

public:
	void informData(IGuiData *keyValue);
	void hideEvent(QHideEvent* event) override;
    void showEvent(QShowEvent* event) override;

	void showHideCreationSettings(bool hide);

private:
	void CleanP1();
	void CleanP2();
	bool sendSize();

public slots:

	void onXSizeEdit();
	void onYSizeEdit();
	void onZSizeEdit();
	void onCenterClick();
	void onBottomClick();
	void onTopClick();

	void onProjectAxis();
	void on2Points();
	void onUserOrientation();

	void onPoint1Click();
	void onPoint2Click();

private:
	float m_xStored;
	float m_yStored;
	float m_zStored;
	int m_point;
	glm::bvec2 m_bpoints;
	glm::vec2 m_points[2];
	double m_angle;
	double m_userAngle;
	Ui::PropertyClippingSettings *m_ui;
	IDataDispatcher &m_dataDispatcher;
};

#endif //PROPERTY_CLIPPING_SETTINGS_H_