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
#include <foundation/RCMacros.h>

#include <foundation/RCBuffer.h>
#include <foundation/RCVector.h>
#include <foundation/RCEnumBitField.h>

namespace Autodesk { namespace RealityComputing { namespace Data {

    using Autodesk::RealityComputing::Foundation::RCBuffer;
    using Autodesk::RealityComputing::Foundation::RCString;
    using Autodesk::RealityComputing::Foundation::RCVector3d;
    using Autodesk::RealityComputing::Foundation::RCVector4ub;

    /// \brief The types of attributes
    enum class RCAttributeType : uint32_t
    {
        None           = 0,
        Position       = 1 << 0,
        Color          = 1 << 1,
        Intensity      = 1 << 2,
        Classification = 1 << 3,
        Normal         = 1 << 4,
        SegmentId      = 1 << 5,
        TimeStamp      = 1 << 6,
        RowIndex       = 1 << 7,
        ColumnIndex    = 1 << 8,
        End            = 1 << 9    // added for iterating purposes
    };

    RC_ENABLE_BITFIELD(RCAttributeType)

    /// \brief The types of coordinates
    enum class RCCoordinateType
    {
        Cartesian,
        Spherical
    };

    /// \brief  Represents buffers for point cloud attribute values.
    /// \details RCPointBuffer has the ownership of the data inside it.
    ///          This point buffer can be used for points from either unstructured scan or structured scan.
    ///
    ///          For unstructured scans, Cartesian position is required,
    ///          while color / intensity / classification / segmentId / timeStamp field is optional.
    ///
    ///          For structured scans, either Cartesian position or spherical coordinates is required,
    ///          while color / intensity / classification / segmentId / timeStamp field is optional.
    ///
    ///			 To colorize points without depth information in structured scans, positions must be provided in
    ///			 spherical coordinates.
    ///
    ///          ColumnIndex / rowIndex is optional for structured scans, but they can help to create better RealViews.
    class RC_DATA_API RCPointBuffer
    {
        RC_PIMPL_DECLARATION(RCPointBuffer)

    public:
        /// \brief Get the buffer size
        /// \return The buffer size
        unsigned int getSize() const;

        /// \brief Resize the buffer.
        /// \param[in] size The new size for the buffer.
        /// \note This method should be called before calling \p setPositions(),
        ///       \p setPositionsAt() or \p setPositionAt(), if the buffer size is 0.
        void resize(unsigned int size);

        /// \brief Release all the buffers
        void clear();

        /// \brief [DEPRECATED] Set the point cloud coordinate system.
        /// \param[in] coordinateSystem The coordinate system to set.
        /// \note get(set)CoordinateSystem() was used when calling RCProjectImporter::createProject(Scan)FromPoints(). Those functions
        /// have been deprecated and have been replaced by RCProject(Scan)ImportSession classes. Coordinate system information can be
        /// set as options in these new classes. As such, RCPointBuffer::get(set)CoordinateSystem() have been deprecated.
        [[deprecated("get(set)CoordinateSystem() was used when calling RCProjectImporter::createProject(Scan)FromPoints(). Those functions "
                     "have been deprecated and have been replaced by RCProject(Scan)ImportSession classes. Coordinate system information can be "
                     "set as options in these new classes. As such, RCPointBuffer::get(set)CoordinateSystem() have been deprecated.")]] void
            setCoordinateSystem(const RCString& coordinateSystem);

        /// \brief [DEPRECATED] Get the point cloud coordinate system.
        /// \return The point cloud coordinate system.
        /// \note get(set)CoordinateSystem() was used when calling RCProjectImporter::createProject(Scan)FromPoints(). Those functions
        /// have been deprecated and have been replaced by RCProject(Scan)ImportSession classes. Coordinate system information can be
        /// set as options in these new classes. As such, RCPointBuffer::get(set)CoordinateSystem() have been deprecated.
        [[deprecated("get(set)CoordinateSystem() was used when calling RCProjectImporter::createProject(Scan)FromPoints(). Those functions "
                     "have been deprecated and have been replaced by RCProject(Scan)ImportSession classes. Coordinate system information can be "
                     "set as options in these new classes. As such, RCPointBuffer::get(set)CoordinateSystem() have been deprecated.")]] const RCString&
            getCoordinateSystem() const;

