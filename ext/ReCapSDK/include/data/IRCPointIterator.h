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

#include <foundation/RCVector.h>
#include <foundation/RCCode.h>
#include <foundation/RCBuffer.h>
#include <cstdint>

#include <data/RCDataAccessDef.h>

namespace Autodesk { namespace RealityComputing { namespace Data {

    using Autodesk::RealityComputing::Foundation::RCBuffer;
    using Autodesk::RealityComputing::Foundation::RCCode;
    using Autodesk::RealityComputing::Foundation::RCVector3d;
    using Autodesk::RealityComputing::Foundation::RCVector4ub;

    ///\brief Accessor for a point
    class IRCPointAccessor
    {
    public:
        // Meta-properties
        /// \brief Check if the point has classification information.
        /// \return \b true if the point has classification information. \b false otherwise.
        virtual bool hasClassification() const = 0;

        /// \brief check if the point has intensity information.
        /// \return \b true if the point has intensity information. \b false otherwise.
        virtual bool hasIntensity() const = 0;

        // Reading properties of the point
        /// \brief Reading position of the point.
        /// \return The point position.
        virtual const RCVector3d& getPosition() const = 0;

        /// \brief Reading color of the point.
        /// \return The point color.
        virtual RCVector4ub getColor() const = 0;

        /// \brief Reading normalized intensity of the point, in range [0, 255].
        /// \return The normalized intensity of the point.
        /// \note Use getIntensity() to get the original intensity from the raw scan of this point.
        virtual std::uint8_t getNormalizedIntensity() const = 0;

        /// \brief Reading original intensity of the point.
        /// \return The original intensity of the point.
        virtual float getIntensity() const = 0;

        /// \brief Reading normal of the point.
        /// \return The point normal.
        virtual RCVector3d getNormal() const = 0;

        /// \brief Reading classification of the point.
        /// \return The point classification.
        virtual std::uint8_t getClassification() const = 0;

        /// \brief Return if this point has been marked as deleted.
        /// \return \b true if this point is marked as deleted. \b false otherwise.
        virtual bool isDeleted() const = 0;

        /// \brief Reading region index of the point.
        /// \return The region index of the point.
        virtual int getRegion() const = 0;

        /// \brief Check if this point is read-only.
        /// \return \b true if this point is read-only. \b false otherwise.
        virtual bool isReadOnly() const = 0;
    };

    /// \brief Change values of certain point attributes.
    /// \note Only the non-spatial attribute values can be changed. Point position cannot be changed through this class.
    class IRCWritePointAccessor
    {
    public:
        /// \brief Set color for this point.
        /// \param[in] color The color value to set.
        /// \return \b true if successful. \b false otherwise.
        virtual bool setColor(const RCVector4ub& color) = 0;

        /// \brief Set normalized intensity for this point, in range [0,255].
        /// \param[in] normalizedIntensity The normalized intensity value to set.
        /// \return \b true if successful. \b false otherwise.
        virtual bool setNormalizedIntensity(std::uint8_t normalizedIntensity) = 0;

        /// \brief Set normal for this point.
        /// \param[in] normal The normal value to set.
        /// \return \b true if successful. \b false otherwise.
        virtual bool setNormal(const RCVector3d& normal) = 0;

        /// \brief Set classification for this point.
        /// \param[in] classification The classification value to set.
        /// \return \b true if successful. \b false otherwise.
        virtual bool setClassification(std::uint8_t classification) = 0;

        /// \brief Mark this point as deleted/undeleted.
        /// \param[in] deleted A flag of point deletion status.
        /// \return \b true if successful. \b false otherwise.
        virtual bool setDeleted(bool deleted) = 0;
    };

    ///\brief Iterator to traverse the points
    class RC_DATA_ACCESS_API IRCPointIterator
    {
    public:
        using PointId = unsigned long long;
        
		virtual ~IRCPointIterator() = default;

        // Methods for iterating over points
        /// \brief Reset the index to the beginning of the points.
        virtual void reset() = 0;

