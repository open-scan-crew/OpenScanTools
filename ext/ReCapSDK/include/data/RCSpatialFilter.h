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

#include <geplane.h>
#include <geblok3d.h>

#include <foundation/RCUUID.h>
#include <foundation/RCVector.h>
#include <foundation/RCBox.h>
#include <foundation/RCTransform.h>
#include <foundation/RCCylinder.h>
#include <foundation/RCBuffer.h>
#include <foundation/RCSharedPtr.h>
#include <data/RCFilterDef.h>

namespace Autodesk { namespace RealityComputing { namespace Data {

    using Autodesk::RealityComputing::Foundation::RCBox;
    using Autodesk::RealityComputing::Foundation::RCBuffer;
    using Autodesk::RealityComputing::Foundation::RCCylinder;
    using Autodesk::RealityComputing::Foundation::RCProjection;
    using Autodesk::RealityComputing::Foundation::RCSharedPtr;
    using Autodesk::RealityComputing::Foundation::RCTransform;
    using Autodesk::RealityComputing::Foundation::RCUUID;
    using Autodesk::RealityComputing::Foundation::RCVector2d;
    using Autodesk::RealityComputing::Foundation::RCVector3d;

    /// <description>
    /// Represents the type of predefined filter type.
    /// </description>
    enum class FilterType
    {
        Unknown = 0,
        Plane,
        MultiPlane,
        Box,
        Circle,
        Polygon,
        Inverse,
        Planarslab,
        Multiple,
        Customized
    };

    /// <description>
    /// Represents how multiple filters take effect together.
    /// </description>
    enum class FilterSetOperation
    {
        Intersection,
        Union
    };

    /// \brief  A class to represent a spatial filter, used to cull out points that are returned by
    ///         the point cloud queries.
    class RC_FILTERS_API RCSpatialFilter
    {
    public:
        /// \brief  Represents whether a given target is inside, outside, or in the edge of a filter.
        enum class FilterResult
        {
            Inside = 0,
            Outside,
            Intersection
        };

        /// \brief Default virtual destructor
        virtual ~RCSpatialFilter() = default;

        /// \brief  Checks to see if a given box is inside, outside, or on the edge of this filter
        /// \param  box The box with which the intersection mode is to be determined
        /// \return Returns whether a given target is inside, outside, or in the edge of this filter
        virtual FilterResult checkBox(const RCBox& box) const = 0;

        /// <description>
        /// Checks to see if a given point list is inside, or outside of filter for each point.
        /// </description>
        RCBuffer<FilterResult> checkPoints(const RCBuffer<RCVector3d>& points) const;

        /// \brief  Checks to see if a given point is inside, or outside of filter
        /// \param  point The point with which the intersection mode is to be determined
        /// \return Returns whether a given target is inside, outside, or in the edge of this filter
        virtual FilterResult checkPoint(const RCVector3d& point) const = 0;

        /// \brief  Transforms the spatial filter by the given 4x4 matrix
        /// \param  transform4x4 The transformation to be applied to the new instance of this filter
        /// \return Returns a reference to current filter after transformation
        virtual RCSpatialFilter& transformBy(const RCTransform& transform4x4) = 0;

        /// \brief  Creates a clone of this filter. Multiple clones of this filter can be
        ///         created if multi-threaded filtering is enabled.
        /// \return Returns the cloned instance of this filter
        virtual RCSharedPtr<RCSpatialFilter> clone() const = 0;

        /// \brief  Gets the predefined filter type that represents this filter
        /// \return Returns the type of a predefined filter
        virtual FilterType getType() const;

        /// \brief  Gets the inverted filter of this spatial filter
        /// \return Returns the inverted filter of this spatial filter
        RCSharedPtr<RCSpatialFilter> getInverse() const;
    };

    /// \brief  A class to represent the inverse of another spatial filter, used to cull out points that are returned by
    ///         the point cloud queries.
    class RC_FILTERS_API RCInverseFilter : public RCSpatialFilter
    {
        RC_PIMPL_DECLARATION(RCInverseFilter)