        /// \brief Return the coordinate type of the positions buffer.
        /// \return Cartesian or Spherical. Default is Cartesian.
        RCCoordinateType getCoordinateType() const;

        /// \brief Set the coordinate type of the positions buffer.
        /// \param[in] type Cartesian or Spherical. Cylindrical is not supported for now. Default is Cartesian.
        void setCoordinateType(RCCoordinateType type);

        /// \brief Append the given \p points to the positions buffer.
        /// \param[in] points The source points to append to the end of the positions buffer. Points can be Cartesian
        ///               (x, y, z) or Spherical (r, a, e), depending on what is set in setCoordinateType().
        /// \note For a point in a structured scan, Cartesian coordinate of (0.0, 0.0, 0.0) or Spherical coordinate
        ///       with range 0.0 is regarded as a bad point without depth information.
        ///       Color information can still be attached to a point without depth information (such as sky point).
        virtual void appendPositions(const RCBuffer<RCVector3d>& points);

        /// \brief Append the given \p point to the positions buffer.
        /// \param[in] point The source point to append to the end of the positions buffer. Point can be Cartesian
        ///               (x, y, z) or Spherical (r, a, e), depending on what is set in setCoordinateType().
        /// \note For a point in a structured scan, Cartesian coordinate of (0.0, 0.0, 0.0) or Spherical coordinate
        ///       with range 0.0 is regarded as a bad point without depth information.
        ///       Color information can still be attached to a point without depth information (such as sky point).
        virtual void appendPosition(const RCVector3d& point);

        /// \brief Set/Update the positions buffer with the given \p points.
        /// \param[in] points The source points to set the positions buffer. Points can be Cartesian
        ///               (x, y, z) or Spherical (r, a, e), depending on what is set in setCoordinateType().
        /// \note For a point in a structured scan, Cartesian coordinate of (0.0, 0.0, 0.0) or Spherical coordinate
        ///       with range 0.0 is regarded as a bad point without depth information.
        ///       Color information can still be attached to a point without depth information (such as sky point).
        /// \note The point buffer must be resized before positions are set.
        /// \return \b true if buffer is larger than or equal to the number of points provided, and the operation is
        ///         successful. \b false otherwise.
        virtual bool setPositions(const RCBuffer<RCVector3d>& points);

        /// \brief Set/Update the positions buffer with the given \p points at the given \p index
        /// \param[in] index The index of the positions buffer item to copy into.
        /// \param[in] points The positions to set the positions buffer with.
        /// \note For a point in a structured scan, Cartesian coordinate of (0.0, 0.0, 0.0) or Spherical coordinate
        ///       with range 0.0 is regarded as a bad point without depth information.
        ///       Color information can still be attached to a point without depth information (such as sky point).
        /// \note The point buffer must be resized before positions are set.
        /// \return \b true if buffer has enough space starting at \p index to set the given \points. \b false otherwise.
        virtual bool setPositionsAt(unsigned int index, const RCBuffer<RCVector3d>& points);

        /// \brief Get the positions buffer. Positions can be Cartesian (x,y,z) or Spherical (r,a,e), depending on
        ///        the return value of getCoordinateType().
        /// \return The positions buffer.
        virtual const RCBuffer<RCVector3d>* const getPositions() const;

        /// \brief Set/Update the positions buffer with the given \p point at the given \p index.
        /// \param[in] index The index of the positions buffer item to set.
        /// \param[in] point The position to set the positions buffer with.
        /// \note For a point in a structured scan, Cartesian coordinate of (0.0, 0.0, 0.0) or Spherical coordinate
        ///       with range 0.0 is regarded as a bad point without depth information.
        ///       Color information can still be attached to a point without depth information (such as sky point).
        /// \return \b true if \p index is smaller than the buffer's size. \b false otherwise.
        virtual bool setPositionAt(unsigned int index, const RCVector3d& point);

