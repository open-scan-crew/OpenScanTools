#include "controller/controls/ControlExportPC.h"
#include "controller/Controller.h"
#include "controller/functionSystem/FunctionManager.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/ExportTexts.hpp"


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
            GraphManager& graphManager = controller.getGraphManager();
            std::unordered_set<SafePtr<PointCloudNode>> scans = graphManager.getVisibleScans(tls::ScanGuid());
            if (scans.size() < 2)
            {
                controller.updateInfo(new GuiDataWarning(TEXT_EXPORT_COLOR_BALANCE_NEEDS_MULTIPLE_SCANS));
                return;
            }
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
    }
}