    public:
        ///\brief Constructs an inverse filter of the given spatial filter
        ///\param filter Spatial filter from which to construct an inverse filter
        explicit RCInverseFilter(const RCSharedPtr<RCSpatialFilter>& filter);

        /// \brief Checks to see if a given box is inside, outside, or on the edge of this filter
        /// \note Returns the inverse of the original filter's checkBox() result
        FilterResult checkBox(const RCBox& box) const override;

        /// \brief Checks to see if a given point is inside, or outside of filter
        /// \note Returns the inverse of the original filter's checkPoint() result
        FilterResult checkPoint(const RCVector3d& point) const override;

        /// \brief  Transforms the spatial filter by the given 4x4 matrix
        /// \param  transform4x4 The transformation to be applied to the new instance of this filter
        /// \return Returns a reference to current filter after transformation
        RCSpatialFilter& transformBy(const RCTransform& transform4x4) override;

        ///\brief Returns the type of the filter.
        FilterType getType() const override;

        /// \brief Creates a new instance of this filter
        RCSharedPtr<RCSpatialFilter> clone() const override;
    };

    /// <description>
    /// \brief A planar implementation of the spatial filter - on the normal side of the plane is inside, on the reverse is outside.
    /// Note that this is an infinite plane
    /// </description>
    class RC_FILTERS_API RCPlaneSpatialFilter : public RCSpatialFilter
    {
        RC_PIMPL_DECLARATION(RCPlaneSpatialFilter)

    public:
        ///\brief constructs a plane filter from a plane
        RCPlaneSpatialFilter(const AcGePlane& plane);

        /// \brief Given a box boundary, checks if the infinite plane of the filter intersects with the box at all.
        FilterResult checkBox(const RCBox& box) const override;
        /// \brief Checks if a given point is in front of the infinite plane.
        /// The front being defined by the direction the plane normal is pointing towards.
        FilterResult checkPoint(const RCVector3d& point) const override;
        /// \brief Transforms the plane according to the rotation/translation specified in the 4x4 matrix.
        virtual RCSpatialFilter& transformBy(const RCTransform& transform4x4) override;
        /// \brief Creates a new instance of this filter
        virtual RCSharedPtr<RCSpatialFilter> clone() const override;

        ///\brief Returns the type of the filter.
        FilterType getType() const override;
    };

    /// <description>
    /// \brief A list of planes implementation of the spatial filter - the point on the normal side of all planes is inside.
    /// Conceptually a MultiPlaneSpatialFilter can be seen as a composition of singular PlaneSpatialFilters
    /// </description>
    class RC_FILTERS_API RCMultiPlaneSpatialFilter : public RCSpatialFilter
    {
        RC_PIMPL_DECLARATION(RCMultiPlaneSpatialFilter)

    public:
        ///\brief Checks if a box defined by the inputs is in intersection with any of the planes defined by this filter.
        FilterResult checkBox(const RCBox& box) const override;
        /// \brief Checks if a point is in front of *all* the planes contained by the MultiPlaneFilter
        /// The front being defined by the direction the plane normal is pointing towards.
        virtual FilterResult checkPoint(const RCVector3d& point) const override;
        ///\brief Transforms all of the planes uniformly with the matrix passed in as argument
        /// If you want to transform a specific plane, please look at `getPlanes`
        virtual RCSpatialFilter& transformBy(const RCTransform& transform4x4) override;
        ///\brief Create a new instance of this filter, as well as all the planes it is in charge of
        virtual RCSharedPtr<RCSpatialFilter> clone() const override;
        ///\brief Returns the type of the filter.
        virtual FilterType getType() const override;
        ///\brief Constructs and adds a single new plane for this filter
        void addPlane(const AcGePlane& plane);
        ///\brief Takes in an array of doubles and uses them to create and add `planeCount` new planes into the filter.
        /// The number of elements must be at least 4 * planeCount. Each group of 4 elements must be laid out in [x, y, z, w] order.
        /// elements past 4 * planeCount will not be used.
        void addPlanes(const RCBuffer<AcGePlane>& planes);

        ///\brief Resets the planes owned by the MultiPlaneSpatialFilter to 0
        void clearPlanes();
        ///\brief Returns the number of planes the MultiPlaneSpatialFilter currently owns.
        unsigned int getNumberOfPlanes() const;