        /// \brief Get the position at the given \p index. Positions can be Cartesian (x,y,z) or Spherical (r, a, e),
        ///        depending on the return value of getCoordinateType().
        /// \param[in] index The index of the positions buffer item to get.
        /// \param[out] point The output position at \p index.
        /// \return \b true if successful. \b false otherwise.
        virtual bool getPositionAt(unsigned int index, RCVector3d& point) const;

        /// \brief Add a value buffer for the given \p attribute.
        /// \param[in] attribute Attribute to add. Multiple attributes can be added using bitwise '|' operator.
        /// \return \b true if successful. \b false otherwise.
        /// \note For the given \p attribute, this method must be called before getting or setting the corresponding buffer.
        bool addAttribute(RCAttributeType attribute);

        /// \brief Remove the value buffer for the given \p attribute.
        /// \param[in] attribute Attribute to remove. Multiple attributes can be removed using bitwise '|' operator.
        /// \return \b true if successful. \b false otherwise.
        bool removeAttribute(RCAttributeType attribute);

        /// \brief Checks if the point buffer has the given \p attribute.
        /// \param[in] attribute Attribute to check for. Multiple attributes can be checked for using the bitwise '|' operator.
        /// \return \b true if the point buffer has all the given attributes. \b false otherwise.
        bool hasAttribute(RCAttributeType attribute) const;

        /// \brief Check if the normals buffer is ready to use.
        /// \return \b true if normal buffer exists. \b false otherwise.
        virtual bool hasNormal() const;

        /// \brief Set/Update the normals buffer with the source \p normals.
        /// \param[in] normals The source normals to set the normals buffer.
        /// \return \b true if successful. \b false otherwise.
        /// \note The normals buffer, which has the same size as that of
        ///       the positions buffer, will not be resized by this method.
        virtual bool setNormals(const RCBuffer<RCVector3d>& normals);

        /// \brief Set/Update the normals buffer with the source \p normals from
        /// \p index.
        /// \param[in] index The index of the normals buffer from which to set normals.
        /// \param[in] normals The source normals to set the normals buffer.
        /// \return \b true if successful. \b false otherwise.
        /// \note The normals buffer, which has the same size as that of
        ///       the positions buffer, will not be resized by this method.
        virtual bool setNormalsAt(unsigned int index, const RCBuffer<RCVector3d>& normals);

        /// \brief Get the normals buffer
        /// \return The normals buffer. If a normals buffer does not exist return a nullptr instead.
        virtual const RCBuffer<RCVector3d>* const getNormals() const;

        /// \brief Set/Update the normal value at the given \p index of the
        /// normals buffer with \p normal.
        /// \param[in] index The index of the normals buffer item to set.
        /// \param[in] normal The normal value to set.
        /// \return \b true if successful. \b false otherwise.
        /// \note The normals buffer will not resize if \p index is out of range, in
        ///       which case the operation will fail.
        virtual bool setNormalAt(unsigned int index, const RCVector3d& normal);

        /// \brief Get the normal from the normals buffer at the given \p index.
        /// \param[in] index The index of the normals buffer.
        /// \param[out] normal The output normal at \p index.
        /// \return \b true if successful. \b false otherwise.
        virtual bool getNormalAt(unsigned int index, RCVector3d& normal) const;

        /// \brief Check if the classifications buffer is ready to use.
        /// \return \b true if classifications buffer exists. \b false otherwise.
        virtual bool hasClassification() const;

        /// \brief Set/Update the classifications buffer with the source \p classifications.
        /// \param[in] classifications The source classifications in range [0, 255] to set the classifications buffer.
        /// \return \b true if successful. \b false otherwise.
        /// \note The classifications buffer, which has the same size as that of
        ///       the positions buffer, will not be resized by this method.
        virtual bool setClassifications(const RCBuffer<unsigned char>& classifications);

        /// \brief Set/Update the classifications buffer with the source \p
        /// classifications from \p index.
        /// \param[in] index The index of the classifications buffer in which to set classifications.
        /// \param[in] classifications The source classifications in range [0, 255] to set.
        /// \return \b true if successful. \b false otherwise.
        /// \note The classifications buffer, which has the same size as that of
        ///       the positions buffer, will not be resized by this method.
        virtual bool setClassificationsAt(unsigned int index, const RCBuffer<unsigned char>& classifications);

