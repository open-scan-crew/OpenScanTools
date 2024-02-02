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

#include <data/RCDataDef.h>
#include <importexport/IRCImportPluginFileParser.h>

namespace Autodesk { namespace RealityComputing { namespace Foundation {
    enum class RCCode;
}}}    // namespace Autodesk::RealityComputing::Foundation

namespace Autodesk { namespace RealityComputing { namespace Data {

    using RCDescriptiveScanMetadata = Autodesk::RealityComputing::ImportExport::IRCImportPluginFileParser::DescriptiveScanMetadata;
    using RCStatisticalScanMetadata = Autodesk::RealityComputing::ImportExport::IRCImportPluginFileParser::StatisticalScanMetadata;

    /// \brief Indicating the attribute type for a given batch of points.
    ///        Not used when importing from \b RCPointBuffer.
    enum class RCPointAttributeType : uint32_t
    {
        Position             = 1 << 0,
        PositionX            = 1 << 1,
        PositionY            = 1 << 2,
        PositionZ            = 1 << 3,
        SphericalRange       = 1 << 4,
        SphericalAzimuth     = 1 << 5,
        SphericalElevation   = 1 << 6,
        SphericalColumnIndex = 1 << 7,
        SphericalRowIndex    = 1 << 8,
        RGB                  = 1 << 9,
        RGBA                 = 1 << 10,
        ColorR               = 1 << 11,
        ColorG               = 1 << 12,
        ColorB               = 1 << 13,
        ColorA               = 1 << 14,
        Intensity            = 1 << 15,
        TimeStamp            = 1 << 16,
        Normal               = 1 << 17,
    };

    /// \brief Representing a collection of pointers pointing to the scan data for importing
    struct RC_DATA_API RCDataPointers
    {
        /// \brief The size of the array
        unsigned int size = 0;

        /// \note One and only one of the following four cases is valid for position:
        ///       1. \p fPosition is set
        ///       2. \p fPositionX, fPositionY and fPositionZ are all set
        ///       3. \p dPosition is set
        ///       4. \p dPositionX, dPositionY and dPositionZ are all set
        ///       Otherwise, position is not set correctly

        /// \brief The pointer to the position array in float type
        const float (*fPosition)[3] = nullptr;

        /// \brief The pointer to the x-value array of the positions in float type
        const float* fPositionX = nullptr;

        /// \brief The pointer to the y-value array of the positions in float type
        const float* fPositionY = nullptr;

        /// \brief The pointer to the z-value array of the positions in float type
        const float* fPositionZ = nullptr;

        /// \brief The pointer to the position array in double type
        const double (*dPosition)[3] = nullptr;

        /// \brief The pointer to the x-value array of the positions in double type
        const double* dPositionX = nullptr;

        /// \brief The pointer to the y-value array of the positions in double type
        const double* dPositionY = nullptr;

        /// \brief The pointer to the z-value array of the positions in double type
        const double* dPositionZ = nullptr;

        /// \brief The pointer to the normal array in float type
        const float (*fNormal)[3] = nullptr;

        /// \brief The pointer to the normal array in double type
        const double (*dNormal)[3] = nullptr;

        /// \brief The pointer to the spherical range array in double type
        const double* sphericalRange = nullptr;

        /// \brief The pointer to the spherical azimuth array in double type
        const double* sphericalAzimuth = nullptr;

        /// \brief The pointer to the spherical elevation array in double type
        const double* sphericalElevation = nullptr;

        /// \brief The pointer to the row index array in uint32_t type
        const uint32_t* rowIndex = nullptr;

        /// \brief The pointer to the column index array in uint32_t type
        const uint32_t* columnIndex = nullptr;

        /// \note One and only one of the following four cases is valid for color:
        ///       1. \p colorRGB is set
        ///       2. \p colorR, colorG and colorB are all set
        ///       3. \p colorRGBA is set
        ///       4. \p colorR, colorG, colorR and colorA are all set
        ///       Otherwise, color is not set correctly

        /// \brief The pointer to the RGB color array in uint8_t type
        const uint8_t (*colorRGB)[3] = nullptr;

        /// \brief The pointer to the RGBA color array in uint8_t type
        const uint8_t (*colorRGBA)[4] = nullptr;

        /// \brief The pointer to R-value array of the color in uint8_t type
        const uint8_t* colorR = nullptr;

        /// \brief The pointer to G-value array of the color in uint8_t type
        const uint8_t* colorG = nullptr;

        /// \brief The pointer to B-value array of the color in uint8_t type
        const uint8_t* colorB = nullptr;

        /// \brief The pointer to A-value array of the color in uint8_t type
        const uint8_t* colorA = nullptr;

        /// \brief The pointer to the intensity array in double type
        const double* intensity = nullptr;

        /// \brief The pointer to the classification array in double type
        const uint8_t* classification = nullptr;

        /// \brief The pointer to the time stamp array in double type
        const double* timeStamp = nullptr;

    public:
        /// \brief Checks if this point array is valid
        /// \return true if valid. false otherwise.
        bool isValid() const;

        /// \brief Checks if this point array has valid position pointer(s) set
        /// \return true if having valid position. false otherwise.
        bool hasValidPosition() const;

        /// \brief Checks if this point array has valid color pointer(s) set
        /// \return true if having valid color. false otherwise.
        bool hasValidColor() const;

        /// \brief Checks if this point array has valid normal pointer(s) set
        /// \return true if having valid normal. false otherwise.
        bool hasValidNormal() const;

        /// \brief Checks if this point array has valid classification pointer(s) set
        /// \return true if having valid classification. false otherwise.
        bool hasValidClassification() const;

        /// \brief Checks if this point array has valid intensity pointer(s) set
        /// \return true if having valid intensity. false otherwise.
        bool hasValidIntensity() const;

        /// \brief Checks if this point array has valid timeStamp pointer(s) set
        /// \return true if having valid timeStamp. false otherwise.
        bool hasValidTimeStamp() const;

        /// \brief Checks if this point array has valid spherical range, azimuth and elevation pointer(s) set
        /// \return true if having valid spherical data points. false otherwise.
        bool hasValidSphericalData() const;

        /// \brief Convert position at \p index to RCVector3d format
        /// \param[in] index The index of the point
        /// \param[out] position The converted position in RCVector3d format
        /// \return \p RCCode
        Autodesk::RealityComputing::Foundation::RCCode positionToRCVector3d(size_t index, Autodesk::RealityComputing::Foundation::RCVector3d& position) const;

        /// \brief Convert normal at \p index to RCVector3d format
        /// \param[in] index The index of the point
        /// \param[out] position The converted normal in RCVector3d format
        /// \return \p RCCode
        Autodesk::RealityComputing::Foundation::RCCode normalToRCVector3d(size_t index, Autodesk::RealityComputing::Foundation::RCVector3d& normal) const;
        /// \brief Convert color at \p index to RCVector4ub format
        /// \param[in] index The index of the point
        /// \param[out] color The converted color in RCVector4ub format
        /// \return \p RCCode
        Autodesk::RealityComputing::Foundation::RCCode colorToRCVector4ub(size_t index, Autodesk::RealityComputing::Foundation::RCVector4ub& color) const;
    };
}}}    // namespace Autodesk::RealityComputing::Data
