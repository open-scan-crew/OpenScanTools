#ifndef PROPERTY_CLIPPING_H_
#define PROPERTY_CLIPPING_H_

#include "gui/widgets/APropertyGeneral.h"
#include "ui_Property_Box.h"

class Controller;
class BoxNode;

class PropertyBox;

typedef void (PropertyBox::* PropertyBoxMethod)(IGuiData*);

class PropertyBox : public APropertyGeneral
{
	Q_OBJECT

public:
	explicit PropertyBox(Controller& controller, QWidget *parent = nullptr, float guiScale = 1.f);
	~PropertyBox();
	void hideEvent(QHideEvent* event) override;

	virtual bool actualizeProperty(SafePtr<AGraphNode> object );

private:
	void informData(IGuiData *keyValue);

	bool updateBox();

	void changeSize();
	void changeOrientation();
	void changeCenter();
	void changeDivision();
	void blockGridSignals(bool value);
	void blockClippingSignals(bool value);
	void blockAllSignals(bool value);


public slots:
	void onSizeXEdit();
	void onSizeYEdit();
	void onSizeZEdit();
	void onOrientXEdit();
	void onOrientYEdit();
	void onOrientZEdit();
	void onCenterXEdit();
	void onCenterYEdit();
	void onCenterZEdit();
	void onClippingExpand();
	void onGridDivisionByStep();
	void onGridDivisionByMultiple();
	void onGridXEdit();
	void onGridYEdit();
	void onGridZEdit();

private:
	Ui::PropertyBox m_ui;
	std::unordered_map<guiDType, PropertyBoxMethod> m_clippingMethods;

	SafePtr<BoxNode> m_storedBox;
	glm::dvec3 m_eulers = glm::dvec3(0.0, 0.0, 0.0);
};

#endif //PROPERTY_CLIPPING_H_