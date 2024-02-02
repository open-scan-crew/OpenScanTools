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

#include "managedAPI/Wrap_RCCode.h"
#include "managedAPI/Wrap_RCProjectLoadAttributes.h"
#include "managedAPI/Wrap_RCBox.h"
#include "managedAPI/Wrap_RCVector.h"
#include "managedAPI/Wrap_RCPointIteratorSettings.h"
#include "managedAPI/Wrap_RCPoint.h"
#include "managedAPI/Wrap_RCStructuredScan.h"

#include "RCScopedPointer.h"
#include "managedAPI/Wrap_RCTransform.h"

namespace Autodesk { namespace RealityComputing { namespace Managed {
    public enum class RCScanType
    {
        Stationary,
        Mobile
    };

    public ref class RCScan
    {
    public:
        RCScan() {}
        RCScan(const NS_RCFoundation::RCSharedPtr<NS_RCData::RCScan>& scanPtr);

    public:
        /// \brief loads one RCS file into a scan.
        /// \param filePath the path to the scan to be loaded
        /// \param accessMode the file open mode
        /// \return the RCCode result of the load operation
        RCCode LoadFile(String^ filePath, RCFileAccess accessMode);

        /// \brief get the number of points in this scan
        /// \return The point count
        UInt64 GetNumberOfPoints();

        /// \brief  Get the associated structured scan
        /// \return The associated structured scan.
        /// \brief Get the associated structured scan
        RCStructuredScan^ GetStructuredScan();

        /// \brief Read the points by creating a point iterator used to iterate through all points in this scan.
        /// \param settings Settings for how to create the point iterator.
        /// \return A list of points.
        System::Collections::Generic::List<RCPoint>^ GetPoints(RCPointIteratorSettings^ rc_settings);

        //////////////////////////////////////////////////////////////////////////
        /// \brief Read the points by creating a batch-by-batch point iterator used to iterate through all points in this scan.
        /// \param settings Settings for how to create the point iterator.
        /// \return A list of points.        
        System::Collections::Generic::List<RCPoint>^ GetPointsByBatchIteration(RCPointIteratorSettings^ rc_settings);

        //////////////////////////////////////////////////////////////////////////
        /// \brief Read the points by creating a batch-by-batch point iterator used to iterate through all points in this scan.
        /// \param settings Settings for how to create the point iterator.
        /// \param batchSize Custom size of batch.
        /// \return A list of points.
        System::Collections::Generic::List<RCPoint>^ GetPointsByBatchIteration(RCPointIteratorSettings^ rc_settings, unsigned long batchSize);

        //////////////////////////////////////////////////////////////////////////
        /// \brief Modifies the point in the scan.
        /// \param point Wrapped modified point to be changed in ReCap scan
        /// \param settings Settings for how to create the point iterator. Density should be 0.0
        /// \return True if the point modified successfully, otherwise - false. If the scan is opened in ReadOnly file access mode it will return false.
        bool ModifyPoint(RCPoint% point, RCPointIteratorSettings^ settings);

        /// \brief Return the text name for this scan.
        String^ GetName();

        /// \brief Return the file path for this scan.
        String^ GetFilePath();

        /// \brief Return the group ID for this scan.
        Int32 GetGroupId();

        /// \brief Return the group name for this scan.
        String^ GetGroupName();

        /// \brief Return the GUID for this scan.
        String^ GetScanId();

        /// \brief Return the scale of this scan, independent in each axis.
        RCVector3d GetScale();

        /// \brief Return the axis-aligned bounding box for this scan.
        RCBox GetBoundingBox();

        /// \brief Return the position of the scanner.
        RCVector3d GetOrigin();

        /// \brief Return the maximum intensity value.
        float GetIntensityMaxValue();

        /// \brief Return the minimum intensity value.
        float GetIntensityMinValue();

        /// \brief Return the scan provider in text format.
        String^ GetScanProvider();

        /// \brief Return whether this scan is a structured scan.
        ///        If the .rcs file is created from a structured raw scan but .rcc file is missing,
        ///        it will return false
        bool IsStructured();

        /// \brief Return the type of scan.
        /// \return RCScanType enum with values Stationary or Mobile.
        RCScanType GetType();

        /// \brief Return whether this scan has normal data.
        bool HasNormals();

        /// \brief Return whether this scan has color data.
        bool HasColors();

        /// \brief Return whether this scan has intensity data.
        bool HasIntensities();

        /// \brief Return whether this scan has segment info.
        bool HasSegmentInfo();

        /// \brief Return whether this scan has timeStamp info.
        bool HasTimeStamp();

        /// \brief Get the transform from scan to the registration group
        /// \param scanToGroupTransform The transform from scan to the registration group
        /// \return True if getting scan to registration group transform correctly. False otherwise.
        bool GetScanToGroupTransform(RCTransform% scanToGroupTransform);

        /// \brief Get the final scan transform
        /// \note If the scan is loaded as part of a project, 
        ///       full Transform is the result of UCSTransform * GroupToSurveyTransform * ScanToGroupTransform
        ///       If the scan is loaded not as part of a project, full transform is the single scan transform
        /// \param fullTransform The final transformation of the scan
        /// \return True if getting final scan transform correctly. False otherwise.
        bool GetFullTransform(RCTransform% fullTransform);

        /// \brief Set the final scan transform
        /// \note  If the scan is loaded  not as part of a project, the new transform information won't be written into .rcs file
        /// \param fullTransform The final transformation of the scan to set
        /// \return True if setting final scan transform correctly. False otherwise.
        bool SetFullTransform(RCTransform% fullTransform);

        /// \brief Return the width of the structured spherical image
        size_t GetStructuredScanWidth();

        /// \brief Return the height of the structured spherical image
        size_t GetStructuredScanHeight();

        /// \brief Return azimuth start of the structured scan
        double GetStructuredScanAzimuthStart();

        /// \brief Return azimuth end of the structured scan
        double GetStructuredScanAzimuthEnd();

        /// \brief Return elevation start of the structured scan
        double GetStructuredScanElevationStart();

        /// \brief Return elevation end of the structured scan
        double GetStructuredScanElevationEnd();

        /// \brief Get the estimated number of points in this scan given the input point iterator setting.
        ///        This method can be used to quickly get a rough idea on how many points could be fetched 
        ///        for the given setting without loading the full scan.
        /// \param setting The setting to describe the options to traverse the points in this scan
        /// \return The estimated number of points
        UInt64 GetNumberOfPointsEstimate(RCPointIteratorSettings settings);

    private:
        RCScopedPointer<NS_RCData::RCScan> mScan;
    };
}}}    // namespace Autodesk::RealityComputing::Managed
