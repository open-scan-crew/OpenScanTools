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

#include "RCScopedPointer.h"
#include "managedAPI/Wrap_RCBoundBlock3d.h"
#include "managedAPI/Wrap_RCPlane.h"
#include "managedAPI/Wrap_RCVector.h"
#include "managedAPI/Wrap_RCBox.h"
#include "managedAPI/Wrap_RCTransform.h"
#include "managedAPI/Wrap_RCCylinder.h"

namespace Autodesk { namespace RealityComputing { namespace Managed {
    //We use static_cast to default type (for scoped enums) to avoid intelisence 'errors' for MS Visual Studio
    public enum class FilterType
    {
        Unknown = static_cast<int>(NS_RCData::FilterType::Unknown),
        Plane = static_cast<int>(NS_RCData::FilterType::Plane),
        MultiPlane = static_cast<int>(NS_RCData::FilterType::MultiPlane),
        Box = static_cast<int>(NS_RCData::FilterType::Box),
        Circle = static_cast<int>(NS_RCData::FilterType::Circle),
        Polygon = static_cast<int>(NS_RCData::FilterType::Polygon),
        Inverse = static_cast<int>(NS_RCData::FilterType::Inverse),
        Planarslab = static_cast<int>(NS_RCData::FilterType::Planarslab),
        Multiple = static_cast<int>(NS_RCData::FilterType::Multiple),
        Customized = static_cast<int>(NS_RCData::FilterType::Customized)
    };

    public enum class FilterSetOperation
    {
        Intersection = static_cast<int>(NS_RCData::FilterSetOperation::Intersection),
        Union = static_cast<int>(NS_RCData::FilterSetOperation::Union)
    };

    public ref class RCSpatialFilter abstract
    {
    public:
        enum class FilterResult
        {
            Inside = static_cast<int>(NS_RCData::RCSpatialFilter::FilterResult::Inside),
            Outside = static_cast<int>(NS_RCData::RCSpatialFilter::FilterResult::Outside),
            Intersection = static_cast<int>(NS_RCData::RCSpatialFilter::FilterResult::Intersection)
        };

    public:

        /// \brief  Checks to see if a given box is inside, outside, or on the edge of this filter
        /// \param  box The box with which the intersection mode is to be determined
        /// \return Returns whether a given target is inside, outside, or in the edge of this filter
        virtual FilterResult CheckBox(RCBox box) { throw gcnew NotImplementedException; };

        /// \brief  Checks to see if a given point is inside, or outside of filter
        /// \param  point The point with which the intersection mode is to be determined
        /// \return Returns whether a given target is inside, outside, or in the edge of this filter
        virtual FilterResult CheckPoint(RCVector3d point) { throw gcnew NotImplementedException; };

        //TODO: make 4x4 transform matrix?
        /// \brief  Transforms the spatial filter by the given 4x4 matrix
        /// \param  transform4x4 The transformation to be applied to the new instance of this filter
        /// \return Returns a reference to current filter after transformation
        virtual RCSpatialFilter^ TransformBy(RCTransform% transform4x4) { throw gcnew NotImplementedException; };
        //end of TODO

        /// \brief  Gets the predefined filter type that represents this filter
        /// \return Returns the type of a predefined filter
        virtual FilterType GetType() { throw gcnew NotImplementedException; };

        /// \brief  Gets the inverted filter of this spatial filter
        /// \return Returns the inverted filter of this spatial filter
        virtual RCSpatialFilter^ GetInverse() { throw gcnew NotImplementedException; };

        /// \brief Returns ReCap object corresponding to the wrapped filter to use in C++ code
        virtual NS_RCData::RCSharedPtr<NS_RCData::RCSpatialFilter> ToRecapObject() { throw gcnew NotImplementedException; };
    };

