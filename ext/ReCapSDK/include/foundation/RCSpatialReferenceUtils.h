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

#include <foundation/RCSpatialRefDef.h>

#include <foundation/RCCode.h>
#include <foundation/RCString.h>
#include <foundation/RCUnitService.h>

namespace Autodesk { namespace RealityComputing { namespace Foundation {

    enum class RCUnitType;

#if defined(rcSpatialReferenceUtils)
#undef rcSpatialReferenceUtils
#endif
    /// \brief A macro that returns the RCSpatialReferenceUtils singleton.
#define rcSpatialReferenceUtils RCSpatialReferenceUtils::getInstance()

    using Autodesk::RealityComputing::Foundation::RCCode;
    using Autodesk::RealityComputing::Foundation::RCString;
    using Autodesk::RealityComputing::Foundation::RCUnitType;

    class RC_SPATIALREF_API RCSpatialReferenceUtils
    {
    public:
        static RCSpatialReferenceUtils& getInstance();

        RCCode getCoordinateSystemInfo(const RCString& cs, bool& isGeographic, bool& isLinearUnit, double& unitScale);
        RCCode isValidCoordinateSystem(const RCString& cs);
        double getUnitInMeters(RCUnitType unit);
        RCUnitType getUnitType(const RCString& cs);
        void getUnitIdentifier(RCUnitType unit, RCString& unitString);
        RCString getADSKCoordinateSystemCode(const RCString& epsg);
        RCString getEPSGCoordinateSystemCode(const RCString& adsk);
        double getGeoUnitScale(int linearUnitsGeoKey);
        int toMentorUnitCode(int linearUnitsGeoKey);

    private:
        RCSpatialReferenceUtils();
    };

}}}    // namespace Autodesk::RealityComputing::Foundation
