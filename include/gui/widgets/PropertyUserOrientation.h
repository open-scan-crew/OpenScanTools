#ifndef PROPERTY_USER_ORIENTATIONS_H
#define PROPERTY_USER_ORIENTATIONS_H

#include <vector>
#include "gui/widgets/APropertyGeneral.h"
#include "ui_Property_User_Orientation.h"

#include "models/application/UserOrientation.h"

class PropertyUserOrientation;

typedef void (PropertyUserOrientation::* PropertyUserOrientationMethod)(IGuiData*);

class PropertyUserOrientation : public APropertyGeneral
{
	Q_OBJECT

public:
	explicit PropertyUserOrientation(IDataDispatcher &dataDispatcher, QWidget *parent = nullptr, float guiScale = 1.f);
	~PropertyUserOrientation();

	virtual bool actualizeProperty(SafePtr<AGraphNode> object );

public:
	void informData(IGuiData *keyValue);

	void hideEvent(QHideEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:

	void onAbort(IGuiData* data);
	void onPointReceived(IGuiData* data);
	void onUserOrientation(IGuiData* data);
	void onClose(IGuiData* data);

	void updateUO();
	void updateUI();

	void Clean();

	void Close();

private slots:
	void updateCustomAxisFrame();

	//Orientation Axis
	void onPoint1Click();
	void onPoint2Click();
	void onCustomAxisUpdate();

	//Translation
	void onPointXClick();
	void onPointYClick();
	void onPointZClick();
	void onNewPointEdit();

	void onOkButton();
	void onResetButton();
	void onDeleteButton();

private:
	glm::dvec3* m_currentEditPoint;

	std::array<glm::dvec3,2> m_axisPoints;
	std::array<glm::dvec3, 2> m_customAxis;

	glm::dvec3 m_oldPoint;
	glm::dvec3 m_newPoint;

	bool m_empty;
	UserOrientation m_uo;

	Ui::PropertyUserOrientation m_ui;
	std::unordered_map<guiDType, PropertyUserOrientationMethod> m_UOmethods;

};

#endif //PROPERTY_USER_ORIENTATIONS_H