        /// \brief Get the classifications buffer.
        /// \return The classifications buffer. A \b nullptr is returned instead if there is no classifications buffer.
        virtual const RCBuffer<unsigned char>* const getClassifications() const;

        /// \brief Set/Update the classification value at the given \p index of
        /// the classifications buffer with \p classification.
        /// \param[in] index The index of the classifications buffer item to set.
        /// \param[in] classification The classification value to set in range [0, 255].
        /// \return \b true if successful. \b false otherwise.
        /// \note The classifications buffer will not resize if \p index is out of range, in
        ///       which case the operation will fail.
        virtual bool setClassificationAt(unsigned int index, unsigned char classification);

        /// \brief Get the classification from the classifications buffer at the
        /// given \p index.
        /// \param[in] index The index of the classifications buffer.
        /// \param[out] classification The output classification at \p index.
        /// \return \b true if successful. \b false otherwise.
        virtual bool getClassificationAt(unsigned int index, unsigned char& classification) const;

        /// \brief Check if the colors buffer is ready to use.
        /// \return \b true if colors buffer exists. \b false otherwise.
        virtual bool hasColor() const;

        /// \brief Set/Update the colors buffer with the source \p colors.
        /// \param[in] colors The source colors (rgba in range [0, 255]) to set the colors buffer.
        /// \return \b true if successful. \b false otherwise.
        /// \note The colors buffer, which has the same size as that of
        ///       the positions buffer, will not be resized by this method.
        virtual bool setColors(const RCBuffer<RCVector4ub>& colors);

        /// \brief Set/Update the colors buffer with the source \p colors from
        /// \p index.
        /// \param[in] index The index of the colors buffer from which to set colors.
        /// \param[in] colors The source colors (rgba in range [0, 255]) to set the colors buffer.
        /// \return \b true if successful. \b false otherwise.
        /// \note The colors buffer, which has the same size as that of
        ///       the positions buffer, will not be resized by this method.
        virtual bool setColorsAt(unsigned int index, const RCBuffer<RCVector4ub>& colors);

        /// \brief Get the colors buffer
        /// \return The colors buffer. A \b nullptr is returned if none exists.
        virtual const RCBuffer<RCVector4ub>* const getColors() const;

        /// \brief Set/Update the color value at the given \p index of the
        /// colors buffer with \p color.
        /// \param[in] index The index of the colors buffer item to set.
        /// \param[in] color The color value to set (rgba in range [0, 255]).
        /// \return \b true if successful. \b false otherwise.
        /// \note The colors buffer will not resize if \p index is out of range, in
        ///       which case the operation will fail.
        virtual bool setColorAt(unsigned int index, const RCVector4ub& color);

        /// \brief Get the color from the colors buffer at the given \p index.
        /// \param[in] index  The index of the colors buffer.
        /// \param[out] color The output color at \p index.
        /// \return \b true if successful. \b false otherwise.
        virtual bool getColorAt(unsigned int index, RCVector4ub& color) const;

        /// \brief Check if the intensities buffer is ready to use.
        /// \return \b true if intensities buffer exists. \b false otherwise.
        virtual bool hasIntensity() const;

        /// \brief Set/Update the intensities buffer with the source \p intensities.
        /// \param[in] intensities The source intensities in range [0, 255] to set the intensities buffer.
        /// \return \b true if successful. \b false otherwise.
        /// \note The intensities buffer, which has the same size as that of
        ///       the positions buffer, will not be resized by this method.
        virtual bool setIntensities(const RCBuffer<unsigned char>& intensities);

        /// \brief Set/Update the intensities buffer with the source \p intensities from \p index.
        /// \param[in] index The index of the intensities buffer from which to set intensities.
        /// \param[in] intensities The source intensities to set the intensities buffer, in range [0, 255].
        /// \return \b true if successful. \b false otherwise.
        /// \note The intensities buffer, which has the same size as that of
        ///       the positions buffer, will not be resized by this method.
        virtual bool setIntensitiesAt(unsigned int index, const RCBuffer<unsigned char>& intensities);

