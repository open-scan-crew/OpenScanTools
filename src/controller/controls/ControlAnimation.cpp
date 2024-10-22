#include "controller/controls/ControlAnimation.h"
#include "controller/Controller.h"


namespace control
{
	namespace animation
	{
		AddViewPoint::AddViewPoint(SafePtr<AObjectNode> toAdd)
			: m_toAdd(toAdd)
		{}

		void AddViewPoint::doFunction(Controller& controller)
		{
			/*ContextType currentContext(controller.getFunctionManager().isActiveContext());

			

			if (currentContext != ContextType::viewPointAnimation)
			{
				if (currentContext != ContextType::none)
					return;
				controller.getFunctionManager().launchFunction(controller, ContextType::viewPointAnimation);
			}
			
			for (const xg::Guid & id : data)
			{
				Object3D* object = controller.getContext().getCurrentProject()->getObjectOnId<Object3D>(id);
				if (object)
				{
					ClickMessage message(tag->getCenter());
					controller.getFunctionManager().feedMessage(controller, &message);
				}
			}*/
			
		}

		bool AddViewPoint::canUndo() const
		{
			return true;
		}

		void AddViewPoint::undoFunction(Controller& controller)
		{
			//TO Do (Aurélien)
			//send message that remove position form list;
		}

		ControlType AddViewPoint::getType() const
		{
			return ControlType::addAnimationKeyPoint;
		}


		AddScansViewPoint::AddScansViewPoint()
		{}

		AddScansViewPoint::~AddScansViewPoint()
		{}

		void AddScansViewPoint::doFunction(Controller& controller)
		{
			/*
			ContextType currentContext(controller.getFunctionManager().isActiveContext());
			Project* project = controller.getContext().getCurrentProject();
			assert(project);
			std::vector<Scan*> scans(controller.getContext().getCurrentProject()->getObjectsOnType<Scan>(ElementType::Scan));
			struct {
				bool operator()(Scan* a, Scan* b) const { return a->getId() < b->getId(); }
			} dataIdSort;
			std::sort(scans.begin(), scans.end(), dataIdSort);
			for (uint64_t iterator(0); iterator < scans.size(); iterator++)
			{
				ViewPoint animKeyPoint((*scans[iterator]), RenderingParameters());
				controller.updateInfo(new GuiDataRenderAnimationViewPoint(animKeyPoint));
			}
			*/
		}
			
		ControlType AddScansViewPoint::getType() const 
		{
			return ControlType::addScansAnimationKeyPoint;
		}

	} 
}