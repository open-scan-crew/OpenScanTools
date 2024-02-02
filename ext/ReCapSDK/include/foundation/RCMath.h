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

#include <foundation/RCCommonDef.h>
#include <cmath>
#include <cfloat>

#include <foundation/RCVectorFwd.h>

namespace Autodesk { namespace RealityComputing { namespace Foundation {

    namespace Constants {
        // CONSTANTS
        static const double EPSILON            = 1.0e-5;    // Arbitrary small number - use with care.
        static const double MATH_PI            = 3.14159265358979323846264338327950;
        static const double DISTANCE_TOLERANCE = 1.0e-6;         // Distance tolerance : The distance where points which are
                                                                 // within this range from each
                                                                 //      other are considered to be at the same location.
        static const double ANGLE_TOLERANCE = 1.0e-8;            // Angle tolerance : the angle, in radians, between
                                                                 //      two vectors which deviate by DISTANCE_TOLERANCE at
                                                                 //      a range of 100. ANGLE_TOLERANCE =
                                                                 //      asin(DISTANCE_TOLERANCE/100.0)
        static const double ANGLE_COSINE_TOLERANCE = 1.0e-10;    // Angle Cosine tolerance : Value suitable for comparing
                                                                 // the dot product of two
                                                                 //      unit vectors for "sameness" with **similar**
                                                                 //      results as comparing the cos of the of the angles
                                                                 //      and using ANGLE_TOLERANCE. Directly computing the
                                                                 //      cos of ANGLE_TOLERANCE is outside the usable
                                                                 //      precision of the cos function provide by the
                                                                 //      standard library. One minus the cos of
                                                                 //      ANGLE_TOLERANCE, as computed by the dot product of
                                                                 //      vectors constructed to have an angle of
                                                                 //      angleTolerances is (1.1102230246251565e-16) which
                                                                 //      is very close to the ulp of 1.0 making it
                                                                 //      unsuitable. The value of 1.0e-10 was selected
                                                                 //      after empirical validation.
        static const double DETERMINANT_THRESHOLD = 1.0e-10;
        static const double DEGREE_TO_RADIAN      = MATH_PI / 180.0;
        static const double RADIAN_TO_DEGREE      = 180.0 / MATH_PI;
        static const double PI_OVER_180           = DEGREE_TO_RADIAN;

        static const float EPSILON_F                = static_cast<float>(EPSILON);
        static const float MATH_PI_F                = static_cast<float>(MATH_PI);
        static const float DISTANCE_TOLERANCE_F     = static_cast<float>(DISTANCE_TOLERANCE);
        static const float ANGLE_TOLERANCE_F        = static_cast<float>(ANGLE_TOLERANCE);
        static const float ANGLE_COSINE_TOLERANCE_F = static_cast<float>(ANGLE_COSINE_TOLERANCE);
        static const float DETERMINANT_THRESHOLD_F  = static_cast<float>(DETERMINANT_THRESHOLD);
        static const float DEGREE_TO_RADIAN_F       = static_cast<float>(DEGREE_TO_RADIAN);
        static const float RADIAN_TO_DEGREE_F       = static_cast<float>(RADIAN_TO_DEGREE);
        static const float PI_OVER_180_F            = static_cast<float>(PI_OVER_180);
    }    // namespace Constants

    // Normal lookup table/compression (convert a 3-element float vector
    // into a 14bit index and vice versa)
    static const int NumberOfNormalBins = 16225;
    static const int IndexNoNormal      = 16224;
    class RC_COMMON_API NormalUtils
    {
    public:
        // set up normal lookup table
        static int indexForNormal(const RCVector3f& val);
        static const RCVector3f normalForIndex(int val);
        static float* getNormals();

    private:
        static void initialize();
        static float mNormals[NumberOfNormalBins][3];
        static bool mInitialized;
    };

    class RC_COMMON_API RCMath
    {
    public:
        // Floating point arithmetic
        //
        static double clamp(double v, double min, double max);
        static float clamp(float v, float min, float max);
        static double lerp(double from, double to, double fraction);
        static float lerp(float from, float to, float fraction);
        static double min3(double a, double b, double c);
        static float min3(float a, float b, float c);
        static double max3(double a, double b, double c);
        static float max3(float a, float b, float c);
        static double median3(double a, double b, double c);
        static float median3(float a, float b, float c);
        static double random(double min, double max);
        static float random(float min, float max);
        static double round(double v, int places);
        static float round(float v, int places);
        static int round(double v);
        static int round(float v);
        static void sincos(double v, double& sin, double& cos);
        static void sincos(float v, float& sin, float& cos);

        // Integer arithmetic
        //
        static int clamp(int v, int min, int max);
        static int random(int min, int max);
        static bool isPowerOfTwo(int n);
        static int bitCount(int n);