        /// \brief Get the intensities buffer.
        /// \return The intensities buffer. A \b nullptr is returned if none exists.
        virtual const RCBuffer<unsigned char>* const getIntensities() const;

        /// \brief Set/Update the intensity value at the given \p index of the intensities buffer with \p intensity
        /// \param[in] index The index of the intensities buffer item to set.
        /// \param[in] intensity The intensity value to set in range [0, 255].
        /// \return \b true if successful. \b false otherwise.
        /// \note The intensities buffer will not resize if \p index is out of range, in
        ///       which case the operation will fail.
        virtual bool setIntensityAt(unsigned int index, unsigned char intensity);

        /// \brief Get the intensity from the intensities buffer at the given \p index.
        /// \param[in] index  The index of the intensities buffer.
        /// \param[out] intensity The output intensity at \p index.
        /// \return \b true if successful. \b false otherwise.
        virtual bool getIntensityAt(unsigned int index, unsigned char& intensity) const;

        /// \brief Check if the segment-ids buffer is ready to use.
        /// \return \b true if segment-ids buffer exists. \b false otherwise.
        virtual bool hasSegmentId() const;

        /// \brief Set the total number of segments that all points belong to.
        /// \param[in] numberOfSegments The number of segments to set.
        /// \note The segment-id for each point should not exceed the number of
        /// segments.
        virtual void setNumberOfSegments(unsigned short numberOfSegments);

        /// \brief Get the total number of segments that all points belong to.
        /// \return The current number of segments.
        /// \note The segment-id for each point should not exceed the number of
        /// segments.
        virtual unsigned short getNumberOfSegments() const;

        /// \brief Check all segment-ids against the number of segments.
        /// \return \b true if all segment-ids do not exceed the number of
        /// segments. \b false otherwise.
        virtual bool areSegmentIdsValid() const;

        /// \brief Set/Update the segment-ids buffer with the source \p segmentIds.
        /// \param[in] segmentIds The source segment IDs to set the segment-ids buffer.
        /// \return \b true if successful. \b false otherwise.
        /// \note The segment-ids buffer, which has the same size as that of
        ///       the positions buffer, will not be resized by this method.
        virtual bool setSegmentIds(const RCBuffer<unsigned short>& segmentIds);

        /// \brief Set/Update the segment-ids buffer with the source \p
        /// segmentIds from \p index.
        /// \param[in] index The index of the segment-ids buffer from which to set segment-ids.
        /// \param[in] segmentIds The source segment IDs to set the segment-ids buffer.
        /// \return \b true if successful. \b false otherwise.
        /// \note The segment-ids buffer, which has the same size as that of
        ///       the positions buffer, will not be resized by this method.
        virtual bool setSegmentIdsAt(unsigned int index, const RCBuffer<unsigned short>& segmentIds);

        /// \brief Get the segment-ids buffer.
        /// \return The segment-ids buffer. A \b nullptr is returned if none exists.
        virtual const RCBuffer<unsigned short>* const getSegmentIds() const;

        /// \brief Set/Update the segment-id value at the given \p index of the
        /// segment-ids buffer with \p segmentId.
        /// \param[in] index The index of the segment-ids buffer item to set.
        /// \param[in] segmentId The segment-id value to set.
        /// \return \b true if successful. \b false otherwise.
        /// \note The segment-ids buffer will not resize if \p index is out of range, in
        ///       which case the operation will fail.
        virtual bool setSegmentIdAt(unsigned int index, unsigned short segmentId);

        /// \brief Get the segment-id from the segment-ids buffer at the given \p index.
        /// \param[in] index  The index of the segment-ids buffer.
        /// \param[out] segmentId The output segment-id at \p index.
        /// \return \b true if successful. \b false otherwise.
        virtual bool getSegmentIdAt(unsigned int index, unsigned short& segmentId) const;

        /// \brief Check if the time-stamps buffer is ready to .
        /// \return \b true if time-stamps buffer exists. \b false otherwise.
        virtual bool hasTimeStamp() const;

