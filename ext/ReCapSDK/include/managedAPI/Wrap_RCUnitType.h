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

#include "globals.h"

namespace Autodesk { namespace RealityComputing { namespace Managed {
    public enum class RCUnitType
    {
        Unknown = (int)NS_RCFoundation::RCUnitType::Unknown,
        Meter = (int)NS_RCFoundation::RCUnitType::Meter,
        Foot = (int)NS_RCFoundation::RCUnitType::Foot,
        Inch = (int)NS_RCFoundation::RCUnitType::Inch,
        IFoot = (int)NS_RCFoundation::RCUnitType::IFoot,
        ClarkeFoot = (int)NS_RCFoundation::RCUnitType::ClarkeFoot,
        IInch = (int)NS_RCFoundation::RCUnitType::IInch,
        Centimeter = (int)NS_RCFoundation::RCUnitType::Centimeter,
        Kilometer = (int)NS_RCFoundation::RCUnitType::Kilometer,
        Yard = (int)NS_RCFoundation::RCUnitType::Yard,
        SearsYard = (int)NS_RCFoundation::RCUnitType::SearsYard,
        Mile = (int)NS_RCFoundation::RCUnitType::Mile,
        IYard = (int)NS_RCFoundation::RCUnitType::IYard,
        IMile = (int)NS_RCFoundation::RCUnitType::IMile,
        Knot = (int)NS_RCFoundation::RCUnitType::Knot,
        NautM = (int)NS_RCFoundation::RCUnitType::NautM,
        Lat66 = (int)NS_RCFoundation::RCUnitType::Lat66,
        Lat83 = (int)NS_RCFoundation::RCUnitType::Lat83,
        Decimeter = (int)NS_RCFoundation::RCUnitType::Decimeter,
        Millimeter = (int)NS_RCFoundation::RCUnitType::Millimeter,
        Dekameter = (int)NS_RCFoundation::RCUnitType::Dekameter,
        Hectometer = (int)NS_RCFoundation::RCUnitType::Hectometer,
        GermanMeter = (int)NS_RCFoundation::RCUnitType::GermanMeter,
        CaGrid = (int)NS_RCFoundation::RCUnitType::CaGrid,
        ClarkeChain = (int)NS_RCFoundation::RCUnitType::ClarkeChain,
        GunterChain = (int)NS_RCFoundation::RCUnitType::GunterChain,
        BenoitChain = (int)NS_RCFoundation::RCUnitType::BenoitChain,
        SearsChain = (int)NS_RCFoundation::RCUnitType::SearsChain,
        ClarkeLink = (int)NS_RCFoundation::RCUnitType::ClarkeLink,
        GunterLink = (int)NS_RCFoundation::RCUnitType::GunterLink,
        BenoitLink = (int)NS_RCFoundation::RCUnitType::BenoitLink,
        SearsLink = (int)NS_RCFoundation::RCUnitType::SearsLink,
        Rod = (int)NS_RCFoundation::RCUnitType::Rod,
        Perch = (int)NS_RCFoundation::RCUnitType::Perch,
        Pole = (int)NS_RCFoundation::RCUnitType::Pole,
        Furlong = (int)NS_RCFoundation::RCUnitType::Furlong,
        Rood = (int)NS_RCFoundation::RCUnitType::Rood,
        CapeFoot = (int)NS_RCFoundation::RCUnitType::CapeFoot,
        Brealey = (int)NS_RCFoundation::RCUnitType::Brealey,
        SearsFoot = (int)NS_RCFoundation::RCUnitType::SearsFoot,
        GoldCoastFoot = (int)NS_RCFoundation::RCUnitType::GoldCoastFoot,
        MicroInch = (int)NS_RCFoundation::RCUnitType::MicroInch,
        IndianYard = (int)NS_RCFoundation::RCUnitType::IndianYard,
        IndianFoot = (int)NS_RCFoundation::RCUnitType::IndianFoot,
        IndianFt37 = (int)NS_RCFoundation::RCUnitType::IndianFt37,
        IndianFt62 = (int)NS_RCFoundation::RCUnitType::IndianFt62,
        IndianFt75 = (int)NS_RCFoundation::RCUnitType::IndianFt75,
        IndianYd37 = (int)NS_RCFoundation::RCUnitType::IndianYd37,
        Decameter = (int)NS_RCFoundation::RCUnitType::Decameter,
        InternationalChain = (int)NS_RCFoundation::RCUnitType::InternationalChain,
        InternationalLink = (int)NS_RCFoundation::RCUnitType::InternationalLink,
        Micrometer = (int)NS_RCFoundation::RCUnitType::Micrometer,
        FootAndInch = (int)NS_RCFoundation::RCUnitType::FootAndInch,
        IFootAndInch = (int)NS_RCFoundation::RCUnitType::IFootAndInch,
        Degree = (int)NS_RCFoundation::RCUnitType::Degree,
        Grad = (int)NS_RCFoundation::RCUnitType::Grad,
        Grade = (int)NS_RCFoundation::RCUnitType::Grade,
        MapInfo = (int)NS_RCFoundation::RCUnitType::MapInfo,
        Mil = (int)NS_RCFoundation::RCUnitType::Mil,
        Minute = (int)NS_RCFoundation::RCUnitType::Minute,
        Radian = (int)NS_RCFoundation::RCUnitType::Radian,
        Second = (int)NS_RCFoundation::RCUnitType::Second,
        Decisec = (int)NS_RCFoundation::RCUnitType::Decisec,
        Centisec = (int)NS_RCFoundation::RCUnitType::Centisec,
        Millisec = (int)NS_RCFoundation::RCUnitType::Millisec
    };
}}}    // namespace Autodesk::RealityComputing::Managed