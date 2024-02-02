#ifndef PROPERTY_DUPLICATION_SETTINGS_H
#define PROPERTY_DUPLICATION_SETTINGS_H

#include <QtWidgets/QWidget>
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"
#include "ui_Property_Duplication_Settings.h"

#include "gui/UnitConverter.h"

class PropertyDuplicationSettings : public QWidget, public IPanel
{
	Q_OBJECT

public:
	explicit PropertyDuplicationSettings(IDataDispatcher* dataDispatcher, QWidget* parent = nullptr, float guiScale = 1.f);
	explicit PropertyDuplicationSettings(QWidget* parent = nullptr);

	~PropertyDuplicationSettings();
	void hideEvent(QHideEvent* event);
	void initialise(IDataDispatcher* dataDispatcher, float guiScale = 1.f);
public:
	void informData(IGuiData* keyValue);

private:
	void init();
	void sendOffset();
	int getStepX();
	int getStepY();
	int getStepZ();

	void updateUI();

public slots:
	void onXOffsetEdit();
	void onYOffsetEdit();
	void onZOffsetEdit();
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

private:
	float m_xOffsetStored;
	float m_yOffsetStored;
	float m_zOffsetStored;
	Ui::PropertyDuplicationSettings m_ui;
	IDataDispatcher* m_dataDispatcher;
};

#endif //PROPERTY_DUPLICATION_SETTINGS_H_