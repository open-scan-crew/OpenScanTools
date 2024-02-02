#include "gui/GuiData/GuiDataClipping.h"
#include "utils/Logger.h"
#include "io/FileUtils.h"

// **** GuiDataClippingSettingsProperties ****

GuiDataClippingSettingsProperties::GuiDataClippingSettingsProperties(QString windowsName, bool onlyDuplication)
	: m_windowsName(windowsName)
	, m_onlyDuplication(onlyDuplication)
{}

guiDType GuiDataClippingSettingsProperties::getType()
{
	return (guiDType::clippingSettingsProperties);
}

GuiDataPointCloudObjectDialogDisplay::GuiDataPointCloudObjectDialogDisplay(const PointCloudObjectParameters& parameters)
	: m_parameters(parameters)
{}

guiDType GuiDataPointCloudObjectDialogDisplay::getType()
{
	return (guiDType::pcoCreationParametersDisplay);
}