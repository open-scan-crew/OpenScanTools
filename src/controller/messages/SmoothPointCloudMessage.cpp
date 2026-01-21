#include "controller/messages/SmoothPointCloudMessage.h"

SmoothPointCloudMessage::SmoothPointCloudMessage(double maxDisplacementMm, double voxelSizeMm, bool adaptiveVoxel, bool preserveEdges)
    : maxDisplacementMm(maxDisplacementMm)
    , voxelSizeMm(voxelSizeMm)
    , adaptiveVoxel(adaptiveVoxel)
    , preserveEdges(preserveEdges)
{
}

IMessage::MessageType SmoothPointCloudMessage::getType() const
{
    return IMessage::MessageType::SMOOTH_POINT_CLOUD_PARAMETERS;
}

IMessage* SmoothPointCloudMessage::copy() const
{
    return new SmoothPointCloudMessage(*this);
}
