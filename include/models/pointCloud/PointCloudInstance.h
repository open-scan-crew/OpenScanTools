#ifndef POINT_CLOUD_INSTANCE_H
#define POINT_CLOUD_INSTANCE_H

#include "models/pointCloud/TLS.h"

namespace tls
{
    struct PointCloudInstance
    {
        ScanHeader header;
        TransformationModule transfo;
        bool isClippable;
    };
}

#endif