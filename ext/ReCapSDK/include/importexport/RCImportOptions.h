//////////////////////////////////////////////////////////////////////////////
//
//                     (C) Copyright 2019 by Autodesk, Inc.
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

#include <foundation/RCBuffer.h>
#include <foundation/RCCode.h>
#include <foundation/RCString.h>
#include <foundation/RCMatrix.h>
#include <importexport/IRCImportPluginFileParser.h>
#include <importexport/RCIOStatus.h>
#include <data/RCScan.h>

namespace Autodesk { namespace RealityComputing { namespace Data {

    /// \brief An enum class for the noise filter level.
    enum class RCNoiseFilterLevel
    {
        Minimal    = 0,
        Medium     = 1,
        Aggressive = 2,
        None       = 100
    };

    /// \brief Struct to hold scan file import settings to be used in RCProjectImporter::importFiles(...).
    struct RCImportFilesSettings
    {
        /// \brief A flag indicating whether to enable range clipping.
        bool enableRangeClipping = true;

        /// \brief The minimum clipping range.
        int minClippingRange = -1;

        /// \brief The maximum clipping range.
        int maxClippingRange = -1;

        /// \brief The minimum clipping intensity.
        int minCippingIntensity = -1;

        /// \brief The maximum clipping intensity.
        int maxClippingIntensity = -1;

        /// \brief A flag indicating whether to enable noise filtering.
        bool enableNoiseFilter = true;

        /// \brief The noise filtering level.
        RCNoiseFilterLevel noiseFilterLevel = RCNoiseFilterLevel::Minimal;

        /// \brief The point cloud decimation unit in mm.
        /// \note If it is not set, the default decimation value will be 1.
        int decimation = -1;

        /// \brief A flag indicating whether to unify the scans.
        bool unify = false;

        /// \brief A flag indicating whether to normalize the intensity.
        bool normalizeIntensity = true;

        /// \brief The source coordinate system of the scan data.
        /// \note This value will overwrite the coordinate system
        ///       can be identified from the scan data.
        Foundation::RCString sourceCoordinateSystem;

        /// \brief The target coordinate system that the scan data will be transformed
        ///        to during the import process.
        /// \note This value will be ignored if \p sourceCoordinateSystem is not set
        Foundation::RCString targetCoordinateSystem;

        /// \brief After scans have been imported into project, generate cache files so that the project is ready for realview navigation.
        bool updateCacheFiles = true;
    };

    /// \brief Struct to hold import settings for point clouds in RCProjectImporter::createProjectFromPoints(...) and
    /// RCProjectImporter::createScanFromPoints(...).
    struct RCImportPointCloudSettings
    {
        /// \brief The type of scan.
        RCScanType scanType;

        /// \brief The name given to the generated scan file.
        Foundation::RCString scanName;
    };

    /// \brief Struct to hold the completion status for a scan import
    struct RCScanImportStatus
    {
        /// \brief The name of scan
        Foundation::RCString scanName;

        /// \brief The error code for scan import result
        Foundation::RCCode code;
    };

    /// \brief Struct to hold scan import options to be used in RCProjectImportSession::addScan(...) or RCScanImportSession::init(...).
    struct RCScanImportOptions
    {
        /// \brief Points with range less than minRangeInMeters will be clipped. Leave value as -1 to not set a minimum range.
        double minRangeInMeters = -1;

        /// \brief Points with range more than maxRangeInMeters will be clipped. Leave value as -1 to not set a maximum range.
        double maxRangeInMeters = -1;

        /// \brief Points with intensity less than minIntensity will be clipped. Leave value as -1 to not set a minimum intensity.
        std::int8_t minIntensity = -1;

        /// \brief Points with intensity more than maxIntensity will be clipped. Leave value as -1 to not set a maximum intensity.
        std::int8_t maxIntensity = -1;

        /// \brief The noise filtering level.
        RCNoiseFilterLevel noiseFilterLevel = RCNoiseFilterLevel::Minimal;

        /// \brief The average distance between points in mm in the ReCap rcs file created.
        /// \note  If not set (i.e. left as -1), default distance between points of 1 mm will be used.
        int pointDistanceInMM = -1;

        /// \brief A flag indicating whether to normalize the intensity. Normalizing the intensities will put them in the range [0, 255].
        bool normalizeIntensity = true;

        /// \brief If true, cache files will be generated so that the project is ready for realview navigation after import.
        bool updateCacheFiles = true;

        /// \brief Convert the scan being imported to the given coordinate system.
        Foundation::RCString targetCoordinateSystem = "";
    };

    using RCStructuredScanMetadata = ImportExport::IRCImportPluginFileParser::StatisticalScanMetadata;

    /// \brief Metadata of the scan to be imported
    struct RCScanMetadata
    {
        ///\brief Name of the scan. This will define how the scan is labeled in ReCap applications and on disk once imported. No extension.
        /// required.
        Foundation::RCString name;

        ///\brief Provider of this scan being imported.
        Foundation::RCScanProvider provider;

        /// \brief The type of the scan: stationary or mobile.
        RCScanType type = RCScanType::Stationary;

        ///\brief Specify if the scan is structured or not. Default is false.
        bool isStructured = false;

        ///\brief This is a row-major 4x4 transformation matrix which transforms points from the local scan coordinate system to their final
        ///       position. Default is the identity matrix.
        Foundation::RCMatrix4x4d globalTransformation;

        /// \brief The coordinate System the scan being imported is in, if any.
        Foundation::RCString coordinateSystem;

        /// \brief The estimated point count in the scan.
        /// \note The more accurate this value is, the more accurate the progress reported will be.
        std::uint64_t estimatedPointCount = 0;

        /// \brief The structured scan meta data, not applicable for unstructured scans.
        RCStructuredScanMetadata structuredScanMetadata;
    };

    typedef void (*RCImportProgressCallbackPtr)(const RCIOStatus& status);
    typedef void (*RCImportCompletionCallbackPtr)(Foundation::RCCode errorCode, const Foundation::RCBuffer<RCScanImportStatus>& importStatus);
    typedef void (*RCScanImportCompletionCallbackPtr)(const RCScanImportStatus& importStatus);

}}}    // namespace Autodesk::RealityComputing::Data
