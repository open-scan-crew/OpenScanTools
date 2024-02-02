#ifndef CONTROL_VIEWPORT_H_
#define CONTROL_VIEWPORT_H_

#include "controller/controls/IControl.h"
#include "models/OpenScanToolsModelEssentials.h"
#include "pointCloudEngine/RenderingTypes.h"
#include "models/pointCloud/TLS.h"
#include "models/3d/ClickInfo.h"

#include <vector>
#include <unordered_set>

class IPanel;

class CameraNode;

namespace control
{
    namespace viewport
    {

		class Examine : public AControl
		{
		public:
			Examine(const ClickInfo& info);
            Examine(SafePtr<CameraNode> target);
            ~Examine();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
            const ClickInfo m_clickInfo;
		};

        class ChangeBackgroundColor : public AControl
        {
        public:
            ChangeBackgroundColor();
            ~ChangeBackgroundColor();
            void doFunction(Controller& controller) override;
            bool canUndo() const override;
            void undoFunction(Controller& controller) override;
            ControlType getType() const override;
        };

        class AlignViewSide : public AControl
        {
        public:
            AlignViewSide(AlignView sideToAlign, SafePtr<CameraNode> destCamera);
            ~AlignViewSide();
            void doFunction(Controller& controller) override;
            bool canUndo() const override;
            void undoFunction(Controller& controller) override;
            ControlType getType() const override;

        private:
            AlignView m_side;
            SafePtr<CameraNode> m_destCamera;
        };

        class AlignView2PointsFunction : public AControl
        {
        public:
            AlignView2PointsFunction(/*const xg::Guid& destViewport*/);
            ~AlignView2PointsFunction();
            void doFunction(Controller& controller) override;
            bool canUndo() const override;
            void undoFunction(Controller& controller) override;
            ControlType getType() const override;

        private:
            //const xg::Guid m_destViewport;
        };

        class AlignView3PointsFunction : public AControl
        {
        public:
            AlignView3PointsFunction(/*const xg::Guid& destViewport*/);
            ~AlignView3PointsFunction();
            void doFunction(Controller& controller) override;
            bool canUndo() const override;
            void undoFunction(Controller& controller) override;
            ControlType getType() const override;

        private:
            //const xg::Guid m_destViewport;
        };

        class AlignViewBoxFunction : public AControl
        {
        public:
            AlignViewBoxFunction(/*const xg::Guid& destViewport*/);
            ~AlignViewBoxFunction();
            void doFunction(Controller& controller) override;
            bool canUndo() const override;
            void undoFunction(Controller& controller) override;
            ControlType getType() const override;

        private:
           //const xg::Guid m_destViewport;
        };


        class MoveManipFunction : public AControl
        {
        public:
            MoveManipFunction();
            ~MoveManipFunction();
            void doFunction(Controller& controller) override;
            ControlType getType() const override;
        };
    }
}

#endif // _CONTROL_VIEWPORT_H_