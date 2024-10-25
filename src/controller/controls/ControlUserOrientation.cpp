#include "controller\controls\ControlUserOrientation.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "gui/GuiData/GuiDataUserOrientation.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/Texts.hpp"


//***	UserOrientationProperties	***//

control::userOrientation::UserOrientationsProperties::UserOrientationsProperties(userOrientationId id) : _userOrientationId(id)
{}

control::userOrientation::UserOrientationsProperties::UserOrientationsProperties() : _userOrientationId(("bad-guid-string"))
{}

control::userOrientation::UserOrientationsProperties::~UserOrientationsProperties()
{}

void control::userOrientation::UserOrientationsProperties::doFunction(Controller & controller)
{

	if (_userOrientationId.isValid()) {
		auto it = controller.getContext().getUserOrientations().find(_userOrientationId);
		if (it != controller.getContext().getUserOrientations().end())
			controller.updateInfo(new GuiDataUserOrientationProperties(it->second));
	}
	else
		controller.updateInfo(new GuiDataUserOrientationProperties());

}

ControlType control::userOrientation::UserOrientationsProperties::getType() const
{
	return ControlType::userOrientationProperties;
}

//***	SetUserOrientation	***//

control::userOrientation::SetUserOrientation::SetUserOrientation(userOrientationId id) : _userOrientationId(id)
{}

control::userOrientation::SetUserOrientation::~SetUserOrientation()
{}

void control::userOrientation::SetUserOrientation::doFunction(Controller & controller)
{
	UserOrientation uo;
	auto it = controller.getContext().getUserOrientations().find(_userOrientationId);
	if (it != controller.getContext().getUserOrientations().end())
		uo = it->second;
	else
		return;

	controller.getContext().setActiveUserOrientation(uo);
	controller.updateInfo(new GuiDataSetUserOrientation(uo));
}

ControlType control::userOrientation::SetUserOrientation::getType() const
{
	return ControlType::setUserOrientation;
}

control::userOrientation::UnsetUserOrientation::UnsetUserOrientation()
{}

control::userOrientation::UnsetUserOrientation::~UnsetUserOrientation()
{}

void control::userOrientation::UnsetUserOrientation::doFunction(Controller& controller)
{
	controller.getContext().setActiveUserOrientation(UserOrientation());
	controller.updateInfo(new GuiDataUnsetUserOrientation());
}

ControlType control::userOrientation::UnsetUserOrientation::getType() const
{
	return ControlType::unsetUserOrientation;
}

control::userOrientation::CreateEditUserOrientation::CreateEditUserOrientation(const UserOrientation& uo) : m_uo(uo)
{}

control::userOrientation::CreateEditUserOrientation::~CreateEditUserOrientation()
{
}

void control::userOrientation::CreateEditUserOrientation::doFunction(Controller & controller)
{
	if (m_uo.getName() == "") {
		controller.updateInfo(new GuiDataWarning(TEXT_USER_ORIENTATION_PROPERTIES_WARNING_NO_NAME));
		return;
	}

	std::unordered_map<userOrientationId, UserOrientation>& orientations = controller.getContext().getUserOrientations();
	for (auto ori = orientations.begin(); ori != orientations.end(); ori++) {
		if (ori->second.getName() == m_uo.getName() && ori->second.getId() != m_uo.getId()) {
			controller.updateInfo(new GuiDataWarning(TEXT_USER_ORIENTATION_PROPERTIES_WARNING_NAME_ALREADY_USED));
			return;
		}
	}

	auto editElement = orientations.insert_or_assign(m_uo.getId(), m_uo);
	if (editElement.second)
		orientations.at(m_uo.getId()).setOrder((uint32_t)orientations.size() - 1);

	controller.updateInfo(new GuiDataCallbackAddRemoveUserOrientation(m_uo.getId(), false, m_uo.getName()));
	controller.updateInfo(new GuiDataCloseUserOrientationProperties());
	
}

ControlType control::userOrientation::CreateEditUserOrientation::getType() const
{
	return ControlType::createEditUserOrientation;
}

/***	DeleteUserOrientation	***/

control::userOrientation::DeleteUserOrientation::DeleteUserOrientation(userOrientationId delete_id) : _delete_id(delete_id)
{
}

control::userOrientation::DeleteUserOrientation::~DeleteUserOrientation()
{

}

void control::userOrientation::DeleteUserOrientation::doFunction(Controller & controller)
{
	std::unordered_map<userOrientationId, UserOrientation>& uos = controller.getContext().getUserOrientations();
	uint32_t orderDelete;
	auto it = uos.find(_delete_id);
	if (it != uos.end()) {
		orderDelete = it->second.getOrder();
	}
	else
		return;

	if (!uos.erase(_delete_id))
		return;

	for (auto it = uos.begin(); it != uos.end(); it++) {
		uint32_t order = it->second.getOrder();
		if (order > orderDelete)
			it->second.setOrder(order - 1);
	}

	controller.updateInfo(new GuiDataCallbackAddRemoveUserOrientation(_delete_id, true));
}

ControlType control::userOrientation::DeleteUserOrientation::getType() const
{
	return ControlType::deleteUserOrientation;
}