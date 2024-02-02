#ifndef _CSVWRITER_HXX_
#define _CSVWRITER_HXX_

#include "CSVWriter.h"

template<typename T>
CSVWriter& CSVWriter::operator<<(T&& value)
{
	m_ostream << value << m_separator;
	return (*this);
}


#endif //_CSVWRITER_HXX_