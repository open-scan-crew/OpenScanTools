#include "pointCloudEngine/RenderingTypes.h"

#include "gui/texts/RenderingTexts.hpp"
#include <qstring.h>

std::unordered_map<UiRenderMode, std::string>  getTradUiRenderMode()
{
	return std::unordered_map<UiRenderMode, std::string>({
		{ UiRenderMode::RGB, TEXT_RENDERING_TYPES_RGB.toStdString()},
	    { UiRenderMode::Intensity, TEXT_RENDERING_TYPES_INTENSITY.toStdString()},
	    { UiRenderMode::IntensityRGB_Combined, TEXT_RENDERING_TYPES_INTENSITY_RGB_COMBINED.toStdString()},
	    { UiRenderMode::Grey_Colored, TEXT_RENDERING_TYPES_GREY_COLORED.toStdString()},
	    { UiRenderMode::Scans_Color, TEXT_RENDERING_TYPES_SCANS_COLOR.toStdString()},
	    { UiRenderMode::Clusters_Color, TEXT_RENDERING_TYPES_CLUSTERS_COLOR.toStdString()},
	    { UiRenderMode::Flat, TEXT_RENDERING_TYPES_FLAT.toStdString()},
	    { UiRenderMode::Distance_Ramp, TEXT_RENDERING_TYPES_DISTANCE_RAMP.toStdString()},
	    { UiRenderMode::Flat_Distance_Ramp, TEXT_RENDERING_TYPES_FLAT_DISTANCE_RAMP.toStdString()},
	    { UiRenderMode::Fake_Color, TEXT_RENDERING_TYPES_FAKE_COLOR.toStdString()},
        { UiRenderMode::Normals_Colored, TEXT_RENDERING_TYPES_NORMALS_COLORED.toStdString()}
	});
}