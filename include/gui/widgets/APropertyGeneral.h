#ifndef PROPERTY_GENERAL_H_
#define PROPERTY_GENERAL_H_

#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"
#include <QtWidgets/qwidget.h>
#include "utils/safe_ptr.h"

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