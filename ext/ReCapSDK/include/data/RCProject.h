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

#include <foundation/RCBox.h>
#include <foundation/RCSharedPtr.h>
#include <foundation/RCBuffer.h>
#include <foundation/RCCode.h>
#include <foundation/RCString.h>
#include <foundation/RCUUID.h>
#include <foundation/RCUnitService.h>
#include <foundation/RCVector.h>
#include <data/RCProjectIODef.h>
#include <data/RCProjectLoadAttributes.h>

namespace Autodesk { namespace RealityComputing { namespace Data {

    class IRCPointIterator;
    class IRCPointBatchIterator;
    class RCPointIteratorSettings;
    class RCSpatialFilter;
    class RCScan;

    using Autodesk::RealityComputing::Foundation::RCBox;
    using Autodesk::RealityComputing::Foundation::RCBuffer;
    using Autodesk::RealityComputing::Foundation::RCCode;
    using Autodesk::RealityComputing::Foundation::RCSharedPtr;
    using Autodesk::RealityComputing::Foundation::RCString;
    using Autodesk::RealityComputing::Foundation::RCTransform;
    using Autodesk::RealityComputing::Foundation::RCUnitType;
    using Autodesk::RealityComputing::Foundation::RCUUID;
    using Autodesk::RealityComputing::Foundation::RCVector4ub;

    typedef void (*RCProcessCallBackPtr)(const Foundation::RCString& scanName, float progress);

    ///
    /// \brief Represents a ReCap project
    ///
    class RC_PROJECTIO_API RCProject
    {
        friend class RCScan;

    public:
        /// \brief Default destructor
        virtual ~RCProject();

        /// \brief Open a given ReCap project using open mode \p fileAccess.
        /// \param projectFilePath The path of the rcp project file
        /// \param fileAccess The file open mode
        /// \param userEdits Specify the type of user edits to load.
        /// \param code RCCode for project loading
        /// \return An instance of RCProject if the file is successfully loaded, or
        ///         nullptr otherwise.
        static RCSharedPtr<RCProject> loadFromFile(const RCString& projectFilePath, const RCFileAccess& fileAccess, const RCProjectUserEdits& userEdits,
                                                   RCCode& code);

        /// \brief Get the \b RCCode for project loading
        /// \return The \b RCCode
        RCCode getProjectLoadCode() const;

        /// \brief Save the current project information to the corresponding project file
        /// \return Success if the file is saved successfully. Other \b RCCode otherwise.
        RCCode save();

        /// \brief Get the project UUID.
        /// \return The project uuid string
        RCUUID getProjectIdentifier() const;

        /// \brief Get the project base name.
        /// \return The project name in text string
        RCString getProjectName() const;

        /// \brief Get the project unit type
        /// \return The project unit type
        RCUnitType getUnitType() const;

        /// \brief Set the project unit type
        /// \param unitType The unit type to set
        RCCode setUnitType(const RCUnitType& unitType);

        /// \brief Remove the scan with the specified \p scanId from this project
        /// \param scanId The identity of the scan to remove
        /// \return \b RCCode for the process
        RCCode removeScan(const RCString& scanId);

        /// \brief Get the bounding box of all the scans in this project including
        ///        hidden and deleted area.
        /// \return The bounding box for the whole project
        RCBox getBoundingBox() const;

        /// \brief Get the bounding box of the visible area of this project
        /// \return The bounding box of the visible project
        RCBox getVisibleBoundingBox() const;

        /// \brief Get the number of the points in this project
        /// \return The number of points
        uint64_t getNumberOfPoints() const;

        /// \brief Get the number of the scans in this project
        /// \return The number of scans
        int getNumberOfScans() const;

        /// \brief Get the scan at the given \p scanIndex
        /// \param scanIndex The index of the scan
        /// \return The scan at the given \p scanIndex
        RCSharedPtr<RCScan> getScanAt(int scanIndex) const;

