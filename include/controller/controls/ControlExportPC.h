#ifndef CONTROL_EXPORT_PC_H
#define CONTROL_EXPORT_PC_H

#include "controller/controls/IControl.h"
#include "controller/messages/ClippingExportParametersMessage.h"

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

        class StartStatisticalOutlierFilter : public AControl
        {
        public:
            StartStatisticalOutlierFilter();
            ~StartStatisticalOutlierFilter();
            void doFunction(Controller& controller) override;
            bool canUndo() const override;
            void undoFunction(Controller& controller) override;
            ControlType getType() const override;
        };

        class StartColorBalanceFilter : public AControl
        {
        public:
            StartColorBalanceFilter();
            ~StartColorBalanceFilter();
            void doFunction(Controller& controller) override;
            bool canUndo() const override;
            void undoFunction(Controller& controller) override;
            ControlType getType() const override;
        };

        class StartColorimetricFilterExport : public AControl
        {
        public:
            StartColorimetricFilterExport();
            ~StartColorimetricFilterExport();
            void doFunction(Controller& controller) override;
            bool canUndo() const override;
            void undoFunction(Controller& controller) override;
            ControlType getType() const override;
        };
    }
}

#endif //!CONTROL_EXPORT_H
