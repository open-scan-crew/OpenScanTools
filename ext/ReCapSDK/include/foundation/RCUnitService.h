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

#include <foundation/RCCommonDef.h>

namespace Autodesk { namespace RealityComputing { namespace Foundation {
    class RCString;

    enum class RCUnitType : int
    {
        Unknown            = 0,
        Meter              = 1,
        Foot               = 2,
        Inch               = 3,
        IFoot              = 4,
        ClarkeFoot         = 5,
        IInch              = 6,
        Centimeter         = 7,
        Kilometer          = 8,
        Yard               = 9,
        SearsYard          = 10,
        Mile               = 11,
        IYard              = 12,
        IMile              = 13,
        Knot               = 14,
        NautM              = 15,
        Lat66              = 16,
        Lat83              = 17,
        Decimeter          = 18,
        Millimeter         = 19,
        Dekameter          = 20,
        Hectometer         = 21,
        GermanMeter        = 22,
        CaGrid             = 23,
        ClarkeChain        = 24,
        GunterChain        = 25,
        BenoitChain        = 26,
        SearsChain         = 27,
        ClarkeLink         = 28,
        GunterLink         = 29,
        BenoitLink         = 30,
        SearsLink          = 31,
        Rod                = 32,
        Perch              = 33,
        Pole               = 34,
        Furlong            = 35,
        Rood               = 36,
        CapeFoot           = 37,
        Brealey            = 38,
        SearsFoot          = 39,
        GoldCoastFoot      = 40,
        MicroInch          = 41,
        IndianYard         = 42,
        IndianFoot         = 43,
        IndianFt37         = 44,
        IndianFt62         = 45,
        IndianFt75         = 46,
        IndianYd37         = 47,
        Decameter          = 48,
        InternationalChain = 49,
        InternationalLink  = 50,
        Micrometer         = 51,
        FootAndInch        = 101,
        IFootAndInch       = 102,
        Degree             = 1001,
        Grad               = 1002,
        Grade              = 1003,
        MapInfo            = 1004,
        Mil                = 1005,
        Minute             = 1006,
        Radian             = 1007,
        Second             = 1008,
        Decisec            = 1009,
        Centisec           = 1010,
        Millisec           = 1011
    };

    const double I_MILE_FACTOR     = 0.000621371192;
    const double I_FOOT_FACTOR     = 3.280839895013;
    const double I_INCH_FACTOR     = 39.37007874016;
    const double MILE_FACTOR       = 0.00062136994937697;
    const double FOOT_FACTOR       = 3.2808333333333;
    const double INCH_FACTOR       = 39.37;
    const double METER_FACTOR      = 1.0;
    const double MILLIMETER_FACTOR = 1000.0;
    const double CENTIMETER_FACTOR = 100.0;
    const double KILOMETER_FACTOR  = 0.001;
    const double INTERN_US_FACTOR  = 0.9999980000000274;
    const double MICROMETER_FACTOR = 1000000.0;

    class RC_COMMON_API RCUnitService
    {
    public:
        RCUnitService();
        ~RCUnitService();

        //\brief: get current unit
        static RCUnitType getUnitType();

        //\brief: set current unit
        static void setUnitType(RCUnitType unitType);

        //\brief: get current unit's scale based on meter
        static double getUnitScale(RCUnitType unitType = RCUnitType::Unknown);
        static bool findUnitScaleAndType(const RCString& unitString, double& unitScale, RCUnitType& unitType);

        static void getUSUnitType(const RCUnitType intUnitType, const bool isSurveyMode, RCUnitType& usUnitType);
        static void getInternationalUnitType(const RCUnitType usUnitType, bool& isSurveyMode, RCUnitType& intUnitType);
        static bool isImperial(const RCUnitType unitType);

        static int getUnitPrecision();
        static void setUnitPrecision(int precision);

        //\brief: get description of unit
        static RCString getUnitDescription(RCUnitType unitType = RCUnitType::Unknown, int dimension = 1);

        static bool getUnitAdapted();
        static void setUnitAdapted(bool toAdapted);

        //\brief: get display string with unit adapted to nearest sub-unit.
        // value is in meter!
        static RCString getDisplayStringWithMeter(double value, int precision);

        //\brief: get display string with unit adapted to nearest sub-unit.
        // value is in current unit, not necessarily in meter!
        static RCString getAdaptedDisplayStringWithUnit(const double& value, const int& precision, RCUnitType& resultUnitType, const bool& includeUnit = true);

        //\brief: get display string with unit. value is in meter! parameter
        // originValue is used to convert the float value to int value
        static RCString getDisplayStringInFeetAndInch(const double& value, const int& precision, bool isUSType, bool originInchValue = false);

        //\brief: set localization string for type description, only used for
        // initialization
        static void setTypeString(const RCUnitType unitType, RCString description);

        static RCString getDisplayString(const double& value);
        static RCString getUnitDisplayString();

        static double convertUnits(double inputVal, RCUnitType inputType, RCUnitType outputType = RCUnitType::Unknown, unsigned short dimensions = 1);
    };

}}}    // namespace Autodesk::RealityComputing::Foundation
