#ifndef CONTROL_EXPORT_PC_H
#define CONTROL_EXPORT_PC_H

#include "controller/controls/IControl.h"
#include "controller/messages/ClippingExportParametersMessage.h"
#include "controller/messages/SmoothPointsMessage.h"

namespace control
{
    namespace exportPC
    {
        class StartExport : public AControl
        {
        public:
            StartExport(const ExportInitMessage& exportMessage);
            ~StartExport();
            void doFunction(Controller& controller) override;
            bool canUndo() const override;
            void undoFunction(Controller& controller) override;
            ControlType getType() const override;
        private:
            ExportInitMessage m_exportMessage;
        };

        class StartDeletePoints : public AControl
        {
        public:
            StartDeletePoints();
            ~StartDeletePoints();
            void doFunction(Controller& controller) override;
            bool canUndo() const override;
            void undoFunction(Controller& controller) override;
            ControlType getType() const override;
        };

        class StartSmoothPoints : public AControl
        {
        public:
            explicit StartSmoothPoints(const SmoothPointsParameters& parameters);
            ~StartSmoothPoints();
            void doFunction(Controller& controller) override;
            bool canUndo() const override;
            void undoFunction(Controller& controller) override;
            ControlType getType() const override;
        private:
            SmoothPointsMessage m_message;
        };
    }
}

#endif //!CONTROL_EXPORT_H
