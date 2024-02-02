#ifndef PROPERTY_ELBOW_H_
#define PROPERTY_ELBOW_H_

#include "gui/widgets/APropertyGeneral.h"
#include "ui_Property_Elbow.h"


class Controller;

class TorusNode;

class PropertyElbow;

typedef void (PropertyElbow::* PropertyElbowMethod)(IGuiData*);


class PropertyElbow : public APropertyGeneral
{
	Q_OBJECT

public:
	//enum DiameterMode { Standard, Forced, Detected };
public:
	explicit PropertyElbow(Controller& controller, QWidget *parent = nullptr, float guiScale = 1.f);
	~PropertyElbow();

	void hideEvent(QHideEvent* event);
	virtual bool actualizeProperty(SafePtr<AGraphNode> object );

private:
	void informData(IGuiData *keyValue);

	//void onStandardRecieved(IGuiData* data);
	bool updateTorus();

	//void onDiameterChange();

	//void onDiameterMethod(const DiameterMode& mode);

public slots:
	//void onStandardChange(const int& index);

	//void setStandards(const std::unordered_map<listId, StandardList>& list);

private:
	Ui::PropertyElbow m_ui;
	std::unordered_map<guiDType, PropertyElbowMethod> m_elbowMethods;

	SafePtr<TorusNode> m_storedTorus;
	//double m_diameter;
	//DiameterMode m_mode;
	uint32_t m_standardSet;
	std::vector<std::pair<listId, std::string>> m_standards;

	glm::dvec3 m_eulers;
};

#endif //PROPERTY_ELBOW_H_