#include "gui/widgets/QPFields/AQPField.h"

AQPField::AQPField(const sma::tField& field, QWidget* parent)
	: QWidget(parent) 
{}

sma::tFieldType AQPField::getType() const 
{
	return m_field.m_type; 
}

sma::tFieldId AQPField::getFieldId() const 
{
	return m_field.m_id; 
}

sma::tField AQPField::getFieldData() const
{ 
	return m_field; 
}