#ifndef OPENSCANTOOLS_MODE_ESSENTIALS_IMPL_H
#define OPENSCANTOOLS_MODE_ESSENTIALS_IMPL_H

#include "models/OpenScanToolsModelEssentials.h"

#include <ostream>
#include <iomanip>

inline std::ostream& operator<<(std::ostream& out, const Pos3D& vec)
{
	out << std::setprecision(4) << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
	return (out);
}

#endif // !OPENSCANTOOLS_MODE_ESSENTIALS_IMPL_H