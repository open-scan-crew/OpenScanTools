#ifndef CONTROL_FUNCTION_H_
#define CONTROL_FUNCTION_H_

#include <glm/glm.hpp>
#include "controller/controls/IControl.h"
#include "models/OpenScanToolsModelEssentials.h"

#include <unordered_set>

class IMessage;
class AGraphNode;
enum class ContextType;

namespace control
{
	namespace function
	{
		class Abort : public AControl
		{
		public:
			Abort();
			~Abort();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		};

		class AbortSelection : public AControl
		{
		public:
			AbortSelection();
			~AbortSelection();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		};

		class Validate : public AControl
		{
		public:
			Validate();
			~Validate();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		};

		class ForwardMessage : public AControl
		{
		public:
			ForwardMessage(IMessage* message, const ContextType& type);
			ForwardMessage(IMessage* message, const uint32_t& id = 0);
			~ForwardMessage();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			IMessage* message;
			const ContextType m_type;
			const uint32_t m_id;
		};

		class AddNodes : public AControl
		{
		public:
			AddNodes(const SafePtr<AGraphNode>& nodeToAdd, bool select = true, bool undo = true);
			AddNodes(const std::unordered_set<SafePtr<AGraphNode>>& nodesToAdd, bool select = true, bool undo = true);
			~AddNodes();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			void redoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			std::unordered_set<SafePtr<AGraphNode>> m_nodesToAdd;
			bool m_select;
			bool m_undo;
		};
	}
}

#endif // !CONTROL_FUNCTION_H_