#include "vulkan/RenderingSwitchConfiguration.h"

RenderingSwitchConfiguration::RenderingSwitchConfiguration(const std::unordered_map<tls::PointFormat, std::list<RenderMode>>& rules)
	: m_rules(rules)
{
	if (m_rules.empty())
	{
		/*m_rules = { 
			{ tls::PointFormat::TL_POINT_XYZ_I, { RenderMode::Intensity, RenderMode::Flat_I, RenderMode::Grey_Colored, RenderMode::Grey_Ramp }},
			{ tls::PointFormat::TL_POINT_XYZ_RGB, { RenderMode::RGB, RenderMode::Flat_I, RenderMode::Grey_Colored, RenderMode::Grey_Ramp }},
			{ tls::PointFormat::TL_POINT_XYZ_I_RGB, { RenderMode::Intensity, RenderMode::RGB, RenderMode::Flat_I, RenderMode::Grey_Colored, RenderMode::Grey_Ramp, RenderMode::IntensityRGB_Combined }}
		};*/
	}
}

RenderingSwitchConfiguration::~RenderingSwitchConfiguration()
{}

const std::list<RenderMode>& RenderingSwitchConfiguration::getAvailableMode(const tls::PointFormat& format = tls::PointFormat::TL_POINT_FORMAT_UNDEFINED) const
{
	if(m_rules.find(format) != m_rules.end())
		return m_rules.at(format);
	return m_rules.begin()->second;
}

bool RenderingSwitchConfiguration::isAcceptableRenderingMode(const tls::PointFormat& format, const RenderMode& mode) const
{
	std::unordered_map<tls::PointFormat, std::list<RenderMode>>::const_iterator iterator(m_rules.find(format));
	if (iterator == m_rules.end())
		return false;
	auto renderIterator(std::find(iterator->second.begin(), iterator->second.end(), mode));
	return (renderIterator != iterator->second.end());
}

RenderMode  RenderingSwitchConfiguration::getAcceptableRenderingMode(const tls::PointFormat& format, const RenderMode& mode) const
{
	return RenderMode::RenderMode_MaxEnum;
}