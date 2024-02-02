#ifndef CONTROL_FUNCTION_MEASURE_H_
#define CONTROL_FUNCTION_MEASURE_H_

#include "controller/controls/IControl.h"
#include "controller/functionSystem/AContext.h"
#include "models/OpenScanToolsModelEssentials.h"
#include "models/3d/Measures.h"

class PolylineMeasureNode;

namespace control
{
	namespace function
	{
		namespace measure
		{
			class AddMeasureToPolylineMeasure : public AControl
			{
			public:
				AddMeasureToPolylineMeasure(SafePtr<PolylineMeasureNode> polyline, const Measure& measure, ContextId contextId);
				~AddMeasureToPolylineMeasure();
				void doFunction(Controller& controller) override;
				bool canUndo() const override;
				void undoFunction(Controller& controller) override;
				ControlType getType() const override;
			private:
				SafePtr<PolylineMeasureNode> m_polyline;
				const Measure	m_measure;
				ContextId m_contextId;
				bool m_isRedo;
			};
		}
	}
}

#endif // !CONTROL_FUNCTION_MEASURE_H_