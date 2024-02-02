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
#include <foundation/RCString.h>
#include <foundation/RCUUID.h>
#include <foundation/RCVector.h>
#include <foundation/RCSharedPtr.h>
#include <data/RCProjectIODef.h>
#include <data/RCStructuredScan.h>
#include <data/RCProjectLoadAttributes.h>

namespace Autodesk { namespace RealityComputing {
    namespace Foundation {
        class RCSphericalModel;
    }    // namespace Foundation

    namespace Data {

        struct RCScanDescriptor;

        class RCScan;

        using Autodesk::RealityComputing::Foundation::RCCode;
        using Autodesk::RealityComputing::Foundation::RCSharedPtr;
        using Autodesk::RealityComputing::Foundation::RCSphericalModel;
        using Autodesk::RealityComputing::Foundation::RCString;
        using Autodesk::RealityComputing::Foundation::RCUUID;
        using Autodesk::RealityComputing::Foundation::RCVector3d;
        using Autodesk::RealityComputing::Foundation::RCVector3f;

        struct RCPixelIndex
        {
            RCPixelIndex(size_t i, size_t j) : i(i), j(j)
            {
            }

            size_t i; //x
            size_t j; //y
        };
        ///
        /// \brief A class to get/set information about a structured scan
        ///
        class RC_PROJECTIO_API RCStructuredScan final
        {
            friend RCScanDescriptor;

        public:
            /// \brief Default constructor
            RCStructuredScan();

            /// \brief Construct an RCStructuredScan from a corresponding RCScan object
            /// \param pScan Scan object
            /// \param accessMode file open and access mode. If the file is opened in read-only mode,
            ///        setting attributes to point is still allowed but the information won't be saved into files
            explicit RCStructuredScan(RCScan* pScan, const RCFileAccess& accessMode = RCFileAccess::ReadWrite);

            /// \brief Default destructor
            virtual ~RCStructuredScan();

            ///\brief Load RCStructuredScan using the .rcc file path
            ///\param scanPath RCC file path
            ///\return Pointer to the structured scan
            static RCSharedPtr<RCStructuredScan> fromFile(const RCString& scanPath);

            ///\brief Load the structured scan into memory
            ///\return True if the structured scan is loaded successfully. False otherwise.
            bool load();

            ///\brief Unload the structured scan from memory
            ///\return True if the structured scan is unloaded successfully. False otherwise.
            void unload();

            ///\brief Check if the structured scan has been loaded into memory
            ///\return True if the structured scan has been loaded into memory. False otherwise.
            bool isLoaded() const;

            ///\brief Check if the structured scan has been loaded into memory and output load error code
            ///\param loadError Error code on the structured scan loading
            ///\return True if the structured scan has been loaded into memory. False otherwise.
            bool isLoaded(RCCode& loadError) const;

            ///\brief Get Unique Id for this structured scan
            ///       Note: Id of structured scan is different from RSScan Id
            ///\return Unique Id of this structured scan
            RCUUID getId() const;

            ///\brief Process this structured scan to generate derived information such as normal and segment
            ///\return True if the process is successful. False otherwise.
            bool process();

            ///\brief Save the point property changes to the structured scan
            ///\Note  If the project or the scan file is opened in a read-only mode or if the .rcc file is read-only
            ///       the information set by the user won't be saved
            ///       Changes to point normal and segment will be saved into files in temporarily cached folder,
            ///       if the folder is deleted, user defined normal and segment information will go away.
            RCCode save();

            ///\brief Get local position at the input pixel
            ///\param pixelIndex Pixel index with format (x,y)
            ///\param pos Local position at the input pixel
            ///\return True if the position is fetched successful. False otherwise
            bool getPosition(const RCPixelIndex& pixelIndex, RCVector3d& pos) const;

            ///\brief Get range value at the input pixel
            ///\param pixelIndex Pixel index with format (x,y)
            ///\param range Range value at the input pixel
            ///\return True if the range is fetched successful. False otherwise
            bool getRange(const RCPixelIndex& pixelIndex, float& range) const;

            ///\brief Get range value of a specified position in local coordinate
            ///\param xyz Position in local coordinate
            ///\param range Range value at the input pixel
            ///\return True if the range is fetched successful. False otherwise
            bool getRange(const RCVector3d& xyz, float& range) const;
            ///\brief Set range value at the input pixel
            ///\param pixelIndex Pixel index with format (x,y)
            ///\param range Range value to be set to the input pixel
            ///\return True if the range is set successful. False otherwise
            bool setRange(const RCPixelIndex& pixelIndex, float range);
            ///\brief Set range value of a specified position in local coordinate
            ///\param xyz Position in local coordinate
            ///\param range Range value to be set to the input pixel
            ///\return True if the range is set successful. False otherwise
            bool setRange(const RCVector3d& xyz, float range);

            ///\brief Check if the input pixel is a bad point without distance measurement
            ///\param pixelIndex Pixel index with format (x,y)
            ///\return True if the point is a bad point. False otherwise
            bool isBadPoint(const RCPixelIndex& pixelIndex) const;
            ///\brief Check if the input pixel is a bad point without distance measurement
            ///\param xyz Position in local coordinate
            ///\return True if the point is a bad point. False otherwise
            bool isBadPoint(const RCVector3d& xyz) const;
            ///\brief Set the input pixel as a bad point
            ///\param pixelIndex Pixel index with format (x,y)
            ///\return True if the bad point is set successful. False otherwise
            bool setBadPoint(const RCPixelIndex& pixelIndex);
            ///\brief Set the input pixel as a bad point
            ///\param xyz Position in local coordinate
            ///\return True if the bad point is set successful. False otherwise
            bool setBadPoint(const RCVector3d& xyz);

