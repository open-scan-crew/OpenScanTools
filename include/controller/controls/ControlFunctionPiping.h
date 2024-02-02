#ifndef CONTROL_FUNCTION_PIPING_H_
#define CONTROL_FUNCTION_PIPING_H_

#include "controller/controls/IControl.h"
#include "models/OpenScanToolsModelEssentials.h"
#include <list>
#include "models/data/Data.h"
#include "glm/glm.hpp"

class PipingNode;
class AGraphNode;

namespace control
{
	namespace function
	{
		namespace piping
		{
			class ConnectToPiping : public AControl
			{
			public:
				ConnectToPiping(SafePtr<PipingNode> piping, const std::unordered_set<SafePtr<AGraphNode>>& elements);
				~ConnectToPiping();
				void doFunction(Controller& controller) override;
				bool canUndo() const override;
				void undoFunction(Controller& controller) override;
				ControlType getType() const override;
			private:
				SafePtr<PipingNode> m_piping;
				std::unordered_set<SafePtr<AGraphNode>> m_elements;
			};
		}
	}
}

#endif // !CONTROL_FUNCTION_PIPING_H_