#ifndef POINT_CLOUD_INSTANCE_H
#define POINT_CLOUD_INSTANCE_H

#include "models/pointCloud/TLS.h"
#include "models/graph/APointCloudNode.h"
#include "utils/safe_ptr.h"

namespace tls
{
    struct PointCloudInstance
    {
        SafePtr<APointCloudNode> scanNode;
        ScanHeader header;
        TransformationModule transfo;
        bool isClippable;
    };
}

#endif