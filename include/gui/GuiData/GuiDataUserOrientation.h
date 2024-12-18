#ifndef GUI_DATA_USER_ORIENTATIONS_H_
#define GUI_DATA_USER_ORIENTATIONS_H_

#include "gui/GuiData/IGuiData.h"
#include "models/application/UserOrientation.h"



/* Ouvre les propriétés d'UserOrientation et envoie les données  */
class GuiDataUserOrientationProperties : public IGuiData
{
public:
	GuiDataUserOrientationProperties();
	GuiDataUserOrientationProperties(const UserOrientation& uo);
	~GuiDataUserOrientationProperties() {}
	guiDType getType() override;
public:
	UserOrientation m_userOrientation;
	bool m_empty;
};

/* Retourne l'information de modification de la liste d'UserOrientation à l'UI*/
class GuiDataCallbackAddRemoveUserOrientation : public IGuiData
{
public:
	GuiDataCallbackAddRemoveUserOrientation(userOrientationId id, bool remove = false, QString editName = "PROBLEM! NO NAME");
	~GuiDataCallbackAddRemoveUserOrientation() {}
	guiDType getType() override;
public:
	userOrientationId m_id;
	QString m_name;
	bool m_remove;
};

class GuiDataCloseUserOrientationProperties : public IGuiData
{
public:
	GuiDataCloseUserOrientationProperties();
	~GuiDataCloseUserOrientationProperties() {}
	guiDType getType() override;
};

/* Envoie l'UserOrientation sélectionnée dans la toolbar*/
class GuiDataSetUserOrientation : public IGuiData
{
public:
	GuiDataSetUserOrientation(const UserOrientation& uo);
	~GuiDataSetUserOrientation() {}
	guiDType getType() override;
public:
	UserOrientation m_userOrientation;
};

class GuiDataUnsetUserOrientation : public IGuiData
{
public:
	GuiDataUnsetUserOrientation();
	~GuiDataUnsetUserOrientation() {}
	guiDType getType() override;
};

/* Envoie la liste pour afficher les UserOrientations dans la toolbar*/
class GuiDataSendUserOrientationList : public IGuiData
{
public:
	GuiDataSendUserOrientationList(const std::unordered_map<uint32_t, std::pair<userOrientationId, QString>>& orientations);
	~GuiDataSendUserOrientationList() {}
	guiDType getType() override;
public:
	std::unordered_map<uint32_t, std::pair<userOrientationId, QString>> m_orientations;
};

#endif // !GUI_DATA_USER_ORIENTATIONS_H_