        ///\brief gets the list of planes that the MultiPlaneSpatialFilter currently owns.
        /// This list can be manipulated for fine grained control of the planes within the filter.
        const RCBuffer<AcGePlane>& getPlanes() const;
    };

    /// <description>
    /// \brief This class concatenates multiple RCSpatialFilters
    /// Conceptually you can think of this as a group of RCSpatialFilters of different shapes.
    /// </description>
    class RC_FILTERS_API RCMultiSpatialFilter : public RCSpatialFilter
    {
        RC_PIMPL_DECLARATION(RCMultiSpatialFilter)

    public:
        RCMultiSpatialFilter(FilterSetOperation setOp);

        ///\brief Adds a filter into the MultiSpatialFilter.
        void addFilter(const RCSharedPtr<RCSpatialFilter>& filter);

        ///\brief Checks if the box described intersects any of the RCSpatialFilters.
        /// if when constructed FilterSetOperation is set to Union, then we will test for intersection with the union of all the member filters
        /// Otherwise we will test for intersection with the intersection of all the member filters
        FilterResult checkBox(const RCBox& box) const override;
        ///\brief Checks if the point describes intersects/is within/is outside of any/all of the RCSpatialFilters owned by RCMultiSpatialFilter
        /// if when constructed FilterSetOperation is set to Union, then we will test for intersection with the union of all the member filters
        /// Otherwise we will test for intersection with the intersection of all the member filters
        virtual FilterResult checkPoint(const RCVector3d& point) const override;
        ///\brief Transforms uniformly all the RCSpatialFilters owned by the RCMultiSpatialFilter by the given transform
        virtual RCSpatialFilter& transformBy(const RCTransform& transform4x4) override;
        ///\brief Clones the RCMultiSpatialFilter and all the RCSpatialFilters it owns.
        virtual RCSharedPtr<RCSpatialFilter> clone() const override;
        ///\brief Returns the type of the filter.
        virtual FilterType getType() const override;
    };

    /// <description>
    /// \brief A spatial filter defined by a 3d bounding volume.
    /// </description>
    class RC_FILTERS_API RCBoxSpatialFilter : public RCSpatialFilter
    {
        RC_PIMPL_DECLARATION(RCBoxSpatialFilter)

    public:
        ///\brief Constructs an axis aligned box spatial filter taking in the min and max points.
        ///\param boxMin Min point of axis aligned box
        ///\param boxMax Max point of axis aligned box
        RCBoxSpatialFilter(const RCVector3d& boxMin, const RCVector3d& boxMax);

        ///\brief Checks if the RCBoxSpatialFilter intersects with the axis-aligned box
        FilterResult checkBox(const RCBox& box) const override;
        ///\brief Checks if the RCBoxSpatialFilter intersects with the point as described by point
        virtual FilterResult checkPoint(const RCVector3d& point) const override;
        ///\brief Transforms the box by the transform as provided by transform4x4
        virtual RCSpatialFilter& transformBy(const RCTransform& transform4x4) override;
        ///\brief Clones the RCBoxSpatialFilter
        virtual RCSharedPtr<RCSpatialFilter> clone() const override;
        ///\briefReturns the type of the filter.
        virtual FilterType getType() const override;

        ///\brief Get the 3d bounding volume defining the spatial filter.
        AcGeBoundBlock3d getBox() const;
    };

    /// <description>
    /// Defines a 2D polygon of infinite depth, and uses that as a basis for spatial filtering
    /// The mental model you should have while using the filter is that of a fence tool in a GUI application.
    /// You need to pass in the appropriate camera and model transform so the 2d points you place on the viewport can
    /// be used to determine which points in the point cloud you are selecting.
    /// </description>
    class RC_FILTERS_API RCPolygonSpatialFilter : public RCSpatialFilter
    {
        RC_PIMPL_DECLARATION(RCPolygonSpatialFilter)