        /// \brief Move to next point.
        /// \return \b true if successful. \b false otherwise.
        virtual bool moveToNextPoint() = 0;

        /// \brief Check if the index has reached the end.
        /// \return \b true if the index has reached the end. \b false otherwise.
        virtual bool atEnd() const = 0;

        /// \brief Get the current point index.
        /// \return The point identifier.
        virtual PointId getCurrentIndex() const = 0;

        /// \brief Get the scan index.
        /// \return The scan index.
        virtual int getScanIndex() const = 0;

        /// \brief Get the current point that the iterator references.
        /// \return The point accessor.
        /// \note The point reference obtained from this method will become invalid after a call to moveToNextPoint() method.
        virtual IRCPointAccessor& getPoint() = 0;

        /// \brief Move to a point with a specific \p index.
        /// \param[in] index The point identifier.
        /// \return \b true if successful. \b false otherwise.
        /// \note This method is not supported for down-sampling point iterator.
        virtual bool moveToPoint(PointId index) = 0;

        /// \brief Close this iterator, and save all the modified information into the corresponding .rcs file
        ///        when this iterator is writable.
        /// \return An \b RCCode indicating the result of this operation.
        virtual RCCode close() = 0;
    };

    /// \brief Iterator to traverse the points in batches
    class RC_DATA_ACCESS_API IRCPointBatchIterator
    {
    public:
        virtual ~IRCPointBatchIterator() = default;

        using PointId = unsigned long long;

        // Methods for iterating over point
        /// \brief Reset the index to the beginning of the points.
        virtual void reset() = 0;

        /// \brief Move to the next point, which is the same as calling moveToNextBatch() with a batch size of 1.
        /// \return \b true if successful. \b false otherwise.
        virtual bool moveToNextPoint() = 0;

        /// \brief Check if the index has reached the end.
        /// \return \b true if the index has reached the end. \b false otherwise.
        virtual bool atEnd() const = 0;

        /// \brief Get the index of the last point in the current batch.
        /// \return The point identifier.
        virtual PointId getCurrentIndex() const = 0;

        /// \brief Get the index of the scan which the last point in the current batch belongs to.
        /// \return the scan index.
        virtual int getScanIndex() const = 0;

        /// \brief Get the current point that this iterator references, which is the last point in the current batch.
        /// \return The point accessor.
        /// \note The point reference obtained from this method will become invalid after a call to moveToNextPoint() or moveToNextBatch() methods.
        virtual IRCPointAccessor& getPoint() = 0;

        /// \brief Move to a specific \p index, which may be acquired through getCurrentIndex() method.
        /// \param[in] index The point identifier.
        /// \return \b true if the point of \p PointId can be reached. \b false if \p PointId is invalid.
        /// \note This method is not supported for down-sampling point iterator (e.g. when Density value is set in point iterator setting).
        virtual bool moveToPoint(PointId index) = 0;

        /// \brief Close this iterator, and save all the modified information into the corresponding .rcs file
        ///        when this iterator is writable.
        /// \return An \b RCCode indicating the result of this operation.
        virtual RCCode close() = 0;

        /// \brief Move to the next batch of points.
        ///       If the point count in the last batch is fewer than \p batchSize, the actual size of fetched points will be fewer than \p batchSize.
        /// \param[in] batchSize Number of points to be fetched during this move operation.
        /// \return \b true if points are fetched successfully. \b false otherwise.
        /// \note If this method returns \b false in case of error (e.g. out of memory), the iterator is not moved.
        virtual bool moveToNextBatch(unsigned long batchSize) = 0;

        /// \brief Get the accessors to the points of the current batch that the iterator references.
        /// \return A collection of point accessors.
        /// \note The point accessors obtained from this method will become invalid after a call to moveToNextPoint() or moveToNextBatch() methods.
        virtual RCBuffer<IRCPointAccessor*>& getPoints() = 0;
    };

}}}    // namespace Autodesk::RealityComputing::Data