        /// \brief Get the index of the scan with the given \p scanId
        /// \param scanId The identity of the scan
        /// \return The index of the scan
        int getIndexOfScan(const RCString& scanId) const;

        /// \brief Get the scan by its identification \p scanId
        /// \param scanId The identity of the scan
        /// \return The scan of \p scanId
        RCSharedPtr<RCScan> getScanById(const RCString& scanId) const;

        /// \brief Get the number of structured scans in this project
        /// \return The structured-scan count
        int getNumberOfStructuredScans() const;

        /// \brief Get the coordinate system of the project.
        ///        Scans in one project have the same target coordinate system as the project target coordinate system.
        /// \param coordinateSystem The coordinate system of the project
        /// \return True if getting coordinate system correctly. False otherwise.
        bool getCoordinateSystem(RCString& coordinateSystem);

        /// \brief Set the coordinate system of the project.
        ///        Scans in one project have the same target coordinate system as the project target coordinate system.
        /// \param coordinateSystem The project coordinate system to set
        /// \return True if setting coordinate system correctly. False otherwise.
        RCCode setCoordinateSystem(const RCString& coordinateSystem);

        /// \brief Get the current project user defined coordinate system
        /// \param UCSTransform The current UCS transform of the project
        /// \return True if getting UCS correctly. False otherwise.
        bool getUCSTransform(RCTransform& UCSTransform) const;

        /// \brief Set the current project user defined coordinate system
        /// \param UCSTransform The UCS transform of the project to set
        /// \return True if setting UCS correctly. False otherwise.
        bool setUCSTransform(const RCTransform& UCSTransform);

        /// \brief Get the project survey transform for the registration group
        /// \param groupToSurveyTransform The current project survey transform for the registration group
        /// \return True if getting survey transform correctly. False otherwise.
        bool getProjectToSurveyPointsTransform(RCTransform& groupToSurveyTransform) const;

        /// \brief Set the project survey transform for the registration group
        /// \param groupToSurveyTransform The new project survey transform for the registration group
        /// \return True if survey transform was set correctly. False otherwise.
        bool setProjectToSurveyPointsTransform(const RCTransform& groupToSurveyTransform);

        //////////////////////////////////////////////////////////////////////////
        /// \brief Create a point iterator that can be used to iterate through all points in this project.
        /// \param settings Settings for how to create the point iterator.
        /// \return A new instance of a point iterator.
        RCSharedPtr<IRCPointIterator> createPointIterator(const RCPointIteratorSettings& setting);

        //////////////////////////////////////////////////////////////////////////
        /// \brief Create a batch-by-batch point iterator that can be used to iterate through all points in this project.
        /// \param settings Settings for how to create the batch-by-batch point iterator.
        /// \return A new instance of a batch-by-batch point iterator.
        RCSharedPtr<IRCPointBatchIterator> createPointBatchIterator(const RCPointIteratorSettings& settings);
        //////////////////////////////////////////////////////////////////////////
        /// \brief Add a temporary spatial filter for the project
        ///        Point traversal result will be affected by this spatial filter
        /// \note  The temporary spatial filter won't be persistent int the .rcp project file. It doesn't apply to RCStructuredScan either
        /// \param spatialFilter spatial filter to be added to the project
        /// \return Unique RCUUID to identify the spatial filter if successful, RCUUID::nil() otherwise.
        RCUUID addTemporarySpatialFilter(const RCSharedPtr<RCSpatialFilter>& spatialFilter);

        /// \brief Remove a temporary spatial filter with specified UUID from the project
        ///        Point traversal result will be affected by this spatial filter
        /// \param spatialFilterId UUID of the spatial filter to be removed from the project
        RCCode removeTemporarySpatialFilter(const RCUUID& spatialFilterId);

