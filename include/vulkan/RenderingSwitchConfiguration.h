#ifndef RENDERING_SWITCH_CONFIGURATION_H_
#define RENDERING_SWITCH_CONFIGURATION_H_

#include "tls_def.h"
#include "pointCloudEngine/RenderingTypes.h"

#include <unordered_map>

class RenderingSwitchConfiguration
{
public:
	RenderingSwitchConfiguration(const std::unordered_map<tls::PointFormat, std::list<RenderMode>>& rules = {});
	~RenderingSwitchConfiguration();

	const std::list<RenderMode>& getAvailableMode(const tls::PointFormat& format) const;
	bool isAcceptableRenderingMode(const tls::PointFormat& format, const RenderMode& mode) const;
	RenderMode getAcceptableRenderingMode(const tls::PointFormat& format, const RenderMode& mode) const;

private:
	std::unordered_map<tls::PointFormat, std::list<RenderMode>> m_rules;
};

#endif