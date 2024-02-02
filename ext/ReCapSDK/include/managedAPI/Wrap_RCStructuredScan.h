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
#include "managedAPI/Wrap_RCVector.h"
#include "managedAPI/Wrap_RCSphericalModel.h"

#include "RCScopedPointer.h"

namespace Autodesk { namespace RealityComputing { namespace Managed {
    public value struct RCPixelIndex
    {
        RCPixelIndex(UInt32 i, UInt32 j) : i(i), j(j)
        {
        }

        UInt32 i;
        UInt32 j;

        NS_RCData::RCPixelIndex ToReCapObject()
        {
            NS_RCData::RCPixelIndex result(i, j);
            return result;
        }
    };

    public ref class RCStructuredScan
    {

    public:
        /// \brief Default constructor
        RCStructuredScan() {};
        RCStructuredScan(const NS_RCFoundation::RCSharedPtr<NS_RCData::RCStructuredScan>& scanPtr);

        /// \brief Construct an RCStructuredScan from a corresponding RCScan object
        /// \param pScan Scan object
        /// \param accessMode file open and access mode. If the file is opened in read-only mode,
        ///        setting attributes to point is still allowed but the information won't be saved into files
        //explicit RCStructuredScan(RCScan* pScan, const RCFileAccess& accessMode = RCFileAccess::ReadWrite);

        /// \brief Default destructor
        //virtual ~RCStructuredScan();

        ///\brief Load RCStructuredScan using the .rcc file path
        ///\param scanPath RCC file path
        ///\return true if succsesfull
        bool FromFile(String^ scanPath);

        ///\brief Load the structured scan into memory
        ///\return True if the structured scan is loaded successfully. False otherwise.
        bool Load();

        ///\brief Unload the structured scan from memory
        ///\return True if the structured scan is unloaded successfully. False otherwise.
        void Unload();

        ///\brief Check if the structured scan has been loaded into memory
        ///\return True if the structured scan has been loaded into memory. False otherwise.
        bool IsLoaded();

        ///\brief Check if the structured scan has been loaded into memory and output load error code
        ///\param loadError Error code on the structured scan loading
        ///\return True if the structured scan has been loaded into memory. False otherwise.
        bool IsLoaded([Out] RCCode% loadError);

        ///\brief Get Unique Id for this structured scan
        ///       Note: Id of structured scan is different from RSScan Id
        ///\return Unique Id of this structured scan
        /*RCUUID*/String^ GetId();

        ///\brief Process this structured scan to generate derived information such as normal and segment
        ///\return True if the process is successful. False otherwise.
        bool Process();

        ///\brief Save the point property changes to the structured scan
        ///\Note  If the project or the scan file is opened in a read-only mode or if the .rcc file is read-only
        ///       the information set by the user won't be saved
        ///       Changes to point normal and segment will be saved into files in temporarily cached folder,
        ///       if the folder is deleted, user defined normal and segment information will go away.
        RCCode Save();

        ///\brief Get local position at the input pixel
        ///\param pixelIndex Pixel index with format (x,y)
        ///\param pos Local position at the input pixel
        ///\return True if the position is fetched successful. False otherwise
        bool GetPosition(RCPixelIndex pixelIndex, [Out] RCVector3d% pos);

        ///\brief Get range value at the input pixel
        ///\param pixelIndex Pixel index with format (x,y)
        ///\param range Range value at the input pixel
        ///\return True if the range is fetched successful. False otherwise
        bool GetRange(RCPixelIndex pixelIndex, [Out] float% range);

        ///\brief Get range value of a specified position in local coordinate
        ///\param xyz Position in local coordinate
        ///\param range Range value at the input pixel
        ///\return True if the range is fetched successful. False otherwise
        bool GetRange(RCVector3d xyz, [Out] float% range);
        ///\brief Set range value at the input pixel
        ///\param pixelIndex Pixel index with format (x,y)
        ///\param range Range value to be set to the input pixel
        ///\return True if the range is set successful. False otherwise
        bool SetRange(RCPixelIndex pixelIndex, float range);
        ///\brief Set range value of a specified position in local coordinate
        ///\param xyz Position in local coordinate
        ///\param range Range value to be set to the input pixel
        ///\return True if the range is set successful. False otherwise
        bool SetRange(RCVector3d xyz, float range);

        ///\brief Check if the input pixel is a bad point without distance measurement
        ///\param pixelIndex Pixel index with format (x,y)
        ///\return True if the point is a bad point. False otherwise
        bool IsBadPoint(RCPixelIndex pixelIndex);
        ///\brief Check if the input pixel is a bad point without distance measurement
        ///\param xyz Position in local coordinate
        ///\return True if the point is a bad point. False otherwise
        bool IsBadPoint(RCVector3d xyz);
        ///\brief Set the input pixel as a bad point
        ///\param pixelIndex Pixel index with format (x,y)
        ///\return True if the bad point is set successful. False otherwise
        bool SetBadPoint(RCPixelIndex pixelIndex);
        ///\brief Set the input pixel as a bad point
        ///\param xyz Position in local coordinate
        ///\return True if the bad point is set successful. False otherwise
        bool SetBadPoint(RCVector3d xyz);

        ///\brief Get color value at the input pixel
        ///\param pixelIndex Pixel index with format (x,y)
        ///\param color Color at the input pixel
        ///\return True if the color is fetched successful. False otherwise
        bool GetColor(RCPixelIndex pixelIndex, [Out] RCColor% color);
        ///\brief Get color of a specified position in local coordinate
        ///\param xyz Position in local coordinate
        ///\param color Color at the input pixel
        ///\return True if the color is fetched successful. False otherwise
        bool GetColor(RCVector3d xyz, [Out] RCColor% color);
        ///\brief Set color value at the input pixel
        ///\param pixelIndex Pixel index with format (x,y)
        ///\param color Color value to be set to the input pixel
        ///\return True if the color is set successful. False otherwise
        bool SetColor(RCPixelIndex pixelIndex, RCColor color);
        ///\brief Set color value of a specified position in local coordinate
        ///\param xyz Position in local coordinate
        ///\param range Color value to be set to the input pixel
        ///\return True if the color is set successful. False otherwise
        bool SetColor(RCVector3d xyz, RCColor color);

        ///\brief Get intensity value at the input pixel
        ///\param pixelIndex Pixel index with format (x,y)
        ///\param intensity intensity value at the input pixel
        ///\return True if the intensity is fetched successful. False otherwise
        bool GetNormalizedIntensity(RCPixelIndex pixelIndex, [Out] Byte% intensity);
        ///\brief Get intensity of a specified position in local coordinate
        ///\param xyz Position in local coordinate
        ///\param intensity Intensity at the input pixel
        ///\return True if the intensity is fetched successful. False otherwise
        bool GetNormalizedIntensity(RCVector3d xyz, [Out] Byte% intensity);
        ///\brief Set intensity value at the input pixel
        ///\param pixelIndex Pixel index with format (x,y)
        ///\param intensity Intensity value to be set to the input pixel
        ///\return True if the intensity is set successful. False otherwise
        bool SetNormalizedIntensity(RCPixelIndex pixelIndex, Byte intensity);
        ///\brief Set intensity value at the input pixel
        ///\param xyz Position in local coordinate
        ///\param intensity Intensity value to be set to the input pixel
        ///\return True if the intensity is set successful. False otherwise
        bool SetNormalizedIntensity(RCVector3d xyz, Byte intensity);

        ///\brief Get normal at the input pixel
        ///\param pixelIndex Pixel index with format (x,y)
        ///\param normal Normal at the input pixel
        ///\return True if normal is fetched successful. False otherwise
        bool GetNormal(RCPixelIndex pixelIndex, [Out] RCVector3d% normal);
        ///\brief Get normal of a specified position in local coordinate
        ///\param xyz Position in local coordinate
        ///\param normal Normal at the input pixel
        ///\return True if normal is fetched successful. False otherwise
        bool GetNormal(RCVector3d xyz, [Out] RCVector3d% normal);
        ///\brief Set normal at the input pixel
        ///\param pixelIndex Pixel index with format (x,y)
        ///\param normal Normal to be set to the input pixel
        ///\return True if normal is set successful. False otherwise
        bool SetNormal(RCPixelIndex pixelIndex, RCVector3d normal);
        ///\brief Set normal at the input pixel
        ///\param xyz Normal in local coordinate
        ///\param normal Normal to be set to the input pixel
        ///\return True if normal is set successful. False otherwise
        bool SetNormal(RCVector3d xyz, RCVector3d normal);

        ///\brief Get segment Id at the input pixel
        ///\param pixelIndex Pixel index with format (x,y)
        ///\param segmentId Segment Id value at the input pixel
        ///\return True if segment is fetched successful. False otherwise
        bool GetSegmentId(RCPixelIndex pixelIndex, [Out] UInt16% segmentId);
        ///\brief get segment Id of a specified position in local coordinate
        ///\param xyz Position in local coordinate
        ///\param segmentId Segment Id at the input pixel
        ///\return True if segment is fetched successful. False otherwise
        bool GetSegmentId(RCVector3d xyz, [Out] UInt16% segmentId);
        ///\brief Set segment Id at the input pixel
        ///\param pixelIndex Pixel index with format (x,y)
        ///\param segmentId Segment Id to be set to the input pixel
        ///\return True if segment is set successful. False otherwise
        bool SetSegmentId(RCPixelIndex pixelIndex, UInt16 segmentId);
        ///\brief Set segment Id at the input pixel
        ///\param xyz Position in local coordinate
        ///\param segmentId Segment Id to be set to the input pixel
        ///\return True if segment is set successful. False otherwise
        bool SetSegmentId(RCVector3d xyz, UInt16 segmentId);

        ///\brief Get total number of segments in this structured scan
        ///\return Number of segments in this structured scan
        UInt32 GetSegmentCount();

        ///\brief Get spherical model of this structured scan.
        ///       This spherical model can be used to get resolution and angle range of the structured scan
        ///       Scan needs to be loaded first before getting valid spherical model
        ///\return Spherical model of this structured scan. Return nullptr if structured scan is not loaded
        RCSphericalModel^ GetSphericalModel();

    private:
        RCScopedPointer<NS_RCData::RCStructuredScan> mStructuredScan;
    };
}}}    // namespace Autodesk::RealityComputing::Managed
