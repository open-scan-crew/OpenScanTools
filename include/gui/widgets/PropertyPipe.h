#ifndef PROPERTY_PIPE_H_
#define PROPERTY_PIPE_H_

#include "gui/widgets/APropertyGeneral.h"
#include "ui_Property_Pipe.h"

#include "models/data/Piping/StandardRadiusData.h"
#include <vector>

class Controller;

class CylinderNode;
class TransformationModule;

enum class ManipulationMode;

class PropertyPipe : public APropertyGeneral
{
	Q_OBJECT

public:
	explicit PropertyPipe(Controller& controller, QWidget *parent = nullptr, float guiScale = 1.f);
	~PropertyPipe();

	void hideEvent(QHideEvent* event);

	virtual bool actualizeProperty(SafePtr<AGraphNode> object );

private:
	void informData(IGuiData* keyValue) override;

	void onStandardRecieved(IGuiData* data);
	bool updateCylinder();
	void setObject3DParameters(const TransformationModule& data);

	void onDiameterChange();

	void onDiameterMethod(const StandardRadiusData::DiameterSet& mode);

	void blockSignals(bool block);


public slots:
	void onStandardChange(const int& index);

	void onDiameterEdit();

	void setStandards(const std::unordered_set<SafePtr<StandardList>>& lists);
	void updateStandard();

private:
	Ui::PropertyPipe m_ui;
	typedef void (PropertyPipe::* PropertyPipeMethod)(IGuiData*);
	std::unordered_map<guiDType, PropertyPipeMethod> m_pipeMethods;

	SafePtr<CylinderNode> m_cylinder;

	double m_forcedDiameter;
	StandardRadiusData::DiameterSet m_mode;
	uint32_t m_standardSet;

	std::vector<SafePtr<StandardList>> m_standards;
};

#endif //PROPERTY_PIPE_H_