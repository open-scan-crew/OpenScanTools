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

#include <gemat3d.h>

#include <foundation/RCCommonDef.h>

#include <foundation/RCMath.h>
#include <foundation/RCMatrix.h>
#include <foundation/RCVector.h>

namespace Autodesk { namespace RealityComputing { namespace Foundation {
    class RCQuaternion;
    class RCTransform;

    // \brief a 2x2 rotation matrix
    //
    class RC_COMMON_API RCRotationMatrix2d
    {
    public:
        RCRotationMatrix2d();
        RCRotationMatrix2d(const RCRotationMatrix2d& mat);
        RCRotationMatrix2d(const double angle, const RCVector2d& scale = RCVector2d(1, 1));    // create from euler angles
                                                                                               // (degrees) and optional scale

        RCRotationMatrix2d operator*(const RCRotationMatrix2d& m) const;
        RCVector2d operator*(const RCVector2d& v) const;
        RCRotationMatrix2d& operator*=(const RCRotationMatrix2d& m);
        RCRotationMatrix2d& operator=(const RCRotationMatrix2d& m);

        RCRotationMatrix2d& identity();
        RCRotationMatrix2d& transpose();
        RCRotationMatrix2d& orthonormalize();
        RCRotationMatrix2d& inverse();

        void scale(const RCVector2d& t);    // creates a scale only matrix

        RCVector2d getScale() const;

        RCVector2d getColumn(int col) const;

        double getRadians() const;

    private:
        double mValues[4];
    };

    // \brief a 3x3 rotation matrix
    //
    class RC_COMMON_API RCRotationMatrix
    {
        friend class RCTransform;    // RCTransform needs access to array
                                     // elements.

    public:
        enum class OrthographicBias    // Strong axis followed by weak axis
        {
            XY = 0,
            XZ,
            YX,
            YZ,
            ZX,
            ZY
        };
        RCRotationMatrix();
        RCRotationMatrix(const RCRotationMatrix& mat);
        RCRotationMatrix(const RCVector3d& eulerAngles, const RCVector3d& scale = RCVector3d(1, 1, 1));    // create from euler angles
                                                                                                           // (degrees) and optional scale
        RCRotationMatrix(const RCVector3d& axis, double radians,
                         const RCVector3d& scale = RCVector3d(1, 1,
                                                              1));    // create from axis angles
                                                                      // and optional scale

        // Create from strong and weak axis vectors
        RCRotationMatrix(const RCVector3d& strong, const RCVector3d& weak, OrthographicBias bias);

        RCRotationMatrix operator*(const RCRotationMatrix& m) const;
        RCVector3d operator*(const RCVector3d& v) const;
        RCRotationMatrix& operator*=(const RCRotationMatrix& m);
        RCRotationMatrix& operator=(const RCRotationMatrix& m);

        bool operator==(const RCRotationMatrix& rot) const;
        bool operator!=(const RCRotationMatrix& rot) const;

        RCRotationMatrix& identity();
        RCRotationMatrix& transpose();
        RCRotationMatrix& orthonormalize();
        RCRotationMatrix& inverse();

        void scale(const RCVector3d& scale);    // creates a scale only matrix
        void rotX(double radians);              // creates a rotation matrix with pure
                                                // x-rotation
        void rotY(double radians);              // creates a rotation matrix with pure
                                                // y-rotation
        void rotZ(double radians);              // creates a rotation matrix with pure
                                                // z-rotation

        RCVector3d getScale() const;

        RCQuaternion toQuaternion() const;
        RCVector3d toEulerAngle() const;

        // Creates a rotation matrix from normalized x, y, z vectors of the new
        // rotation.
        static RCRotationMatrix createFromVectors(const RCVector3d& x, const RCVector3d& y, const RCVector3d& z);

        RCVector3d getColumn(int col) const;

        // Camera Manipulation
        // This constructor has misleading argument names and is often used to
        // create left handed matrices. Unless this form is required, please use
        // createFromVectors.
        RCRotationMatrix(const RCVector3d& side, const RCVector3d& up, const RCVector3d& view);
        RCVector3d getView() const;
        RCVector3d getSide() const;
        RCVector3d getUp() const;

        // The following functions expect 9 elements in the array (i.e. a 3x3
        // matrix)
        void toRowMajor(double* ptr) const;         // populate input array with row
                                                    // major convention
        void fromRowMajor(const double* ptr);       // set transform from a row major
                                                    // input array
        void toColumnMajor(double* ptr) const;      // populate input array with
                                                    // column major convention
        void fromColumnMajor(const double* ptr);    // set transform from a column
                                                    // major input array

        static void convertToFloatPtr(const RCRotationMatrix& rot,
                                      float* ptr);    // column major convention
                                                      // for use in OpenGL
                                                      // functions

    private:
        double mValues[9];
    };
    RCTransform RC_COMMON_API operator*(const RCRotationMatrix& rotMat, const RCTransform& tform);

    enum class TransformType
    {
        Rigid,                   // Rotation + translation only (scale must be 1.0)
        RigidWithUniformScale    // Rotation + uniform scale + translation only
    };