        /// \brief Set/Update the time-stamps buffer with the source \p timeStamps.
        /// \param[in] timeStamps The source time stamps to set the time-stamps buffer.
        /// \return \b true if successful. \b false otherwise.
        /// \note The time-stamps buffer, which has the same size as that of
        ///       the positions buffer, will not be resized by this method.
        virtual bool setTimeStamps(const RCBuffer<double>& timeStamps);

        /// \brief Set/Update the time-stamps buffer with the source \p timeStamps from \p index.
        /// \param[in] index The index of the time-stamps buffer from which to set time-stamps.
        /// \param[in] timeStamps The source time stamps to set the time-stamps buffer.
        /// \return \b true if successful. \b false otherwise.
        /// \note The time-stamps buffer, which has the same size as that of
        ///       the positions buffer, will not be resized by this method.
        virtual bool setTimeStampsAt(unsigned int index, const RCBuffer<double>& timeStamps);

        /// \brief Get the time-stamps buffer.
        /// \return The time-stamps buffer. A \b nullptr is returned if none exists.
        virtual const RCBuffer<double>* const getTimeStamps() const;

        /// \brief Set/Update the time-stamp value at the given \p index of the
        ///        time-stamps buffer with \p timeStamp.
        /// \param[in] index The index of the time-stamps buffer item to set.
        /// \param[in] timeStamp The time-stamp value to set.
        /// \return \b true if successful. \b false otherwise.
        /// \note The time-stamps buffer will not resize if \p index is out of range, in
        ///       which case the operation will fail.
        virtual bool setTimeStampAt(unsigned int index, double timeStamp);

        /// \brief Get the time-stamp from the time-stamps buffer at the given \p index.
        /// \param[in] index The index of the time-stamps buffer.
        /// \param[out] timeStamp The output time-stamp at \p index.
        /// \return \b true if successful. \b false otherwise.
        virtual bool getTimeStampAt(unsigned int index, double& timeStamp) const;

        /// \brief Set/Update the column index buffer with the source \p columnIndices.
        /// \param[in] columnIndices The source column index value to set the column index buffer
        /// \return \b true if successful. \b false otherwise.
        /// \note This field is not required for unstructured scan and optional for structured scan.
        /// \note The column index buffer, which has the same size as that of
        ///       the positions buffer, will not be resized by this method.
        bool setColumnIndices(const RCBuffer<unsigned int>& columnIndices);

        /// \brief Set/Update the column index buffer with the source \p columnIndices from \p index.
        /// \param[in] index The index of the column index buffer from which to set column index values.
        /// \param[in] columnIndices The source column index data to set the column index buffer.
        /// \return \b true if successful. \b false otherwise.
        /// \note This field is not required for unstructured scan and optional for structured scan.
        /// \note The column index buffer, which has the same size as that of
        ///       the positions buffer, will not be resized by this method.
        bool setColumnIndicesAt(std::uint64_t index, const RCBuffer<unsigned int>& columnIndices);

        /// \brief Get the column index buffer.
        /// \return The column index buffer. A \b nullptr is returned if none exists.
        const RCBuffer<unsigned int>* const getColumnIndices() const;

        /// \brief Set/Update the column index value at the given \p index of the column index buffer.
        /// \param[in] index The index of the column index buffer item to set.
        /// \param[in] columnIndex The column index value to set.
        /// \return \b true if successful. \b false otherwise.
        /// \note This field is not required for unstructured scan and optional for structured scan.
        /// \note The column index buffer will not resize if \p index is out of range, in
        ///       which case the operation will fail.
        bool setColumnIndexAt(std::uint64_t index, unsigned int columnIndex);

        /// \brief Get the column index value from the column index buffer at the given \p index.
        /// \param[in] index The index of the column index buffer.
        /// \param[out] columnIndex The output column index value at \p index.
        /// \return \b true if successful. \b false otherwise.
        bool getColumnIndexAt(std::uint64_t index, unsigned int& columnIndex) const;

        /// \brief Set/Update the row index buffer with the source \p rowIndices.
        /// \param[in] rowIndices The source row index data to set the row index buffer.
        /// \return \b true if successful. \b false otherwise.
        /// \note This field is not required for unstructured scan and optional for structured scan.
        /// \note The row index buffer, which has the same size as that of
        ///       the positions buffer, will not be resized by this method.
        bool setRowIndices(const RCBuffer<unsigned int>& rowIndices);

