#ifndef PROPERTY_GENERAL_H_
#define PROPERTY_GENERAL_H_

#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"
#include "models/LicenceTypes.h"
#include <QtWidgets/QWidget>
#include "models/OpenScanToolsModelEssentials.h"

class AGraphNode;

class APropertyGeneral : public QWidget, public IPanel
{
public:
	explicit APropertyGeneral(IDataDispatcher& dataDispatcher, QWidget* parent);
	~APropertyGeneral();

	void informData(IGuiData* keyValue) override;
	virtual bool actualizeProperty(SafePtr<AGraphNode> object) = 0;


public:
	IDataDispatcher&								m_dataDispatcher;
};

#endif //PROPERTY_GENERAL_H_