    // Homogeneous transform (rotation/scale matrix + translation vector)
    // container using row-major convention
    class RC_COMMON_API RCTransform
    {
    public:
        RCTransform();
        RCTransform(const RCRotationMatrix& rot, const RCVector3d& trans);

        RCTransform operator*(const RCTransform& m) const;
        RCTransform operator*(const RCRotationMatrix& r) const;
        RCVector3f operator*(const RCVector3f& v) const;
        RCVector3d operator*(const RCVector3d& v) const;
        RCTransform& operator*=(const RCTransform& m);
        RCTransform& operator*=(const RCRotationMatrix& r);

        RCVector3f rotateOnly(const RCVector3f& v) const;
        RCVector3d rotateOnly(const RCVector3d& v) const;

        void identity();

        // Invert matrix assuming the transform matches given type
        void invert(TransformType type = TransformType::RigidWithUniformScale);
        RCTransform getInverse(TransformType type = TransformType::RigidWithUniformScale) const;

        void orthonormalize();

        void translate(const RCVector3d& t);
        const RCVector3d& getTranslation() const;
        void setTranslation(const RCVector3d& t);
        void zeroTranslation();    // convert to rotation only for transforming
                                   // normals
        const RCRotationMatrix& getRotation() const;
        void setRotation(const RCRotationMatrix& r);

        // The following functions expect 16 elements in the array (i.e. a 4x4
        // matrix)
        void toRowMajor(double* ptr) const;         // populate input array with row
                                                    // major convention
        void fromRowMajor(const double* ptr);       // set transform from a row major
                                                    // input array
        void toColumnMajor(double* ptr) const;      // populate input array with
                                                    // column major convention
        void fromColumnMajor(const double* ptr);    // set transform from a column
                                                    // major input array

        static RCTransform fromAcGeMatrix3d(const AcGeMatrix3d& matrix);
        AcGeMatrix3d toAcGeMatrix3d() const;

    private:
        RCRotationMatrix mRot;
        RCVector3d mTrans;
    };

    // Convenience functions for camera manipulation
    bool RC_COMMON_API equals(const RCTransform& tformA, const RCTransform& tformB,
                              double eps = Constants::EPSILON);    // compare function useful for
                                                                   // camera change detection
    RCTransform RC_COMMON_API fromSideUpView(const RCVector3d& side, const RCVector3d& up, const RCVector3d& view);
    RCVector3d RC_COMMON_API getView(const RCTransform& tform);
    RCVector3d RC_COMMON_API getSide(const RCTransform& tform);
    RCVector3d RC_COMMON_API getUp(const RCTransform& tform);
    RCTransform RC_COMMON_API lookAt(const RCVector3d& pos, const RCVector3d& view, const RCVector3d& up);

    // \brief a full 4x4 projection matrix container using row-major
    // convention
    //
    class RC_COMMON_API RCProjection
    {
    public:
        RCProjection();
        RCProjection(const RCTransform& tform);

        RCProjection operator*(const RCProjection& m) const;
        RCProjection operator*(const RCTransform& m) const;
        RCProjection operator*(const RCRotationMatrix& r) const;
        RCVector3f operator*(const RCVector3f& v) const;
        RCVector3d operator*(const RCVector3d& v) const;
        RCVector4f operator*(const RCVector4f& v) const;
        RCVector4d operator*(const RCVector4d& v) const;
        RCProjection& operator*=(const RCProjection& m);
        RCProjection& operator*=(const RCTransform& m);
        RCProjection& operator*=(const RCRotationMatrix& r);

        double submat(size_t i, size_t j) const;
        double& submat(size_t i, size_t j);

        void identity();
        void invert();
        RCProjection getInverse() const;

        // The following functions expect 16 elements in the array (i.e. a 4x4
        // matrix)
        void toRowMajor(float* ptr) const;         // populate input array with row
                                                   // major convention
        void fromRowMajor(const float* ptr);       // set transform from a row major
                                                   // input array
        void toColumnMajor(float* ptr) const;      // populate input array with
                                                   // column major convention
        void fromColumnMajor(const float* ptr);    // set transform from a column
                                                   // major input array

        void toRowMajor(double* ptr) const;         // populate input array with row
                                                    // major convention
        void fromRowMajor(const double* ptr);       // set transform from a row major
                                                    // input array
        void toColumnMajor(double* ptr) const;      // populate input array with
                                                    // column major convention
        void fromColumnMajor(const double* ptr);    // set transform from a column
                                                    // major input array

        void setOrthographic(double left, double right, double bottom, double top, double nearval, double farval);

        void setPerspective(double angle, double ratio, double fNear, double fFar);

        RCVector3d transformPoint(const RCVector3d& pt) const;

        static void convertToFloatPtr(const RCProjection& tform,
                                      float* ptr);    // column major convention
                                                      // for use in OpenGL
                                                      // functions

    private:
        RCMatrix4x4d mMat;
    };

    bool RC_COMMON_API equals(const RCProjection& tformA, const RCProjection& tformB, double eps = Constants::EPSILON);
}}}    // namespace Autodesk::RealityComputing::Foundation