        /// \brief Set/Update the row index buffer with the source \p rowIndices from \p index.
        /// \param[in] index The index of the row index buffer from which to set row index values.
        /// \param[in] rowIndices The source row index data to set the row index buffer.
        /// \return \b true if successful. \b false otherwise.
        /// \note This field is not required for unstructured scan and optional for structured scan.
        /// \note The row index buffer, which has the same size as that of
        ///       the positions buffer, will not be resized by this method.
        bool setRowIndicesAt(std::uint64_t index, const RCBuffer<unsigned int>& rowIndices);

        /// \brief Get the row index buffer.
        /// \return The row index buffer. A \b nullptr is returned if none exists.
        const RCBuffer<unsigned int>* const getRowIndices() const;

        /// \brief Set/Update the row index value at the given \p index of the
        /// row index buffer.
        /// \param[in] index The index of the row index buffer item to set.
        /// \param[in] rowIndex The row index value to set.
        /// \return \b true if successful. \b false otherwise.
        /// \note  This field is not required for unstructured scan and optional for structured scan.
        /// \note The row index buffer will not resize if \p index is out of range, in
        ///       which case the operation will fail.
        bool setRowIndexAt(std::uint64_t index, unsigned int rowIndex);

        /// \brief Get the row index value from the row index buffer at the given \p index.
        /// \param[in] index The index of the row index buffer.
        /// \param[out] rowIndex The output row index value at \p index.
        /// \return \b true if successful. \b false otherwise.
        bool getRowIndexAt(std::uint64_t index, unsigned int& rowIndex) const;

#pragma region set attribute values from raw array

        /// \brief Set/Update the positions buffer with data copied from a \b double raw array.
        /// \param[in] points The bi-dimensional \double raw array with a column count of 3 storing the position data.
        ///               The columns represent Cartesian (x, y, z) or Spherical (r, a, e) coordinates of a point,
        ///               depending on what is set in setCoordinateType().
        /// \param[in] count The row count of \p points array.
        /// \return \b true if buffer is larger than or equal to the number of rows provided, and the operation is
        ///         successful. \b false otherwise.
        /// \note For a point in a structured scan, Cartesian coordinate of (0.0, 0.0, 0.0) or Spherical coordinate
        ///       with range 0.0 is regarded as a bad point without depth information.
        ///       Color information can still be attached to a point without depth information (such as sky point).
        /// \note The point buffer must be resized before positions are set.
        bool setPositions(const double (*const points)[3], std::uint64_t count);

        /// \brief Set/Update the positions buffer with data copied from a \b float raw array.
        /// \param[in] points The bi-dimensional \float raw array with a column count of 3 storing the position data.
        ///               The columns represent Cartesian (x, y, z) or Spherical (r, a, e) coordinates of a point,
        ///               depending on what is set in setCoordinateType().
        /// \param[in] count The row count of \p points array.
        /// \return \b true if buffer is larger than or equal to the number of rows provided, and the operation is
        ///         successful. \b false otherwise.
        /// \note For a point in a structured scan, Cartesian coordinate of (0.0, 0.0, 0.0) or Spherical coordinate
        ///       with range 0.0 is regarded as a bad point without depth information.
        ///       Color information can still be attached to a point without depth information (such as sky point).
        /// \note The point buffer must be resized before positions are set.
        bool setPositions(const float (*const points)[3], std::uint64_t count);

        /// \brief Set/Update the normals with data copied from a \b double raw array.
        /// \param[in] normals The bi-dimensional \double raw array with a column count of 3 storing the normals data.
        ///               The columns represent a (x, y, z) vector.
        /// \param[in] count The row count of \p normals array
        /// \note The point buffer must be resized before positions are set.
        /// \return \b true if buffer is larger than or equal to the number of rows provided, and the operation is
        ///         successful. \b false otherwise.
        bool setNormals(const double (*const normals)[3], std::uint64_t count);

