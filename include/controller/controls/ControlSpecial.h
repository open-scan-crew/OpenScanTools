#ifndef CONTROL_SPECIAL_H_
#define CONTROL_SPECIAL_H_

#include "controller/controls/IControl.h"
#include "models/Types.hpp"
#include "utils/safe_ptr.h"

#include "crossguid/guid.hpp"

#include <unordered_set>
#include <unordered_map>

class AGraphNode;

namespace control
{
	namespace special
	{
		/*! Supprime les Data correspondant aux xg::Guid  en entrée. Les fichiers de scans ne sont pas supprimées et le contrôle peut être undo
		
		Il ne provoque pas l'alerte en cas de suppression de données importantes (scans)
		*/

		class DeleteElement : public AControl
		{
		public:
			/*!
			\param std::set<xg::Guid> idToDelete : Liste des dataIds à supprimer
			*/
			DeleteElement(std::unordered_set<SafePtr<AGraphNode>> datasToDelete, bool isUndoAble);
			~DeleteElement();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			std::unordered_set<SafePtr<AGraphNode>> m_datasToDelete;
			std::unordered_set<SafePtr<AGraphNode>> m_elemsDeleted;
		};

		/*! Supprime les objets/scans sélectionnées

		Ils ne peuvent pas être annulés

		Ce contrôle sauvegarde aussi le projet*/
		class DeleteTotalData : public AControl
		{
		public:
			DeleteTotalData(std::unordered_set<SafePtr<AGraphNode>> datasToDelete);
			~DeleteTotalData();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			std::unordered_set<SafePtr<AGraphNode>> m_objectsToDelete;
		};

		/*!On a besoin d'un control pour supprimer ce qui est sélectionné dans le model sans avoir besoin d'accéder à celui-ci)
		
		Ce contrôle trie les données séléctionnés entre données importantes(scans) et le reste.
		
		Si il y a une alerte, et que l'utilisateur ne veut pas supprimer les données importantes, alors même les autres données ne sont pas supprimées.
		*/
		class DeleteSelectedElements : public AControl
		{
		public:
			/*!
			
			\param bool supprScan : Si supprScan est vrai, il envoie une alerte si la liste des données importante n'est pas vide pour la suppression de celles ci.
			Sinon, le contrôle ne supprime que les données non importantes. Et il n'y a pas d'alerte.

			*/
			DeleteSelectedElements(bool supprScan);
			~DeleteSelectedElements();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			bool m_supprScan;
		};

		class MultiSelect : public AControl
		{
		public:
			MultiSelect(const std::unordered_set<SafePtr<AGraphNode>>& markers, bool updateTree);
			~MultiSelect();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			const std::unordered_set<SafePtr<AGraphNode>> m_nodesToSelect;
			bool m_updateTree;
		};

		class ShowHideDatas : public AControl
		{
		public:
			ShowHideDatas(std::unordered_set<SafePtr<AGraphNode>> datasToShowHide, bool state, bool isUndoable);
			
			~ShowHideDatas();
			virtual void doFunction(Controller& controller) override;
			virtual bool canUndo() const;
			virtual void undoFunction(Controller& controller) override;
			virtual void redoFunction(Controller& controller) override;
			virtual ControlType getType() const override;

		protected:
			ShowHideDatas();

		protected:
			bool m_state;
			bool m_isUndoable;
			std::unordered_set<SafePtr<AGraphNode>> m_datasToShowHide;
			std::unordered_set<SafePtr<AGraphNode>> m_toUndoDatas;
		};


		class ShowHideObjects : public ShowHideDatas
		{
		public:
			ShowHideObjects(const std::unordered_set<ElementType>& ObjType, const bool& value);
			~ShowHideObjects();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			const std::unordered_set<ElementType> m_types;
		};

		class ShowHideCurrentObjects : public ShowHideDatas
		{
		public:
			ShowHideCurrentObjects(bool state);
			~ShowHideCurrentObjects();
			void doFunction(Controller& controller) override;
			void undoFunction(Controller& controller) override;
			void redoFunction(Controller& controller) override;
			ControlType getType() const override;
		};

		class ShowHideUncurrentObjects : public ShowHideDatas
		{
		public:
			ShowHideUncurrentObjects(bool state);
			~ShowHideUncurrentObjects();
			void doFunction(Controller& controller) override;
			void undoFunction(Controller& controller) override;
			void redoFunction(Controller& controller) override;
			ControlType getType() const override;
		};

		class ShowAll : public ShowHideDatas
		{
		public:
			ShowAll(bool state);
			~ShowAll();
			void doFunction(Controller& controller) override;
			void undoFunction(Controller& controller) override;
			void redoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			std::unordered_map<xg::Guid, bool> m_undoStates;
		};
	}
}

#endif // !CONTROLSPECIAL_H_