    /// \brief  A class to represent the inverse of another spatial filter, used to cull out points that are returned by
    ///         the point cloud queries.
    public ref class RCInverseFilter : public RCSpatialFilter
    {
    public:
        RCInverseFilter();

    public:
        ///\brief Constructs an inverse filter of the given spatial filter
        ///\param filter Spatial filter from which to construct an inverse filter
        RCInverseFilter(RCSpatialFilter^ filter);

        RCInverseFilter(const NS_RCData::RCSharedPtr <NS_RCData::RCSpatialFilter>& filter);

        /// \brief Checks to see if a given box is inside, outside, or on the edge of this filter
        /// \note Returns the inverse of the original filter's checkBox() result
        RCSpatialFilter::FilterResult CheckBox(RCBox box) override;

        /// \brief Checks to see if a given point is inside, or outside of filter
        /// \note Returns the inverse of the original filter's checkPoint() result
        RCSpatialFilter::FilterResult CheckPoint(RCVector3d point) override;

        /// \brief  Transforms the spatial filter by the given 4x4 matrix
        /// \param  transform4x4 The transformation to be applied to the new instance of this filter
        /// \return Returns a reference to current filter after transformation
        RCSpatialFilter^ TransformBy(RCTransform% transform4x4) override;

        ///\brief Returns the type of the filter.
        FilterType GetType() override;

        /// \brief Returns ReCap object corresponding to the wrapped filter to use in C++ code
        NS_RCData::RCSharedPtr <NS_RCData::RCSpatialFilter> ToRecapObject() override;

    private:
        RCScopedPointer<NS_RCData::RCSpatialFilter> mFilter;
    };

    /// <description>
    /// \brief A planar implementation of the spatial filter - on the normal side of the plane is inside, on the reverse is outside.
    /// Note that this is an infinite plane
    /// </description>
    public ref class RCPlaneSpatialFilter : public RCSpatialFilter
    {
    public:
        RCPlaneSpatialFilter();

    public:
        ///\brief constructs a plane filter from a plane
        RCPlaneSpatialFilter(RCPlane plane);

        /// \brief Given a box boundary, checks if the infinite plane of the filter intersects with the box at all.
        FilterResult CheckBox(RCBox box) override;

        /// \brief Checks if a given point is in front of the infinite plane.
        /// The front being defined by the direction the plane normal is pointing towards.
        FilterResult CheckPoint(RCVector3d point) override;

        /// \brief Transforms the plane according to the rotation/translation specified in the 4x4 matrix.
        RCSpatialFilter^ TransformBy(RCTransform% transform4x4) override;

        ///\brief Returns the type of the filter.
        FilterType GetType() override;

        RCSpatialFilter^ GetInverse() override;

        /// \brief Returns ReCap object corresponding to the wrapped filter to use in C++ code
        NS_RCData::RCSharedPtr<NS_RCData::RCSpatialFilter> ToRecapObject() override;

    private:
        RCScopedPointer<NS_RCData::RCSpatialFilter> mFilter;
    };

    /// <description>
    /// A circular filtering area. In reality, an infinite cylinder.
    /// </description>
    public ref class RCCylinderSpatialFilter : public RCSpatialFilter
    {
    public:
        RCCylinderSpatialFilter();
    public:

        ///\brief Defines a cylinder spatial filter using a cylinder
        RCCylinderSpatialFilter(RCCylinder^ cylinder);

        RCCylinderSpatialFilter(RCVector3d origin, RCVector3d axisOfSymmetry, double radius);

        RCCylinderSpatialFilter(RCVector3d origin, RCVector3d axisOfSymmetry);

        ///\brief Checks if the bounding box intersect with the RCCylinderSpatialFilter
        FilterResult CheckBox(RCBox box) override;

        ///\brief Checks if a given point point intersects/falls within the RCCylinderSpatialFilter
        FilterResult CheckPoint(RCVector3d point) override;

        ///\brief Transforms the RCCylinderSpatialFilter with the provided matrix transform4x4
        RCSpatialFilter^ TransformBy(RCTransform% transform4x4) override;

        ///\brief Returns the type of the filter.
        FilterType GetType() override;

        RCSpatialFilter^ GetInverse() override;

        /// \brief Returns ReCap object corresponding to the wrapped filter to use in C++ code
        NS_RCData::RCSharedPtr<NS_RCData::RCSpatialFilter> ToRecapObject() override;

        RCCylinder^ GetCylinder();

    private:
        RCScopedPointer<NS_RCData::RCSpatialFilter> mFilter;
    };

