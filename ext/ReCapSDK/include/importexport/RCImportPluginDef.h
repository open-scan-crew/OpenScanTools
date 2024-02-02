//////////////////////////////////////////////////////////////////////////////
//
//                     (C) Copyright 2018 by Autodesk, Inc.
//
// The information contained herein is confidential, proprietary to Autodesk,
// Inc., and considered a trade secret as defined in section 499C of the
// penal code of the State of California.  Use of this information by anyone
// other than authorized employees of Autodesk, Inc. is granted only under a
// written non-disclosure agreement, expressly prescribing the scope and
// manner of such use.
//
//        AUTODESK PROVIDES THIS PROGRAM "AS IS" AND WITH ALL FAULTS.
//        AUTODESK SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTY OF
//        MERCHANTABILITY OR FITNESS FOR A PARTICULAR USE.  AUTODESK, INC.
//        DOES NOT WARRANT THAT THE OPERATION OF THE PROGRAM WILL BE
//        UNINTERRUPTED OR ERROR FREE.
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include <foundation/RCString.h>

namespace Autodesk { namespace RealityComputing { namespace ImportExport {

    using Autodesk::RealityComputing::Foundation::RCString;

    ///\brief Point location
    struct PointLocation
    {
        double x;
        double y;
        double z;
    };

    ///\brief Point normal
    struct PointNormal
    {
        double x;
        double y;
        double z;
    };

    ///\brief Point color
    struct PointColor
    {
        int r;
        int g;
        int b;
        int a;
    };

    ///\brief Intensity
    using Intensity = double;

    ///\brief A node to store all information about a point
    struct PointNode
    {
        PointLocation pos;
        PointNormal normal;
        PointColor color;
        Intensity intensity;
        // Is the position valid?
        bool validPos;
        float azimuth;
        float elevation;
        double timestamp;
        uint8_t classification;
    };

    ///\brief The information of supported file
    struct ImportFileInfo
    {
        ///\brief The extension for a supported file, such as ".oop"
        RCString fileExtension;
        ///\brief The description for a supported file type, such as "Orange Orb File"
        RCString fileTypeDescription;
    };

}}}    // namespace Autodesk::RealityComputing::ImportExport
