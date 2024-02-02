//////////////////////////////////////////////////////////////////////////////
//
//                     (C) Copyright 2020 by Autodesk, Inc.
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

#include "managedAPI/Wrap_RCVector.h"
#include "globals.h"

namespace Autodesk { namespace RealityComputing { namespace Managed {
    public value class RCPoint
    {
    public:
        RCPoint(const NS_RCData::IRCPointAccessor% recapPoint)
        {
            Position = RCVector3d(recapPoint.getPosition());
            Normal = RCVector3d(recapPoint.getNormal());
            Color = RCColor(recapPoint.getColor());
            Deleted = recapPoint.isDeleted();
            ReadOnly = recapPoint.isReadOnly();
            HasClassification = recapPoint.hasClassification();
            if (HasClassification)
                Classification = recapPoint.getClassification();
            else
                Classification = 0;
            HasItensity = recapPoint.hasIntensity();
            if (HasItensity)
                Intensity = recapPoint.getIntensity();
            else
                Intensity = 0.0f;
            NormalizedIntensity = recapPoint.getNormalizedIntensity();
            Region = recapPoint.getRegion();
        }
    public:
        property RCVector3d Position;
        property RCVector3d Normal;
        property RCColor Color;
        property bool Deleted;
        property bool ReadOnly;
        property uint8_t Classification;
        property float Intensity;
        property uint8_t NormalizedIntensity;
        property int Region;
        property bool HasItensity;
        property bool HasClassification;
        property unsigned long long Index;
    public:
        virtual System::String^ ToString() override
        {
            System::String^ result = gcnew System::String("Point info:\n");
            result += "Position:            " + Position.ToString() + "\n";
            result += "Normal:              " + Normal.ToString() + "\n";
            result += "Color:               " + Color.ToString() + "\n";
            result += "Deleted:             " + (Deleted ? "Yes" : "No") + "\n";
            result += "Read Only:           " + (ReadOnly ? "Yes" : "No") + "\n";
            result += "Classification:      " + (HasClassification ? Classification.ToString() : "Has no classification") + "\n";
            result += "Intensity:           " + (HasItensity ? Intensity.ToString() : "Has no itensity") + "\n";
            result += "NormalizedIntensity: " + NormalizedIntensity + "\n";
            result += "Region:              " + Region + "\n";
            result += "Index                " + Index + "\n";
            return result;
        }
    };
}}}    // namespace Autodesk::RealityComputing::Managed