    /// <description>
    /// \brief A spatial filter defined by a 3d bounding volume.
    /// </description>
    public ref class RCBoxSpatialFilter : public RCSpatialFilter
    {
    public:
        RCBoxSpatialFilter();

    public:
        ///\brief Constructs an axis aligned box spatial filter taking in the min and max points.
        ///\param boxMin Min point of axis aligned box
        ///\param boxMax Max point of axis aligned box
        RCBoxSpatialFilter(RCVector3d boxMin, RCVector3d boxMax);

        ///\brief Checks if the RCBoxSpatialFilter intersects with the axis-aligned box
        FilterResult CheckBox(RCBox box) override;

        ///\brief Checks if the RCBoxSpatialFilter intersects with the point as described by point
        FilterResult CheckPoint(RCVector3d point) override;

        ///\brief Transforms the box by the transform as provided by transform4x4
        RCSpatialFilter^ TransformBy(RCTransform% transform4x4) override;

        ///\briefReturns the type of the filter.
        FilterType GetType() override;

        RCSpatialFilter^ GetInverse() override;

        /// \brief Returns ReCap object corresponding to the wrapped filter to use in C++ code
        NS_RCData::RCSharedPtr<NS_RCData::RCSpatialFilter> ToRecapObject() override;

        ///\brief Get the 3d bounding volume defining the spatial filter.
        RCBoundBlock3d^ GetBox();
    private:
        RCScopedPointer<NS_RCData::RCSpatialFilter> mFilter;
    };


    /// <description>
    /// Plane Slab defines the area between two infinite planes. It is defined by a center plane and a thickness
    /// Point between or on the two infinite planes is inside, otherwise is outside.
    /// </description>
    public ref class RCPlanarSlabSpatialFilter : public RCSpatialFilter
    {
    public:
        RCPlanarSlabSpatialFilter();
    public:
        ///\brief constructs a plane slab filter from a plane and plane thickness, i.e. the distance between the two planar surfaces
        ///\param plane The plane located in the center of the planar slab
        ///\param thickness Distance between the two planar surfaces of the planar slab,
        ///                 This value should be greater than 0. If negative value is input, it will be set to 0
        RCPlanarSlabSpatialFilter(RCPlane% plane, double thickness);

        /// \brief Given a box boundary, checks if the infinite plane of the filter intersects with the box at all.
        FilterResult CheckBox(RCBox box) override;
        /// \brief Checks if a given point is in front of the infinite plane.
        /// The front being defined by the direction the plane normal is pointing towards.
        FilterResult CheckPoint(RCVector3d point) override;
        /// \brief Transforms the plane according to the rotation/translation specified in the 4x4 matrix.
        RCSpatialFilter^ TransformBy(RCTransform% transform4x4) override;

        ///\brief Returns the type of the filter.
        FilterType GetType() override;

        RCSpatialFilter^ GetInverse() override;

        /// \brief Returns ReCap object corresponding to the wrapped filter to use in C++ code
        NS_RCData::RCSharedPtr<NS_RCData::RCSpatialFilter> ToRecapObject() override;

        RCPlane GetPlane();
        double GetThickness();
    private:
        RCScopedPointer<NS_RCData::RCSpatialFilter> mFilter;
    };

    /// <description>
    /// Defines a 2D polygon of infinite depth, and uses that as a basis for spatial filtering
    /// The mental model you should have while using the filter is that of a fence tool in a GUI application.
    /// You need to pass in the appropriate camera and model transform so the 2d points you place on the viewport can
    /// be used to determine which points in the point cloud you are selecting.
    /// </description>
    public ref class RCPolygonSpatialFilter : public RCSpatialFilter
    {
    public:
        RCPolygonSpatialFilter();

    public:
        ///\brief Creates a RCPolygonSpatialFilter with model-view-projection matrix
        ///\param polyPoints The list of points to filter, in normalized device coordinates, i.e. x,y is between [-1, 1]
        ///\param viewMatrix View matrix of the camera, defined by camera rotation and translation
        ///\param projectionMatrix Projection matrix of the camera, it can be either perspective or orthographic
        RCPolygonSpatialFilter(List<RCVector2d>^ polyPoints, RCTransform% viewMatrix, RCProjection^ projectionMatrix);

        ///\brief Checks if the box intersects with the filter
        FilterResult CheckBox(RCBox box) override;

        ///\brief Checks if the given point intersects with the filter
        FilterResult CheckPoint(RCVector3d point) override;

        ///\brief Transforms the filter geometry by the matrix transform4x4
        RCSpatialFilter^ TransformBy(RCTransform% transform4x4) override;

        ///\brief Returns the type of the filter.
        FilterType GetType() override;

        RCSpatialFilter^ GetInverse() override;

        /// \brief Returns ReCap object corresponding to the wrapped filter to use in C++ code
        NS_RCData::RCSharedPtr<NS_RCData::RCSpatialFilter> ToRecapObject() override;

        List<RCVector2d>^ GetPolygonVertices();
        RCTransform GetViewMatrix();
        RCProjection^ GetProjectionMatrix();
    private:
        RCScopedPointer<NS_RCData::RCSpatialFilter> mFilter;
    };

