#ifndef ACTUALIZE_OPTIONS_H
#define ACTUALIZE_OPTIONS_H

#include <functional>

struct ActualizeOptions
{
	bool m_treeActualize = false;

	ActualizeOptions(bool treeActualize)
		: m_treeActualize(treeActualize)
	{}

	ActualizeOptions()
		: m_treeActualize(false)
	{}

	bool operator==(const ActualizeOptions& other) const
	{
		return m_treeActualize == other.m_treeActualize;
	}
};

template<>
struct std::hash<ActualizeOptions> {
	std::size_t operator()(const ActualizeOptions& options) const {
		return std::hash<bool>{}((bool)options.m_treeActualize);
	}
};

#endif // !ACTUALIZE_OPTIONS_H
