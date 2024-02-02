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

#include <cstdint>

#include <geplane.h>

#include <foundation/RCCommonDef.h>
#include <foundation/RCVector.h>

/**
  @namespace Autodesk

  We generally put all the Autodesk related API stuff into this namespace.

  @namespace RealityComputing

  We generally put all the RC related API stuff into this namespace.

  @namespace Foundation

  We generally put all the Foundation API stuff into this namespace.
*/

class AcGeRay3d;

namespace Autodesk { namespace RealityComputing { namespace Foundation {
    class RCTransform;
    class LegacyBox2f;

    /// \brief Two-dimensional axis aligned bounding box.
    /**
      This struct is a foundational data structure that is used to facilitate
      things and stuff
    */
    struct RC_COMMON_API RCBox2D
    {
    private:
        RCVector2d minPoint;
        RCVector2d maxPoint;

    public:
        /**
          @brief
          Default constructor
        */
        RCBox2D();
        /**
          @brief
          Use minimum and maximum point to construct a bounding box

          @param min
          The minimum point of the bounding box

          @param max
          The maximum point of the bounding box
        */
        RCBox2D(const RCVector2d& min, const RCVector2d& max);

        void clear();

        /**
          @brief
          This sets the bounds of the RCBox, effectively resizing it.

          @param min
          The new minimum boundaries of this RCBox2D instance.

          @param max
          The new maximum boundaries of this RCBox2D instance.

          @result
          A reference to itself.
        */
        RCBox2D& setBounds(const RCVector2d& min, const RCVector2d& max);

        /**
          @brief
          This sets the minimum bound of the RCBox

          @param min
          The new minimum boundaries of the RCBox2D instance.

          @result
          A reference to itself
          */
        RCBox2D& setMin(const RCVector2d& min);
        RCBox2D& setMax(const RCVector2d& max);

        const RCVector2d& getMin() const;
        const RCVector2d& getMax() const;

        RCVector2d getCenter() const;

        /**
        We can group documentation blocks as follows:
        [Member Group
        Reference](https://www.stack.nl/~dimitri/doxygen/manual/grouping.html#memgroup)

        These functions are setters that updates the bounds of the RCBox2D. You
        will note that in the documentation the functions are grouped, and will
        share the same documentation with each other.
        */
        ///@{
        RCBox2D& updateBounds(const RCVector2d& pt);
        RCBox2D& updateBounds(const RCBox2D& box);
        ///@}

        RCBox2D& expand(const double val);

        RCBox2D& transform(const RCVector2d& offset);
        RCBox2D getTransformed(const RCVector2d& offset) const;

        bool isEmpty() const;
        RC_COMMON_API_INLINE bool contains(const RCVector2d& pt) const
        {
            if (pt.x < minPoint.x || pt.x > maxPoint.x)
                return false;
            if (pt.y < minPoint.y || pt.y > maxPoint.y)
                return false;
            return true;
        }

        bool intersectWith(const RCBox2D& box) const;

        double width() const;
        double height() const;
        double area() const;
    };

    /// \brief Three-dimensional axis aligned bounding box.
    struct RC_COMMON_API RCBox
    {
    private:
        RCVector3d minPoint;
        RCVector3d maxPoint;

    public:
        RCBox();
        RCBox(const RCVector3d& min, const RCVector3d& max);
        RCBox(const RCVector3d& center,
              const double size);    // creates box with size*2 extents in all
                                     // three dimension, centered at center.

        void clear();
        RCBox& setBounds(const RCVector3d& min, const RCVector3d& max);
        RCBox& setMin(const RCVector3d& min);
        const RCVector3d& getMin() const;
        RCBox& setMax(const RCVector3d& max);
        const RCVector3d& getMax() const;
        RCVector3d getCenter() const;

        RCBox& updateBounds(const RCVector3d& pt);
        RCBox& updateBounds(const RCBox& box);
        RCBox& expand(const double val);

        RCBox& translateBy(const RCVector3d& offset);
        RCBox getTranslated(const RCVector3d& offset) const;
        RCBox& transformBy(const RCTransform& tform);
        RCBox getTransformed(const RCTransform& tform) const;

        bool isEmpty() const;

        bool intersectWith(const RCBox& box) const;
        bool intersectWith(const AcGeRay3d& ray) const;
        bool intersectWith(const AcGeRay3d& ray, double& tnear, double& tfar) const;
        bool intersectWith(const AcGePlane& plane, const double threshold) const;

        bool intersectWith(const RCBox& box, RCBox& intersection) const;

        double getDistanceTo(const RCVector3d& point) const;

        bool contains(const RCBox& box) const;
        bool contains(const RCVector3d& pt) const;

        AcGePlane getSide(size_t side) const;
    };

    void RC_COMMON_API getCorners(const RCBox& box, RCVector3d corners[8]);
}}}    // namespace Autodesk::RealityComputing::Foundation
