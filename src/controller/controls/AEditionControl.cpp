#include "controller/controls/AEditionControl.h"
#include "models/data/Data.h"

#include "controller/Controller.h"
#include "utils/Logger.h"

AEditionControl::AEditionControl()
	: m_doTimeModified(0)
	, m_undoTimeModified(0)
{}

AEditionControl::AEditionControl(const std::time_t& redoTime)
	: m_doTimeModified(0)
	, m_undoTimeModified(redoTime)
{}

AEditionControl::~AEditionControl()
{}


void AEditionControl::doTimeModified(Data& data)
{

	m_undoTimeModified = data.getModificationTime();
	if(!m_doTimeModified)
		m_doTimeModified = std::time(nullptr);
	data.setModificationTime(m_doTimeModified);
}

void AEditionControl::undoTimeModified(Data& data)
{
	data.setModificationTime(m_undoTimeModified);
}
