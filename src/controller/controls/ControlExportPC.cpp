#include "controller/controls/ControlExportPC.h"
#include "controller/Controller.h"
#include "controller/functionSystem/FunctionManager.h"


namespace control
{
    namespace exportPC
    {
        // ***************************
        //     StartExport
        // ***************************

        StartExport::StartExport(const ExportInitMessage& exportMessage)
            : m_exportMessage(exportMessage)
        {
        }

        StartExport::~StartExport()
        {}

        void StartExport::doFunction(Controller& controller)
        {
            controller.getFunctionManager().launchFunction(controller, ContextType::exportPC);
            controller.getFunctionManager().feedMessage(controller, &m_exportMessage);
        }

        bool StartExport::canUndo() const
        {
            return (false);
        }

        void StartExport::undoFunction(Controller& controller)
        {
        }

        ControlType StartExport::getType() const
        {
            return (ControlType::startExport);
        }

        // ***************************
        //     StartDeletePoints    *
        // ***************************

        StartDeletePoints::StartDeletePoints()
        {}

        StartDeletePoints::~StartDeletePoints()
        {}

        void StartDeletePoints::doFunction(Controller& controller)
        {
            controller.getFunctionManager().launchFunction(controller, ContextType::deletePoints);
        }

        bool StartDeletePoints::canUndo() const
        {
            return (false);
        }

        void StartDeletePoints::undoFunction(Controller& controller)
        {
        }

        ControlType StartDeletePoints::getType() const
        {
            return (ControlType::startDeletePoints);
        }

        // ***************************
        //     StartSmoothPoints
        // ***************************

        StartSmoothPoints::StartSmoothPoints(const SmoothPointsParameters& parameters)
            : m_message(parameters)
        {}

        StartSmoothPoints::~StartSmoothPoints()
        {}

        void StartSmoothPoints::doFunction(Controller& controller)
        {
            controller.getFunctionManager().launchFunction(controller, ContextType::smoothPoints);
            controller.getFunctionManager().feedMessage(controller, &m_message);
        }

        bool StartSmoothPoints::canUndo() const
        {
            return (false);
        }

        void StartSmoothPoints::undoFunction(Controller& controller)
        {
        }

        ControlType StartSmoothPoints::getType() const
        {
            return (ControlType::startSmoothPoints);
        }
    }
}
