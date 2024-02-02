#include "controller/controls/ControlFunction.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/functionSystem/FunctionManager.h"
#include "controller/messages/IMessage.h"

#include "models/3d/Graph/AGraphNode.h"
#include "models/3d/Graph/OpenScanToolsGraphManager.hxx"

#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataTree.h"
#include "utils/Logger.h"

// control::function::

namespace control
{
	namespace function
	{
		/*
		** Abort
		*/

		Abort::Abort()
		{
		}

		Abort::~Abort()
		{
		}

		void Abort::doFunction(Controller& controller)
		{
			controller.getFunctionManager().abort(controller);
			controller.updateInfo(new GuiDataAbort());
			CONTROLLOG << "control::function::Abort" << LOGENDL;
		}

		bool Abort::canUndo() const
		{
			return (false);
		}

		void Abort::undoFunction(Controller& controller)
		{
		}

		ControlType Abort::getType() const
		{
			return (ControlType::functionAbort);
		}

		/*
		** AbortSelection
		*/

		AbortSelection::AbortSelection()
		{
		}

		AbortSelection::~AbortSelection()
		{
		}

		void AbortSelection::doFunction(Controller& controller)
		{
            controller.changeSelection({});
			CONTROLLOG << "control::function::AbortSelection" << LOGENDL;
		}

		bool AbortSelection::canUndo() const
		{
			return (false);
		}

		void AbortSelection::undoFunction(Controller& controller)
		{
		}

		ControlType AbortSelection::getType() const
		{
			return (ControlType::abortSelection);
		}

		/*
		** Validate
		*/

		Validate::Validate()
		{
		}

		Validate::~Validate()
		{
		}

		void Validate::doFunction(Controller& controller)
		{
			controller.getFunctionManager().validate(controller);
			CONTROLLOG << "control::function::Cancel" << LOGENDL;
		}

		bool Validate::canUndo() const
		{
			return (false);
		}

		void Validate::undoFunction(Controller& controller)
		{
		}

		ControlType Validate::getType() const
		{
			return (ControlType::functionCancel);
		}

		/*
		** Forward Message
		*/

		ForwardMessage::ForwardMessage(IMessage* message, const ContextType& type)
			: message(message)
			, m_type(type)
			, m_id(0)
		{}

		ForwardMessage::ForwardMessage(IMessage* message, const uint32_t& id)
			: message(message)
			, m_type(ContextType::none)
			, m_id(id)
		{}

		ForwardMessage::~ForwardMessage() 
		{
			delete message;
		}

		void ForwardMessage::doFunction(Controller& controller)
		{
			if(m_id)
				controller.getFunctionManager().feedMessageToSpecificContext(controller, message, m_id);
			else if(m_type != ContextType::none)
				controller.getFunctionManager().feedMessageToSpecificContext(controller, message, m_type);
			else
			controller.getFunctionManager().feedMessage(controller, message);
		}

		bool ForwardMessage::canUndo() const
		{
			return false;
		}

		void ForwardMessage::undoFunction(Controller& controller) 
		{}

		ControlType ForwardMessage::getType() const
		{
			return (ControlType::forwardMessage);
		}

		AddNodes::AddNodes(const SafePtr<AGraphNode>& nodeToAdd, bool select, bool undo)
			: m_nodesToAdd({ nodeToAdd })
			, m_select(select)
			, m_undo(undo)
		{}

		AddNodes::AddNodes(const std::unordered_set<SafePtr<AGraphNode>>& nodesToAdd, bool select, bool undo)
			: m_nodesToAdd(nodesToAdd)
			, m_select(select)
			, m_undo(undo)
		{}

		AddNodes::~AddNodes()
		{
			for (SafePtr<AGraphNode> nodeToAdd : m_nodesToAdd)
			{
				bool nodeIsDead = false;
				{
					ReadPtr<AGraphNode> rNode = nodeToAdd.cget();
					if (!rNode)
						return;
					nodeIsDead = rNode->isDead();
				}

				if (nodeIsDead)
				{
					AGraphNode::cleanLinks(nodeToAdd);
					nodeToAdd.destroy();
				}
			}
		}

		void AddNodes::doFunction(Controller& controller)
		{
			controller.getOpenScanToolsGraphManager().addNodesToGraph(m_nodesToAdd);
			if(m_select)
				controller.changeSelection(m_nodesToAdd);
			controller.actualizeNodes(ActualizeOptions(true), m_nodesToAdd);
		}

		bool AddNodes::canUndo() const
		{
			return m_undo && !m_nodesToAdd.empty();
		}

		void AddNodes::undoFunction(Controller& controller)
		{
			for (const SafePtr<AGraphNode>& nodeToAdd : m_nodesToAdd)
			{
				WritePtr<AGraphNode> wNode = nodeToAdd.get();
				if (!wNode)
					continue;

				wNode->setDead(true);
			}

			if (m_select)
				controller.changeSelection(m_nodesToAdd);

			controller.actualizeNodes(ActualizeOptions(true), m_nodesToAdd);
		}

		void AddNodes::redoFunction(Controller& controller)
		{
			for (const SafePtr<AGraphNode>& nodeToAdd : m_nodesToAdd)
			{
				WritePtr<AGraphNode> wNode = nodeToAdd.get();
				if (!wNode)
					continue;

				wNode->setDead(false);
			}

			if (m_select)
				controller.changeSelection(m_nodesToAdd);

			controller.actualizeNodes(ActualizeOptions(true), m_nodesToAdd);
		}

		ControlType AddNodes::getType() const
		{
			return ControlType::addNode;
		}
	}
}