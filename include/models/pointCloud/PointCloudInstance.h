#ifndef POINT_CLOUD_INSTANCE_H
#define POINT_CLOUD_INSTANCE_H

#include "tls_def.h"
#include "models/graph/TransformationModule.h"

#include <string>

namespace tls
{
    struct PointCloudInstance
    {
        ScanHeader header;
        TransformationModule transfo;
        bool isClippable;
        std::wstring phase;
    };
}

#endif