        // Comparisons
        //
        /// \brief equals(0, epsilon) returns false.
        static bool equals(double x, double y, double epsilon = Constants::EPSILON);
        /// \brief equals(0, epsilon) returns false.
        static bool equals(float x, float y, float epsilon = Constants::EPSILON_F);
        static bool equals(unsigned char x, unsigned char y, unsigned char epsilon = 0);
        /// \brief equalRelative(1, 1 + relativeTolerance) returns true.
        /// equalRelative(1000, 1000 + 1000 * relativeTolerance) returns true.
        static bool equalRelative(double x, double y, double relativeTolerance = 1.0e-9, double absoluteTolerance = 1.0e-32);
        static bool equalRelative(float x, float y, float relativeTolerance = 1.0e-9f, float absoluteTolerance = 1.0e-16f);
        /// \brief greaterThan(epsilon, 0) returns true.
        static bool greaterThan(double x, double y, double epsilon = Constants::EPSILON);
        /// \brief greaterThan(epsilon, 0) returns true.
        static bool greaterThan(float x, float y, float epsilon = Constants::EPSILON_F);
        /// \brief lessThan(0, epsilon) returns true.
        static bool lessThan(double x, double y, double epsilon = Constants::EPSILON);
        /// \brief lessThan(0, epsilon) returns true.
        static bool lessThan(float x, float y, float epsilon = Constants::EPSILON_F);
        /// \brief zero(epsilon) returns false.
        static bool zero(double v, double epsilon = Constants::EPSILON);
        /// \brief zero(epsilon) returns false.
        static bool zero(float v, float epsilon = Constants::EPSILON_F);

        // Conversions
        //
        static double toDegrees(double radians);
        static float toDegrees(float radians);
        static double toRadians(double degrees);
        static float toRadians(float degrees);

        // az el/cartesian
        static RCVector2d cartesianToAzEl(const RCVector3d& cart);
        static RCVector2d cartesianToAzElDepricated(const RCVector3d& cart);    // will be refactored DO NOT USE
        static RCVector3d azElToCartesian(const RCVector2d& azEl);

        // polar/cartesian
        static RCVector3d cartesianToPolar(const RCVector3d& cart);

        // Vector
        //
        static double angleBetween(const RCVector2d& x, const RCVector2d& y);
        static float angleBetween(const RCVector2f& x, const RCVector2f& y);
        static double angleBetween(const RCVector3d& x, const RCVector3d& y);
        static float angleBetween(const RCVector3f& x, const RCVector3f& y);
        static double min3(const RCVector3d& v);
        static float min3(const RCVector3f& v);
        static double max3(const RCVector3d& v);
        static float max3(const RCVector3f& v);

        static RCVector3d getNormalizedOrthogonalVector(const RCVector3d& v);
        static RCVector3f getNormalizedOrthogonalVector(const RCVector3f& v);
        static RCVector3d getNormalizedOrthogonalVector(const RCVector3d& v1, const RCVector3d& v2);
        static RCVector3f getNormalizedOrthogonalVector(const RCVector3f& v1, const RCVector3f& v2);

        static void arbitraryNormalizedBasisFromFirstVector(RCVector3d& v1, RCVector3d& v2, RCVector3d& v3);
        static void arbitraryNormalizedBasisFromFirstVector(RCVector3f& v1, RCVector3f& v2, RCVector3f& v3);

        static bool areParallel(const RCVector3d& v1, const RCVector3d& v2, double cosineTolerance = Constants::ANGLE_COSINE_TOLERANCE);
        static bool areParallel(const RCVector3f& v1, const RCVector3f& v2, float cosineTolerance = Constants::ANGLE_COSINE_TOLERANCE_F);
        static bool arePerpendicular(const RCVector3d& v1, const RCVector3d& v2, double cosineTolerance = Constants::ANGLE_COSINE_TOLERANCE);
        static bool arePerpendicular(const RCVector3f& v1, const RCVector3f& v2, float cosineTolerance = Constants::ANGLE_COSINE_TOLERANCE_F);

        static inline bool isBadPoint(float range)
        {
            return std::fabs(range) < Constants::EPSILON_F;
        }

        // Remaining functions from deprecated alMath
        static void generateCircle(const RCVector3f& dir, float radius, RCVector3f* outVertices, int numVertices);
        static void generateCircle(const RCVector3d& dir, double radius, RCVector3d* outVertices, int numVertices);
        static void generateArc(const RCVector2f& center, float radius, float startAngle, float endAngle, RCVector2f* outVertices, int numVertices);
        static RCVector3d rotatePoint(const RCVector3d& pt, const RCVector3d& rotateAxis, const RCVector3d& ptOnAxis, double rotateAngle);
    };

}}}    // namespace Autodesk::RealityComputing::Foundation
