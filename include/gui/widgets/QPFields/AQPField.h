#ifndef AQP_FIELD_H_
#define AQP_FIELD_H_

#include <QtWidgets/QWidget>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLayout>

#include "models/application/TagTemplate.h"

class AQPField : public QWidget
{
	Q_OBJECT

public:
	AQPField(const sma::tField& field, QWidget* parent);

	sma::tFieldType getType() const;
	sma::tFieldId getFieldId() const;
	sma::tField getFieldData() const;

	virtual void setValue(QString value) = 0;

signals:
	void edited(sma::tFieldId id, std::wstring newValue);

protected:
	virtual void buildQPField(const sma::tField& field) = 0;

protected:
	sma::tField m_field;
};

#endif // !IQP_FIELD_H_