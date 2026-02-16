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
        // StartStatisticalOutlierFilter
        // ***************************

        StartStatisticalOutlierFilter::StartStatisticalOutlierFilter()
        {}

        StartStatisticalOutlierFilter::~StartStatisticalOutlierFilter()
        {}

        void StartStatisticalOutlierFilter::doFunction(Controller& controller)
        {
            controller.getFunctionManager().launchFunction(controller, ContextType::statisticalOutlierFilter);
        }

        bool StartStatisticalOutlierFilter::canUndo() const
        {
            return (false);
        }

        void StartStatisticalOutlierFilter::undoFunction(Controller& controller)
        {
        }

        ControlType StartStatisticalOutlierFilter::getType() const
        {
            return (ControlType::startStatisticalOutlierFilter);
        }

        // ***************************
        // StartColorBalanceFilter
        // ***************************

        StartColorBalanceFilter::StartColorBalanceFilter()
        {}

        StartColorBalanceFilter::~StartColorBalanceFilter()
        {}

        void StartColorBalanceFilter::doFunction(Controller& controller)
        {
            controller.getFunctionManager().launchFunction(controller, ContextType::colorBalanceFilter);
        }

        bool StartColorBalanceFilter::canUndo() const
        {
            return (false);
        }

        void StartColorBalanceFilter::undoFunction(Controller& controller)
        {
        }

        ControlType StartColorBalanceFilter::getType() const
        {
            return (ControlType::startColorBalanceFilter);
        }

        // ***************************
        // StartFilteredScansExport
        // ***************************

        StartFilteredScansExport::StartFilteredScansExport()
        {}

        StartFilteredScansExport::~StartFilteredScansExport()
        {}

        void StartFilteredScansExport::doFunction(Controller& controller)
        {
            controller.getFunctionManager().launchFunction(controller, ContextType::filteredScansExport);
        }

        bool StartFilteredScansExport::canUndo() const
        {
            return (false);
        }

        void StartFilteredScansExport::undoFunction(Controller& controller)
        {
        }

        ControlType StartFilteredScansExport::getType() const
        {
            return (ControlType::startFilteredScansExport);
        }
    }
}