    public:
        ///\brief Creates a RCPolygonSpatialFilter with model-view-projection matrix
        ///\param polyPoints The list of points to filter, in normalized device coordinates, i.e. x,y is between [-1, 1]
        ///\param viewMatrix View matrix of the camera, defined by camera rotation and translation
        ///\param projectionMatrix Projection matrix of the camera, it can be either perspective or orthographic
        RCPolygonSpatialFilter(const RCBuffer<RCVector2d>& polyPoints, const RCTransform& viewMatrix, const RCProjection& projectionMatrix);

        ///\brief Checks if the box intersects with the filter
        FilterResult checkBox(const RCBox& box) const override;
        ///\brief Checks if the given point intersects with the filter
        virtual FilterResult checkPoint(const RCVector3d& point) const override;
        ///\brief Transforms the filter geometry by the matrix transform4x4
        virtual RCSpatialFilter& transformBy(const RCTransform& transform4x4) override;
        ///\brief Creates a new instance of the RCPolygonSpatialFilter
        virtual RCSharedPtr<RCSpatialFilter> clone() const override;
        ///\brief Returns the type of the filter.
        virtual FilterType getType() const override;

        const RCBuffer<RCVector2d>& getPolygonVertices() const;
        const RCTransform& getViewMatrix() const;
        const RCProjection& getProjectionMatrix() const;
    };

    /// <description>
    /// Plane Slab defines the area between two infinite planes. It is defined by a center plane and a thickness
    /// Point between or on the two infinite planes is inside, otherwise is outside.
    /// </description>
    class RC_FILTERS_API RCPlanarSlabSpatialFilter : public RCSpatialFilter
    {
        RC_PIMPL_DECLARATION(RCPlanarSlabSpatialFilter)

    public:
        ///\brief constructs a plane slab filter from a plane and plane thickness, i.e. the distance between the two planar surfaces
        ///\param plane The plane located in the center of the planar slab
        ///\param thickness Distance between the two planar surfaces of the planar slab,
        ///                 This value should be greater than 0. If negative value is input, it will be set to 0
        RCPlanarSlabSpatialFilter(const AcGePlane& plane, double thickness);

        /// \brief Given a box boundary, checks if the infinite plane of the filter intersects with the box at all.
        FilterResult checkBox(const RCBox& box) const override;
        /// \brief Checks if a given point is in front of the infinite plane.
        /// The front being defined by the direction the plane normal is pointing towards.
        FilterResult checkPoint(const RCVector3d& point) const override;
        /// \brief Transforms the plane according to the rotation/translation specified in the 4x4 matrix.
        virtual RCSpatialFilter& transformBy(const RCTransform& transform4x4) override;
        /// \brief Creates a new instance of this filter
        virtual RCSharedPtr<RCSpatialFilter> clone() const override;

        ///\brief Returns the type of the filter.
        FilterType getType() const override;

        const AcGePlane& getPlane() const;
        double getThickness() const;
    };

    /// <description>
    /// A circular filtering area. In reality, an infinite cylinder.
    /// </description>
    class RC_FILTERS_API RCCylinderSpatialFilter : public RCSpatialFilter
    {
        RC_PIMPL_DECLARATION(RCCylinderSpatialFilter)

    public:
        ///\brief Defines a cylinder spatial filter using a cylinder
        RCCylinderSpatialFilter(const RCCylinder& cylinder);
        RCCylinderSpatialFilter(const RCVector3d& origin, const RCVector3d& axisOfSymmetry, double radius);

        ///\brief Checks if the bounding box intersect with the RCCylinderSpatialFilter
        FilterResult checkBox(const RCBox& box) const override;
        ///\brief Checks if a given point point intersects/falls within the RCCylinderSpatialFilter
        virtual FilterResult checkPoint(const RCVector3d& point) const override;
        ///\brief Transforms the RCCylinderSpatialFilter with the provided matrix transform4x4
        virtual RCSpatialFilter& transformBy(const RCTransform& transform4x4) override;
        ///\brief Creates a new instance of RCCylinderSpatialFilter
        virtual RCSharedPtr<RCSpatialFilter> clone() const override;
        ///\brief Returns the type of the filter.
        virtual FilterType getType() const override;

        RCCylinder getCylinder() const;
    };
}}}    // namespace Autodesk::RealityComputing::Data
