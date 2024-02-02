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

namespace Autodesk { namespace RealityComputing { namespace Managed {
    public ref class RCPointIteratorSettings
    {
    public:
        RCPointIteratorSettings() {}
        RCPointIteratorSettings(bool isReadOnly, bool isVisiblePointsOnly, double density) : _readOnly(isReadOnly), _visiblePointsOnly(isVisiblePointsOnly), _density(density) {}
    public:
        property bool IsReadOnly { bool get() { return _readOnly; } void set(bool ro) { _readOnly = ro; } };
        property bool IsVisiblePointsOnly { bool get() { return _visiblePointsOnly; } void set(bool vpo) { _visiblePointsOnly = vpo; } };
        property double Density { double get() { return _density; } void set(double d) { _density = d; } };
    private:
        bool _readOnly = true;
        bool _visiblePointsOnly = false;
        double _density = -1.0;
    };
}}}    // namespace Autodesk::RealityComputing::Managed
