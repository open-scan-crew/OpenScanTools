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

        class StartSmoothPointCloud : public AControl
        {
        public:
            StartSmoothPointCloud(double maxDisplacementMm, double voxelSizeMm, bool adaptiveVoxel, bool preserveEdges);
            ~StartSmoothPointCloud();
            void doFunction(Controller& controller) override;
            bool canUndo() const override;
            void undoFunction(Controller& controller) override;
            ControlType getType() const override;
        private:
            double m_maxDisplacementMm;
            double m_voxelSizeMm;
            bool m_adaptiveVoxel;
            bool m_preserveEdges;
        };
    }
}

#endif //!CONTROL_EXPORT_H
