#ifndef SMOOTH_POINT_CLOUD_MESSAGE_H
#define SMOOTH_POINT_CLOUD_MESSAGE_H

#include "controller/messages/IMessage.h"

class SmoothPointCloudMessage : public IMessage
{
public:
    SmoothPointCloudMessage(double maxDisplacementMm, double voxelSizeMm, bool adaptiveVoxel, bool preserveEdges);
    ~SmoothPointCloudMessage() = default;

    IMessage::MessageType getType() const override;
    IMessage* copy() const override;

    double maxDisplacementMm;
    double voxelSizeMm;
    bool adaptiveVoxel;
    bool preserveEdges;
};

#endif