        ///\brief Get all existing temporary spatial filters added to this project
        ///\note  The spatial filters are the clones of the original spatial filter objects that are added into the project
        RCBuffer<RCUUID> getTemporarySpatialFilters(RCBuffer<RCSharedPtr<RCSpatialFilter>>& spatialFilters) const;

        /// \brief Remove all existing spatial filters from the project
        ///        Point traversal result will be affected by this spatial filter
        RCCode removeAllTemporarySpatialFilters();

        /// \brief Remove all existing edits in the project and update the effects and relevant information.
        void clearUserEdits();

        /// \brief Clip the points defined by the input spatial filter. Clipped points will be invisible.
        /// \param spatialFilter  Spatial filter used to describe the filter criterion;
        ///                      Currently only RCBoxSpatialFilter / RCPlanarSlabSpatialFilter / RCPolygonSpatialFilter are supported
        /// \param clipInside  Determine if point inside spatial filter is clipped or outside is clipped
        /// \return Unique RCUUID to identify the spatial filter if successful, RCUUID::nil() otherwise.
        RCUUID addClip(const RCSharedPtr<RCSpatialFilter>& spatialFilter, bool clipInside);

        ///\brief Remove the clip generated by specified input spatialFilterID
        ///\param UUID of spatial filter to be removed from clip
        ///\return If the specified spatial filter doesn't exist in clips, it will return RCCode::Failed; otherwise, return RCCode::OK
        RCCode removeClip(const RCUUID& spatialFilterId);

        ///\brief Remove last clip that is added to the project
        RCCode removeLastClip();

        ///\brief Remove all the existing clips added to the project
        RCCode removeAllClips();

        //////////////////////////////////////////////////////////////////////////
        /// \brief Set the system memory limit that the project can use.
        ///        Note: Default system memory limit for the project is half of the total system memory.
        /// \param megaBytes The system memory limit of the project in MB.
        void setSystemMemoryLimitInMB(std::uint64_t megaBytes);

        /// \brief Get the system memory limit that the project can use.
        /// \return The system memory limit of the project in MB.
        std::uint64_t getSystemMemoryLimitInMB();

        //////////////////////////////////////////////////////////////////////////
        /// \brief Returns RCCode::OK if the folder exists
        static RCCode getSupportFolder(const RCString& rcpFile, RCString& folder);
        /// \brief Returns RCCode::OK if the folder exists
        static RCCode getCacheFolder(const RCString& rcpFile, RCString& folder);

        //////////////////////////////////////////////////////////////////////////
        /// \brief gather the required resources (.rcc/.rcs files) of the project to copy them under the project support folder
        ///        so that the project is ready to distribute
        /// \note  Project file will be saved in this interface so that scan file path information in the project file will
        ///        be updated to use the file path under support folder
        RCCode copyScansToProjectFolder();

        ///\brief If any points have been marked as deleted before, this will permanently remove those points from their RCS files for the project.
        bool permanentlyDeletePoints();

        /// \brief Get the estimated number of points in this project given the input point iterator setting.
        /// \param setting The setting to describe the options to traverse the points in this project
        /// \return The estimated number of points
        uint64_t getNumberOfPointsEstimate(RCPointIteratorSettings& setting) const;

        ///\brief remove the deleted points from structured scans
        ///\param keepColor True to keep the original color information for the deleted points, 
		///       false to change the color into \p replacementColor for the deleted points.
        ///\param replacementColor If user chooses not to keep the original color
        ///\note  This does not work for projects that have been optimized because the deletion history has been removed from the optimized projects.
        RCCode deletePointsFromStructuredScan(bool keepColor, const RCVector4ub& replacementColor = RCVector4ub(0, 0, 0, 0), 
            RCProcessCallBackPtr processCallBack = nullptr);

    private:
        /// \brief Default private constructor
        RCProject();

    protected:
        class Impl;
        Impl* mImpl;
        friend Impl;
    };

}}}    // namespace Autodesk::RealityComputing::Data
