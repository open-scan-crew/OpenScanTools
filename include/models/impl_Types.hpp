#ifndef IMPL_TYPES_HPP
#define IMPL_TYPES_HPP

#include "models/Types.hpp"

#include <ostream>
#include <magic_enum/magic_enum.hpp>

inline std::ostream& operator<<(std::ostream& out, const ElementType& type)
{
	out << magic_enum::enum_name<ElementType>(type);
	return (out);
}

inline std::ostream& operator<<(std::ostream& out, const TreeType& type)
{
	out << magic_enum::enum_name<TreeType>(type);
	return (out);
}

#endif