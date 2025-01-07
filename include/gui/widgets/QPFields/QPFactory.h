#ifndef QP_FACTORY_H_
#define QP_FACTORY_H_

#include "gui/widgets/QPFields/AQPField.h"

class QPFactory
{
public:
	QPFactory();
	~QPFactory();

	AQPField* getField(const sma::tField field);
	//TemplatePropertiesPanel *createTemplatePanel(IDataDispatcher& dispat, sma::TagTemplate temp, QWidget *parent = nullptr);

};

#endif // !QP_FACTORY_H_
