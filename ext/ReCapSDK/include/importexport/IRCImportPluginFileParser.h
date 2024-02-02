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

#include <foundation/RCScanProperties.h>
#include <foundation/RCString.h>
#include "RCImportDef.h"

namespace Autodesk { namespace RealityComputing { namespace ImportExport {

    using Autodesk::RealityComputing::Foundation::RCScanProvider;
    using Autodesk::RealityComputing::Foundation::RCString;

    struct PointNode;

    ///\brief Interface of file parser for import plugin
    class RC_IMPORT_API IRCImportPluginFileParser
    {
    public:
        virtual ~IRCImportPluginFileParser() = default;

        ///\brief Open the file and return 'true' if succeeded, 'false' if failed.
        virtual bool openFile(const RCString& fileName) = 0;

        ///\brief Close the file and return 'true' if succeeded, 'false' if failed.
        virtual bool closeFile() = 0;

        ///\brief Multi-scan support interface.
        ///\return Number of scans in the file.  Defaults to 1.
        virtual size_t getScanCount() const = 0;

        ///\brief Simple struct to hold descriptive info about a scan.
        ///       Descriptive scan metaData can be obtained after the file is opened
        struct DescriptiveScanMetadata
        {
            ///\brief Name of the scan.  This will define how the scan is labeled in ReCap applications and on disk once imported.  No extension
            /// required.
            RCString name;

            ///\brief Provider of this codec plugin
            RCScanProvider scanProvider;

            ///\brief original coordinate system name
            RCString sourceCoordinateSystem;

            ///\brief Structured scan flag.
            bool isStructured;

            ///\brief Global transformation of this scan.
            ///\brief This is a row-major 4x4 transformation matrix which transforms points from the local scan coordinate system to their final
            /// position. Example:
            ///   For local point L, this matrix M would transform L to world position W as:
            ///      W = M * L
            double globalTransformation[16];

            //\brief: Enumeration for the convention of the coordinates returned in getNextPoint.
            enum ScanCoordinateSystem
            {
                // Points returned are in global coordinates ('W' above, i.e. globalTransformation has already been applied).
                METADATA_CS_GLOBAL = 0,
                // Points returned are in local coordinates ('L' above).
                METADATA_CS_LOCAL = 1
            };

            ///\brief Coordinate system convention.
            ScanCoordinateSystem csConvention;

            ///\brief Flag denoting that this scan contains per-point RGB information.
            ///\note  This property can be updated during point reading
            bool hasRGB;
            ///\brief Flag denoting that this scan contains per-pixel reflectance/intensity information.
            ///\note  This property can be updated during point reading
            bool hasIntensity;
            ///\brief Flag denoting that this scan contains per-pixel normal information.
            ///\note  This property can be updated during point reading
            bool hasNormals;
            ///\brief Flag denoting that this scan contains per-pixel classification information.
            ///\note       This property can be updated during point reading
            bool hasClassification;
            ///\brief Flag denoting that this scan contains per-pixel time stamp information.
            ///\note  This property has to be set correctly before reading the points from the file
            bool hasTimestamp;
        };

        ///\brief Simple struct to hold statistic info about a scan.
        ///       Statistic scan metaData could be updated after all the points in the file are read
        struct StatisticalScanMetadata
        {
            // *** STRUCTURED SCAN API           ***
            // The below fields hold information about the 'structured' properties of the scan.
            // ReCap defines a 'structured' scan as one which was collected from a fixed position
            // and has an inherent local Cartesian coordinate system.
            // Examples include most commercial terrestrial laser scanner formats (FLS, ZFS, PTX).

            ///\brief start azimuth and end azimuth angle of the scan, in radians.
            double startAzimuth, endAzimuth;
            ///\brief start elevation and end elevation angle of the scan, in radians.
            double startElevation, endElevation;

            ///\brief Approximate angular point spacing along azimuth, in radians.
            double azAngularSpacingRad;
            ///\brief Approximate angular point spacing along elevation, in radians.
            double elAngularSpacingRad;
            ///\brief number of rows of the structured scan
            int numRows;
            ///\brief number of columns of the structured scan
            int numColumns;
        };

        ///\brief Get the descriptive metadata for the specified scan index.
        ///       Descriptive scan metadata should be set after the file is opened.
        ///       Scan format property except hasTimeStamp could be updated during point reading.
        ///\note  This interface will be called for multiple times
        virtual DescriptiveScanMetadata getDescriptiveScanMetadata(const size_t index) const = 0;

        ///\brief Get the statistical metadata for the specified scan index.
        ///      Statistical scan metadata should be updated after all the points in the file have been read
        virtual StatisticalScanMetadata getStatisticalScanMetadata(const size_t index) const = 0;

        ///\brief Begin reading the scan at the specified index.
        virtual bool beginScan(const size_t index) = 0;

        ///\brief Read the next point from the current scan. Return false if no points remain.
        virtual bool readNextPoint(PointNode& pointNode) = 0;

        ///\brief Return the estimated point count for the current scan.
        virtual long long getNumberOfPointsEstimate() = 0;
    };

}}}    // namespace Autodesk::RealityComputing::ImportExport
