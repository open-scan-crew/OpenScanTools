#include "gui/GuiData/GuiDataUserOrientation.h"

/////


GuiDataUserOrientationProperties::GuiDataUserOrientationProperties() : m_empty(true)
{}

GuiDataUserOrientationProperties::GuiDataUserOrientationProperties(const UserOrientation& uo) : m_userOrientation(uo), m_empty(false)
{}

guiDType GuiDataUserOrientationProperties::getType()
{
	return guiDType::userOrientationProperties;
}

/////


GuiDataCallbackAddRemoveUserOrientation::GuiDataCallbackAddRemoveUserOrientation(userOrientationId id, bool remove, QString editName) : m_id(id), m_name(editName), m_remove(remove)
{}

guiDType GuiDataCallbackAddRemoveUserOrientation::getType()
{
	return guiDType::callbackUO;
}

/////


GuiDataCloseUserOrientationProperties::GuiDataCloseUserOrientationProperties()
{}

guiDType GuiDataCloseUserOrientationProperties::getType()
{
	return guiDType::closeUOProperties;
}

/////

GuiDataSetUserOrientation::GuiDataSetUserOrientation(const UserOrientation& uo) : m_userOrientation(uo)
{}

guiDType GuiDataSetUserOrientation::getType()
{
	return guiDType::userOrientation;
}

/////

GuiDataUnsetUserOrientation::GuiDataUnsetUserOrientation()
{}

guiDType GuiDataUnsetUserOrientation::getType()
{
	return guiDType::projectOrientation;
}

/////

GuiDataSendUserOrientationList::GuiDataSendUserOrientationList(const std::unordered_map<uint32_t, std::pair<userOrientationId, QString>>& orientations) : m_orientations(orientations)
{
}

guiDType GuiDataSendUserOrientationList::getType()
{
	return guiDType::sendUserOrientationList;
}

/////