            ///\brief Get color value at the input pixel
            ///\param pixelIndex Pixel index with format (x,y)
            ///\param color Color at the input pixel
            ///\return True if the color is fetched successful. False otherwise
            bool getColor(const RCPixelIndex& pixelIndex, std::uint8_t (&color)[3]) const;
            ///\brief Get color of a specified position in local coordinate
            ///\param xyz Position in local coordinate
            ///\param color Color at the input pixel
            ///\return True if the color is fetched successful. False otherwise
            bool getColor(const RCVector3d& xyz, std::uint8_t (&color)[3]) const;
            ///\brief Set color value at the input pixel
            ///\param pixelIndex Pixel index with format (x,y)
            ///\param color Color value to be set to the input pixel
            ///\return True if the color is set successful. False otherwise
            bool setColor(const RCPixelIndex& pixelIndex, const std::uint8_t (&color)[3]);
            ///\brief Set color value of a specified position in local coordinate
            ///\param xyz Position in local coordinate
            ///\param range Color value to be set to the input pixel
            ///\return True if the color is set successful. False otherwise
            bool setColor(const RCVector3d& xyz, const std::uint8_t (&color)[3]);

            ///\brief Get intensity value at the input pixel
            ///\param pixelIndex Pixel index with format (x,y)
            ///\param intensity intensity value at the input pixel
            ///\return True if the intensity is fetched successful. False otherwise
            bool getNormalizedIntensity(const RCPixelIndex& pixelIndex, std::uint8_t& intensity) const;
            ///\brief Get intensity of a specified position in local coordinate
            ///\param xyz Position in local coordinate
            ///\param intensity Intensity at the input pixel
            ///\return True if the intensity is fetched successful. False otherwise
            bool getNormalizedIntensity(const RCVector3d& xyz, std::uint8_t& intensity) const;
            ///\brief Set intensity value at the input pixel
            ///\param pixelIndex Pixel index with format (x,y)
            ///\param intensity Intensity value to be set to the input pixel
            ///\return True if the intensity is set successful. False otherwise
            bool setNormalizedIntensity(const RCPixelIndex& pixelIndex, const std::uint8_t& intensity);
            ///\brief Set intensity value at the input pixel
            ///\param xyz Position in local coordinate
            ///\param intensity Intensity value to be set to the input pixel
            ///\return True if the intensity is set successful. False otherwise
            bool setNormalizedIntensity(const RCVector3d& xyz, const std::uint8_t& intensity);

            ///\brief Get normal at the input pixel
            ///\param pixelIndex Pixel index with format (x,y)
            ///\param normal Normal at the input pixel
            ///\return True if normal is fetched successful. False otherwise
            bool getNormal(const RCPixelIndex& pixelIndex, RCVector3f& normal) const;
            ///\brief Get normal of a specified position in local coordinate
            ///\param xyz Position in local coordinate
            ///\param normal Normal at the input pixel
            ///\return True if normal is fetched successful. False otherwise
            bool getNormal(const RCVector3d& xyz, RCVector3f& normal) const;
            ///\brief Set normal at the input pixel
            ///\param pixelIndex Pixel index with format (x,y)
            ///\param normal Normal to be set to the input pixel
            ///\return True if normal is set successful. False otherwise
            bool setNormal(const RCPixelIndex& pixelIndex, const RCVector3f& normal);
            ///\brief Set normal at the input pixel
            ///\param xyz Normal in local coordinate
            ///\param normal Normal to be set to the input pixel
            ///\return True if normal is set successful. False otherwise
            bool setNormal(const RCVector3d& xyz, const RCVector3f& normal);

            ///\brief Get segment Id at the input pixel
            ///\param pixelIndex Pixel index with format (x,y)
            ///\param segmentId Segment Id value at the input pixel
            ///\return True if segment is fetched successful. False otherwise
            bool getSegmentId(const RCPixelIndex& pixelIndex, std::uint16_t& segmentId) const;
            ///\brief get segment Id of a specified position in local coordinate
            ///\param xyz Position in local coordinate
            ///\param segmentId Segment Id at the input pixel
            ///\return True if segment is fetched successful. False otherwise
            bool getSegmentId(const RCVector3d& xyz, std::uint16_t& segmentId) const;
            ///\brief Set segment Id at the input pixel
            ///\param pixelIndex Pixel index with format (x,y)
            ///\param segmentId Segment Id to be set to the input pixel
            ///\return True if segment is set successful. False otherwise
            bool setSegmentId(const RCPixelIndex& pixelIndex, const std::uint16_t& segmentId);
            ///\brief Set segment Id at the input pixel
            ///\param xyz Position in local coordinate
            ///\param segmentId Segment Id to be set to the input pixel
            ///\return True if segment is set successful. False otherwise
            bool setSegmentId(const RCVector3d& xyz, const std::uint16_t& segmentId);

            ///\brief Get total number of segments in this structured scan
            ///\return Number of segments in this structured scan
            size_t getSegmentCount() const;

            ///\brief Get spherical model of this structured scan.
            ///       This spherical model can be used to get resolution and angle range of the structured scan
            ///       Scan needs to be loaded first before getting valid spherical model
            ///\return Pointer to spherical model of this structured scan. Return nullptr if structured scan is not loaded
            RCSharedPtr<RCSphericalModel> getSphericalModel() const;

            ///\brief Export spherical color image of this structured scan. 
            RCCode exportSphericalColorImage(const RCString& colorImagePath);

        private:
            class Impl;
            Impl* mImpl = nullptr;
        };
    }    // namespace Data
}}       // namespace Autodesk::RealityComputing
