#include "gui/widgets/APropertyGeneral.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataRendering.h"

#include <qlineedit.h>
#include <qabstractbutton.h>
#include <qcombobox.h>
#include <qabstractspinbox.h>
#include <qtextedit.h>
#include <qplaintextedit.h>
#include <qgroupbox.h>
#include <qabstractslider.h>

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