        /// \brief Set/Update the normals with data copied from a \b float raw array.
        /// \param[in] normals The bi-dimensional \float raw array with a column count of 3 storing the normals data.
        ///               The columns represent a (x, y, z) vector.
        /// \param[in] count The row count of \p normals array.
        /// \return \b true if buffer is larger than or equal to the number of rows provided, and the operation is
        ///         successful. \b false otherwise.
        /// \note The point buffer must be resized before positions are set.
        bool setNormals(const float (*const normals)[3], std::uint64_t count);

        /// \brief Set the classifications with data copied from an \b unsigned \b char raw array.
        /// \param[in] classifications The single-dimensional \b unsigned \b char raw array in range [0, 255].
        /// \param[in] count The size of \p classifications array.
        /// \return \b true if the operation is successful. \b false otherwise.
        /// \note The point buffer must be resized before positions are set.
        bool setClassifications(const unsigned char* const classifications, std::uint64_t count);

        /// \brief Set the color with data copied from an \b unsigned \b char raw array.
        /// \param[in] rgb The bi-dimensional \b unsigned \b char raw array with a column count of 3 storing the color data.
        ///            The columns represent the R, G and B values of a color respectively, in range [0, 255].
        /// \param[in] count The row count of \p rgb array.
        /// \return \b true if the operation is successful. \b false otherwise.
        /// \note The point buffer must be resized before positions are set.
        bool setColors(const unsigned char (*const rgb)[3], std::uint64_t count);

        /// \brief Set the color with data copied from an \b unsigned \b char raw array.
        /// \param[in] rgba The bi-dimensional \b unsigned \b char raw array with a column count of 4 storing the color data.
        ///            The columns represent the R, G, B and A values of a color respectively, in range [0, 255].
        /// \param[in] count The row count of \p rgba array.
        /// \return \b true if the operation is successful. \b false otherwise.
        /// \note The point buffer must be resized before positions are set.
        bool setColors(const unsigned char (*const rgba)[4], std::uint64_t count);

        /// \brief Set the intensities with data copied from an \b unsigned \b char raw array.
        /// \param[in] intensities The single-dimensional \b unsigned \b char raw array, in range [0, 255].
        /// \param[in] count The size of \p intensities array.
        /// \return \b true if the operation is successful. \b false otherwise.
        /// \note The point buffer must be resized before positions are set.
        bool setIntensities(const unsigned char* const intensities, std::uint64_t count);

        /// \brief Set the segment ids with data copied from an \b unsigned \b short raw array.
        /// \param[in] segmentIds The single-dimensional \b unsigned \b short raw array
        /// \param[in] count The size of \p segmentIds array.
        /// \return \b true if the operation is successful. \b false otherwise.
        /// \note The point buffer must be resized before positions are set.
        bool setSegmentIds(const unsigned short* const segmentIds, std::uint64_t count);

        /// \brief Set the time stamps with data copied from a \b double raw array.
        /// \param[in] timeStamps The single-dimensional \b double raw array
        /// \param[in] count The size of \p timeStamps array.
        /// \return \b true if the operation is successful. \b false otherwise.
        /// \note The point buffer must be resized before positions are set.
        bool setTimeStamps(const double* const timeStamps, std::uint64_t count);

        /// \brief Set the column indices with data copied from an \b unsigned \b int raw array.
        /// \param[in] columnIndices The single-dimensional \b unsigned \b int raw array
        /// \param[in] count The size of \p columnIndices array.
        /// \return \b true if the operation is successful. \b false otherwise.
        /// \note The point buffer must be resized before positions are set.
        bool setColumnIndices(const unsigned int* const columnIndices, std::uint64_t count);

        /// \brief Set the row indices with data copied from an \b unsigned \b int raw array.
        /// \param[in] rowIndices The single-dimensional \b unsigned \b int raw array
        /// \param[in] count The size of \p rowIndices array.
        /// \return \b true if the operation is successful. \b false otherwise.
        /// \note The point buffer must be resized before positions are set.
        bool setRowIndices(const unsigned int* const rowIndices, std::uint64_t count);

#pragma endregion
    };

}}}    // namespace Autodesk::RealityComputing::Data
