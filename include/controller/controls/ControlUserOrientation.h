#ifndef CONTROL_USERORIENTATION_H_
#define CONTROL_USERORIENTATION_H_

#include "controller/controls/IControl.h"
#include "models/application/UserOrientation.h"

namespace control
{
	namespace userOrientation
	{
		/* Envoie l’ordre d’ouverture des propriétés pour la création/édition d’une orientation utilisateur. */
		class UserOrientationsProperties : public AControl
		{
		public:
			UserOrientationsProperties(userOrientationId id);
			UserOrientationsProperties();
			~UserOrientationsProperties();
			void doFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			userOrientationId _userOrientationId;
		};

		/* Envoie l’information d’orientation utilisateur qui est actuellement utilisé */
		class SetUserOrientation : public AControl
		{
		public:
			SetUserOrientation(userOrientationId id);
			~SetUserOrientation();
			void doFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			userOrientationId _userOrientationId;
		};

		/* Envoie l’information d’orientation utilisateur qui est actuellement utilisé */
		class UnsetUserOrientation : public AControl
		{
		public:
			UnsetUserOrientation();
			~UnsetUserOrientation();
			void doFunction(Controller& controller) override;
			ControlType getType() const override;
		};
		/* Ajoute ou édite une orientation utilisateur dans la liste des orientations utilisateurs du modèle */
		class CreateEditUserOrientation : public AControl
		{
		public:
			CreateEditUserOrientation(const UserOrientation& uo);
			~CreateEditUserOrientation();
			void doFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			UserOrientation m_uo;
		};

		/* Supprime une orientation utilisateur de la liste des orientations utilisateurs du modèle */
		class DeleteUserOrientation : public AControl
		{
		public:
			DeleteUserOrientation(userOrientationId delete_id);
			~DeleteUserOrientation();
			void doFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			userOrientationId _delete_id;
		};
	}
}

#endif // !CONTROL_USERORIENTATION_H_