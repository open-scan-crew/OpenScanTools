#include "io/exports/txtExport.h"
#include "utils/ProjectColor.hpp"

txtExport::txtExport()
{
	f_closed;
	m_status;
}

txtExport::~txtExport()
{
	end();
}

bool txtExport::begin(const std::filesystem::path & fileName)
{
	if (f_closed == false)
		return (false);

	m_file = std::ofstream(fileName);
	if (m_file.is_open() == false)
		return (false);
	f_closed = false;
	m_status = true;

	m_file << "Nom de tag";
	m_file << TXTSEPARATOR;
	m_file << "Index";
	m_file << TXTSEPARATOR;
	m_file << "Action";
	m_file << TXTSEPARATOR;
	m_file << "Couleur";
	m_file << TXTSEPARATOR;
	m_file << "Position X";
	m_file << TXTSEPARATOR;
	m_file << "Position Y";
	m_file << TXTSEPARATOR;
	m_file << "Position Z";
	m_file << TXTSEPARATOR;
	m_file << "Description";
	m_file << std::endl;

	return (true);
}

bool txtExport::end()
{
	if (f_closed == false)
	{
		m_file.close();
		f_closed = true;
	}
	return m_status;
}

/*
bool txtExport::tag(const Tag& tagToExport)
{
	//FIXME
	m_file << tagToExport.getName();
	m_file << TXTSEPARATOR;
	m_file << tagToExport.getUserIndex();
	m_file << TXTSEPARATOR;
	//m_file << tagToExport.getAction();
	//m_file << TXTSEPARATOR;
	m_file << tagToExport.getColor().getWStringHexa();
	m_file << TXTSEPARATOR;
	m_file << tagToExport.getCenter().x;
	m_file << TXTSEPARATOR;
	m_file << tagToExport.getCenter().y;
	m_file << TXTSEPARATOR;
	m_file << tagToExport.getCenter().z;

	//fixme
	//m_file << TXTSEPARATOR;
	//m_file << tagToExport.getDescription();

	m_file << std::endl;
	
	return (true);
}
*/