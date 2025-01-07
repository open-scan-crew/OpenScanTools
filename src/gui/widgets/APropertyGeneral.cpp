#include "gui/widgets/APropertyGeneral.h"


APropertyGeneral::APropertyGeneral(IDataDispatcher& dataDispatcher, QWidget* parent)
	: QWidget(parent)
	, m_dataDispatcher(dataDispatcher)
{
}

APropertyGeneral::~APropertyGeneral()
{
	m_dataDispatcher.unregisterObserver(this);
}

void APropertyGeneral::informData(IGuiData* keyValue)
{
}
