
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

#include <foundation/RCCode.h>
#include <foundation/RCSharedPtr.h>
#include <foundation/RCBox.h>
#include <foundation/RCBuffer.h>
#include <foundation/RCString.h>
#include <foundation/RCTransform.h>
#include <data/RCPointBuffer.h>
#include <importexport/RCIOStatus.h>
#include <data/RCProjectIODef.h>
#include <data/RCProjectLoadAttributes.h>

namespace Autodesk { namespace RealityComputing { namespace Data {

    class IRSPointChunk;
    class IRCPointIterator;
    class IRCPointBatchIterator;
    class RCPointIteratorSettings;
    class RCProject;
    class RCStructuredScan;

    using Autodesk::RealityComputing::Foundation::RCBox;
    using Autodesk::RealityComputing::Foundation::RCSharedPtr;
    using Autodesk::RealityComputing::Foundation::RCString;
    using Autodesk::RealityComputing::Foundation::RCTransform;
    using Autodesk::RealityComputing::Foundation::RCVector3d;

    enum class RCScanType
    {
        Stationary,
        Mobile
    };

    class RC_PROJECTIO_API RCScan final
    {
        friend class RCStructuredScan;

    public:
        RCScan(RCProject* pProject, int index, const RCFileAccess& accessMode = RCFileAccess::ReadWrite);
        virtual ~RCScan();

        /// \brief loads one RCS file into a scan.
        /// \param rcsPath the path to the scan to be loaded
        /// \param accessMode the file open mode
        /// \param code the RCCode result of the load operation
        /// \return a nullptr if the load fails, a pointer to a valid RCScan object if the load succeeds
        static RCSharedPtr<RCScan> loadFile(const RCString& rcsPath, const RCFileAccess& accessMode, Foundation::RCCode& code);

        /// \brief get the number of points in this scan
        /// \return The point count
        uint64_t getNumberOfPoints() const;

        /// \brief  Get the associated structured scan
        /// \return The associated structured scan. The life time of this pointer is managed by the owning
        ///         RCScan object, it should not be referenced after the owning RCScan object is destroyed.
        /// \brief Get the associated structured scan
        RCSharedPtr<RCStructuredScan> getStructuredScan();

        /// \brief Create the point iterator to iterate through all points.
        /// \param settings Settings for how to create the point iterator.
        /// \return A new instance of a point iterator.
        RCSharedPtr<IRCPointIterator> createPointIterator(const RCPointIteratorSettings& settings);

        /// \brief Create a batch-by-batch point iterator to iterate through all points in batches in this scan.
        /// \param settings Settings for how to create the batch-by-batch point iterator.
        /// \return A new instance of a batch-by-batch point iterator.
        RCSharedPtr<IRCPointBatchIterator> createPointBatchIterator(const RCPointIteratorSettings& settings);

        /// \brief Return the text name for this scan.
        RCString getName() const;

        /// \brief Return the file path for this scan.
        RCString getFilePath() const;

        /// \brief Return the group ID for this scan.
        int getGroupId() const;

        /// \brief Return the group name for this scan.
        RCString getGroupName() const;

        /// \brief Return the GUID for this scan.
        RCString getScanId() const;

        /// \brief Return the scale of this scan, independent in each axis.
        RCVector3d getScale() const;

        /// \brief Return the axis-aligned bounding box for this scan.
        RCBox getBoundingBox() const;

        /// \brief Return the position of the scanner.
        RCVector3d getOrigin() const;

        /// \brief Return the maximum intensity value.
        float getIntensityMaxValue() const;

        /// \brief Return the minimum intensity value.
        float getIntensityMinValue() const;

        /// \brief Return the scan provider in text format.
        RCString getScanProvider() const;

        /// \brief Return whether this scan is a structured scan.
        ///        If the .rcs file is created from a structured raw scan but .rcc file is missing,
        ///        it will return false
        bool isStructured() const;

        /// \brief Return the type of scan.
        /// \return RCScanType enum with values Stationary or Mobile.
        RCScanType getType() const;

        /// \brief Return whether this scan has normal data.
        bool hasNormals() const;

        /// \brief Return whether this scan has color data.
        bool hasColors() const;

        /// \brief Return whether this scan has intensity data.
        bool hasIntensities() const;

        /// \brief Return whether this scan has segment info.
        bool hasSegmentInfo() const;

        /// \brief Return whether this scan has timeStamp info.
        bool hasTimeStamp() const;

        /// \brief Get the transform from scan to the registration group
        /// \param scanToGroupTransform The transform from scan to the registration group
        /// \return True if getting scan to registration group transform correctly. False otherwise.
        bool getScanToGroupTransform(RCTransform& scanToGroupTransform) const;

        /// \brief Get the final scan transform
        /// \note If the scan is loaded as part of a project, 
        ///       full Transform is the result of UCSTransform * GroupToSurveyTransform * ScanToGroupTransform
        ///       If the scan is loaded not as part of a project, full transform is the single scan transform
        /// \param fullTransform The final transformation of the scan
        /// \return True if getting final scan transform correctly. False otherwise.
        bool getFullTransform(RCTransform& fullTransform) const;

        /// \brief Set the final scan transform
        /// \note  If the scan is loaded  not as part of a project, the new transform information won't be written into .rcs file
        /// \param fullTransform The final transformation of the scan to set
        /// \return True if setting final scan transform correctly. False otherwise.
        bool setFullTransform(const RCTransform& fullTransform);

        /// \brief Return the width of the structured spherical image
        size_t getStructuredScanWidth() const;

        /// \brief Return the height of the structured spherical image
        size_t getStructuredScanHeight() const;

        /// \brief Return azimuth start of the structured scan
        double getStructuredScanAzimuthStart() const;

        /// \brief Return azimuth end of the structured scan
        double getStructuredScanAzimuthEnd() const;

        /// \brief Return elevation start of the structured scan
        double getStructuredScanElevationStart() const;

        /// \brief Return elevation end of the structured scan
        double getStructuredScanElevationEnd() const;

        /// \brief Get the estimated number of points in this scan given the input point iterator setting.
        ///        This method can be used to quickly get a rough idea on how many points could be fetched 
        ///        for the given setting without loading the full scan.
        /// \param setting The setting to describe the options to traverse the points in this scan
        /// \return The estimated number of points
        uint64_t getNumberOfPointsEstimate(RCPointIteratorSettings& settings) const;

        /// \brief Get the original coordinate system that is embedded in the RCS file
        RCString getOriginalCoordinateSystem() const;

    private:
        class Impl;
        Impl* const mImpl = nullptr;

        RCScan(Impl* impl);
    };
}}}    // namespace Autodesk::RealityComputing::Data
