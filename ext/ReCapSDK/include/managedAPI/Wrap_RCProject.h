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

#include <string>
#include <cliext/list>

#include "globals.h"

#include "managedAPI/Wrap_RCUnitType.h"
#include "managedAPI/Wrap_RCPoint.h"
#include "managedAPI/Wrap_RCPointIteratorSettings.h"
#include "managedAPI/Wrap_RCBox.h"
#include "managedAPI/Wrap_RCCode.h"
#include "managedAPI/Wrap_RCProjectLoadAttributes.h"
#include "managedAPI/Wrap_RCScan.h"
#include "managedAPI/Wrap_RCTransform.h"
#include "managedAPI/Wrap_RCSpatialFilter.h"

#include "RCScopedPointer.h"

//using namespace Autodesk::Revit;
//using namespace Autodesk::Revit::DB;


namespace Autodesk { namespace RealityComputing { namespace Managed {
    public ref class RCProject
    {
    public:
        /// \brief Open a given ReCap project using open mode \p fileAccess.
        /// \param projectFilePath The path of the rcp project file
        /// \param fileAccess The file open mode
        /// \param userEdits Specify the type of user edits to load.
        /// \param code RCCode for project loading
        /// \return An instance of RCProject if the file is successfully loaded, or
        ///         nullptr otherwise.
        RCCode LoadFromFile(String^ filename, RCFileAccess accessOption, RCProjectUserEdits userEdits);

        RCProject() {}
        RCProject(NS_RCData::RCProject * project);
        ~RCProject() {}
        !RCProject() {}
    public:

        /// \brief Get the \b RCCode for project loading
        /// \return The \b RCCode
        RCCode GetProjectLoadCode();
        /// \brief Save the current project information to the corresponding project file
        /// \return Success if the file is saved successfully. Other \b RCCode otherwise.
        RCCode Save();
        /// \brief Get the project UUID.
        /// \return The project unique ID as System::String
        String^ GetProjectIdentifier();

        /// \brief Get the project base name.
        /// \return The project name as System::String
        String^ GetProjectName();

        /// \brief Get the project unit type
        /// \return The project unit type
        RCUnitType GetUnitType();

        /// \brief Set the project unit type
        /// \param unitType The unit type to set
        RCCode SetUnitType(RCUnitType unitType);

        /// \brief Remove the scan with the specified \p scanId from this project
        /// \param scanId The identity of the scan to remove
        /// \return \b RCCode for the process
        RCCode RemoveScan(String^ scanId);

        /// \brief Get the bounding box of all the scans in this project including
        ///        hidden and deleted area.
        /// \return The bounding box for the whole project
        RCBox GetBoundingBox();

        /// \brief Get the bounding box of the visible area of this project
        /// \return The bounding box of the visible project
        RCBox GetVisibleBoundingBox();

        /// \brief Get the number of the points in this project
        /// \return The number of points
        UInt64 GetNumberOfPoints();

        /// \brief Get the number of the scans in this project
        /// \return The number of scans
        Int32 GetNumberOfScans();

        /// \brief Get the scan at the given \p scanIndex
        /// \param scanIndex The index of the scan
        /// \return The scan at the given \p scanIndex
        RCScan^ GetScanAt(int scanIndex);

        /// \brief Get the index of the scan with the given \p scanId
        /// \param scanId The identity of the scan
        /// \return The index of the scan
        Int32 GetIndexOfScan(String^ scanId);

        /// \brief Get the scan by its identification \p scanId
        /// \param scanId The identity of the scan
        /// \return The scan of \p scanId
        RCScan^ GetScanById(String^ scanId);

        /// \brief Get the number of structured scans in this project
        /// \return The structured-scan count
        Int32 GetNumberOfStructuredScans();

        /// \brief Get the coordinate system of the project.
        ///        Scans in one project have the same target coordinate system as the project target coordinate system.
        /// \param coordinateSystem The coordinate system of the project
        /// \return True if getting coordinate system correctly. False otherwise.
        String^ GetCoordinateSystem();

        /// \brief Set the coordinate system of the project.
        ///        Scans in one project have the same target coordinate system as the project target coordinate system.
        /// \param coordinateSystem The project coordinate system to set
        /// \return True if setting coordinate system correctly. False otherwise.
        RCCode SetCoordinateSystem(String^ coordinateSystem);

        /// \brief Get the current project user defined coordinate system
        /// \param UCSTransform The current UCS transform of the project
        /// \return True if getting UCS correctly. False otherwise.
        bool GetUCSTransform(RCTransform% UCSTransform);

        /// \brief Set the current project user defined coordinate system
        /// \param UCSTransform The UCS transform of the project to set
        /// \return True if setting UCS correctly. False otherwise.
        bool SetUCSTransform(RCTransform% UCSTransform);

        /// \brief Get the project survey transform for the registration group
        /// \param groupToSurveyTransform The current project survey transform for the registration group
        /// \return True if getting survey transform correctly. False otherwise.
        bool GetProjectToSurveyPointsTransform(RCTransform% groupToSurveyTransform);

        /// \brief Set the project survey transform for the registration group
        /// \param groupToSurveyTransform The new project survey transform for the registration group
        /// \return True if survey transform was set correctly. False otherwise.
        bool SetProjectToSurveyPointsTransform(RCTransform% groupToSurveyTransform);

        //////////////////////////////////////////////////////////////////////////
        /// \brief Read the points by creating a point iterator used to iterate through all points in this project.
        /// \param settings Settings for how to create the point iterator.
        /// \return A list of points.
        List<RCPoint>^ GetPoints(RCPointIteratorSettings^ rc_settings);

        //////////////////////////////////////////////////////////////////////////
        /// \brief Read the points by creating a batch-by-batch point iterator used to iterate through all points in this project.
        /// \param settings Settings for how to create the point iterator.
        /// \return A list of points.        
        List<RCPoint>^ GetPointsByBatchIteration(RCPointIteratorSettings^ rc_settings);

        //////////////////////////////////////////////////////////////////////////
        /// \brief Read the points by creating a batch-by-batch point iterator used to iterate through all points in this project.
        /// \param settings Settings for how to create the point iterator.
        /// \param batchSize Custom size of batch.
        /// \return A list of points.
        List<RCPoint>^ GetPointsByBatchIteration(RCPointIteratorSettings^ rc_settings, unsigned long batchSize);

        //////////////////////////////////////////////////////////////////////////
        /// \brief Modifies the point in the project. To save changes use save() method after call.
        /// \param point Wrapped modified point to be changed in ReCap project
        /// \param settings Settings for how to create the point iterator. Density should be 0.0
        /// \return True if the point modified successfully, otherwise - false. If project is opened in ReadOnly file access mode it will return false.
        bool ModifyPoint(RCPoint% point, RCPointIteratorSettings^ settings);

        //////////////////////////////////////////////////////////////////////////
        /// \brief Add a temporary spatial filter for the project
        ///        Point traversal result will be affected by this spatial filter
        /// \note  The temporary spatial filter won't be persistent int the .rcp project file. It doesn't apply to RCStructuredScan either
        /// \param spatialFilter spatial filter to be added to the project
        /// \return Unique ID as System::String to identify the spatial filter if successful, System::String::Empty otherwise.
        String^ AddTemporarySpatialFilter(RCSpatialFilter^ spatialFilter);

        /// \brief Remove a temporary spatial filter with specified UUID from the project
        ///        Point traversal result will be affected by this spatial filter
        /// \param spatialFilterId UUID of the spatial filter to be removed from the project
        RCCode RemoveTemporarySpatialFilter(String^ spatialFilterId);

        ///\brief Get all existing temporary spatial filters added to this project
        ///\note  The spatial filters are the clones of the original spatial filter objects that are added into the project
        List<String^>^ GetTemporarySpatialFilters(List<RCSpatialFilter^>^ spatialFilters);

        /// \brief Remove all existing spatial filters from the project
        ///        Point traversal result will be affected by this spatial filter
        RCCode RemoveAllTemporarySpatialFilters();

        /// \brief Remove all existing edits in the project and update the effects and relevant information.
        void ClearUserEdits();

        /// \brief Clip the points defined by the input spatial filter. Clipped points will be invisible.
        /// \param spatialFilter  Spatial filter used to describe the filter criterion;
        ///                      Currently only RCBoxSpatialFilter / RCPlanarSlabSpatialFilter / RCPolygonSpatialFilter are supported
        /// \param clipInside  Determine if point inside spatial filter is clipped or outside is clipped
        /// \return Unique ID as System::String to identify the spatial filter if successful, System::String::Empty otherwise.
        String^ AddClip(RCSpatialFilter^ spatialFilter, bool clipInside);

        ///\brief Remove the clip generated by specified input spatialFilterID
        ///\param UUID of spatial filter to be removed from clip
        ///\return If the specified spatial filter doesn't exist in clips, it will return RCCode::Failed; otherwise, return RCCode::OK
        RCCode RemoveClip(String^ spatialFilterId);

        ///\brief Remove last clip that is added to the project
        RCCode RemoveLastClip();

        ///\brief Remove all the existing clips added to the project
        RCCode RemoveAllClips();

        //////////////////////////////////////////////////////////////////////////
        /// \brief Set the system memory limit that the project can use.
        ///        Note: Default system memory limit for the project is half of the total system memory.
        /// \param megaBytes The system memory limit of the project in MB.
        void SetSystemMemoryLimitInMB(UInt64 megaBytes);

        /// \brief Get the system memory limit that the project can use.
        /// \return The system memory limit of the project in MB.
        UInt64 GetSystemMemoryLimitInMB();

        //////////////////////////////////////////////////////////////////////////
        /// \brief Returns RCCode::OK if the folder exists
        RCCode GetSupportFolder(/*in*/String^ rcpFile, [Out] String^% folder);
        /// \brief Returns RCCode::OK if the folder exists
        RCCode GetCacheFolder(/*in*/String^ rcpFile, [Out] String^% folder);

        //////////////////////////////////////////////////////////////////////////
        /// \brief gather the required resources (.rcc/.rcs files) of the project to copy them under the project support folder
        ///        so that the project is ready to distribute
        /// \note  Project file will be saved in this interface so that scan file path information in the project file will
        ///        be updated to use the file path under support folder
        RCCode CopyScansToProjectFolder();

        ///\brief If any points have been marked as deleted before, this will permanently remove those points from their RCS files for the project.
        bool PermanentlyDeletePoints();

        /// \brief Get the estimated number of points in this project given the input point iterator setting.
        /// \param setting The setting to describe the options to traverse the points in this project
        /// \return The estimated number of points
        UInt64 GetNumberOfPointsEstimate(RCPointIteratorSettings^ settings);

    private:
        RCScopedPointer<NS_RCData::RCProject> mProjectOwned; // used when we need to manage lifetime by this object
        NS_RCData::RCProject* mProject; // raw pointer to RCProject
    };
}}}    // namespace Autodesk::RealityComputing::Managed
