#include "controller/controls/ControlExportPC.h"
#include "controller/Controller.h"
#include "controller/functionSystem/FunctionManager.h"
#include "controller/messages/SmoothPointCloudMessage.h"


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
        //     StartSmoothPointCloud    *
        // ***************************

        StartSmoothPointCloud::StartSmoothPointCloud(double maxDisplacementMm, double voxelSizeMm, bool adaptiveVoxel, bool preserveEdges)
            : m_maxDisplacementMm(maxDisplacementMm)
            , m_voxelSizeMm(voxelSizeMm)
            , m_adaptiveVoxel(adaptiveVoxel)
            , m_preserveEdges(preserveEdges)
        {}

        StartSmoothPointCloud::~StartSmoothPointCloud()
        {}

        void StartSmoothPointCloud::doFunction(Controller& controller)
        {
            controller.getFunctionManager().launchFunction(controller, ContextType::smoothPointCloud);
            SmoothPointCloudMessage message(m_maxDisplacementMm, m_voxelSizeMm, m_adaptiveVoxel, m_preserveEdges);
            controller.getFunctionManager().feedMessage(controller, &message);
        }

        bool StartSmoothPointCloud::canUndo() const
        {
            return (false);
        }

        void StartSmoothPointCloud::undoFunction(Controller& controller)
        {
        }

        ControlType StartSmoothPointCloud::getType() const
        {
            return (ControlType::startSmoothPointCloud);
        }
    }
}
