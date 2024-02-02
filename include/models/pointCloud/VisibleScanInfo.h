#ifndef VISIBLE_SCAN_INFO_H
#define VISIBLE_SCAN_INFO_H

#include "models/pointCloud/TLS.h"
#include "models/3d/Graph/TransformationModule.h"


struct VisibleScanInfo
{
    tls::ScanGuid			id;
    TransformationModule	tranfo;
    bool					isPCO;
};

#endif