    /// <description>
    /// \brief This class concatenates multiple RCSpatialFilters
    /// Conceptually you can think of this as a group of RCSpatialFilters of different shapes.
    /// </description>
    public ref class RCMultiSpatialFilter : public RCSpatialFilter
    {
    public:
        RCMultiSpatialFilter();

    public:
        RCMultiSpatialFilter(FilterSetOperation setOp);

        ///\brief Adds a filter into the MultiSpatialFilter.
        void AddFilter(RCSpatialFilter^ filter);

        ///\brief Checks if the box described intersects any of the RCSpatialFilters.
        /// if when constructed FilterSetOperation is set to Union, then we will test for intersection with the union of all the member filters
        /// Otherwise we will test for intersection with the intersection of all the member filters
        FilterResult CheckBox(RCBox box) override;

        ///\brief Checks if the point describes intersects/is within/is outside of any/all of the RCSpatialFilters owned by RCMultiSpatialFilter
        /// if when constructed FilterSetOperation is set to Union, then we will test for intersection with the union of all the member filters
        /// Otherwise we will test for intersection with the intersection of all the member filters
        FilterResult CheckPoint(RCVector3d point) override;

        ///\brief Transforms uniformly all the RCSpatialFilters owned by the RCMultiSpatialFilter by the given transform
        RCSpatialFilter^ TransformBy(RCTransform% transform4x4) override;

        ///\brief Returns the type of the filter.
        FilterType GetType() override;

        RCSpatialFilter^ GetInverse() override;

        /// \brief Returns ReCap object corresponding to the wrapped filter to use in C++ code
        NS_RCData::RCSharedPtr<NS_RCData::RCSpatialFilter> ToRecapObject() override;
    private:
        RCScopedPointer<NS_RCData::RCSpatialFilter> mFilter;
    };

    /// <description>
    /// \brief A list of planes implementation of the spatial filter - the point on the normal side of all planes is inside.
    /// Conceptually a MultiPlaneSpatialFilter can be seen as a composition of singular PlaneSpatialFilters
    /// </description>
    public ref class RCMultiPlaneSpatialFilter : public RCSpatialFilter
    {
    public:
        RCMultiPlaneSpatialFilter();

    public:
        ///\brief Checks if a box defined by the inputs is in intersection with any of the planes defined by this filter.
        FilterResult CheckBox(RCBox box) override;

        /// \brief Checks if a point is in front of *all* the planes contained by the MultiPlaneFilter
        /// The front being defined by the direction the plane normal is pointing towards.
        FilterResult CheckPoint(RCVector3d point) override;

        ///\brief Transforms all of the planes uniformly with the matrix passed in as argument
        /// If you want to transform a specific plane, please look at `getPlanes`
        RCSpatialFilter^ TransformBy(RCTransform% transform4x4) override;

        ///\brief Returns the type of the filter.
        FilterType GetType() override;

        RCSpatialFilter^ GetInverse() override;

        /// \brief Returns ReCap object corresponding to the wrapped filter to use in C++ code
        NS_RCData::RCSharedPtr<NS_RCData::RCSpatialFilter> ToRecapObject() override;

        ///\brief Constructs and adds a single new plane for this filter
        void AddPlane(RCPlane% plane);

        void AddPlanes(List<RCPlane>^ planes);

        ///\brief Resets the planes owned by the MultiPlaneSpatialFilter to 0
        void ClearPlanes();

        ///\brief Returns the number of planes the MultiPlaneSpatialFilter currently owns.
        unsigned int GetNumberOfPlanes();

        ///\brief gets the list of planes that the MultiPlaneSpatialFilter currently owns.
        /// This list can be manipulated for fine grained control of the planes within the filter.
        List<RCPlane>^ GetPlanes();
    private:
        RCScopedPointer<NS_RCData::RCSpatialFilter> mFilter;
    };
}}}    // namespace Autodesk::RealityComputing::Managed
