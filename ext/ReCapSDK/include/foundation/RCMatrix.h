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

#include <foundation/RCVectorFwd.h>

namespace Autodesk { namespace RealityComputing { namespace Foundation {
    ///
    /// \brief Row-major order four-by-four matrix template
    ///
    template <typename T>
    struct RC_COMMON_API RCMatrix4x4T
    {
        /// \brief The matrix values
        T values[16];

        /// \brief Default constructor
        RCMatrix4x4T();

        /// \brief Constructor with input elements from m11 to m44
        RCMatrix4x4T(double m11, double m12, double m13, double m14, double m21, double m22, double m23, double m24, double m31, double m32, double m33,
                     double m34, double m41, double m42, double m43, double m44);

        /// \brief Constructor with an input double array
        RCMatrix4x4T(const double* vals);

        /// \brief Constructor with an input float array
        RCMatrix4x4T(const float* vals);

        /// \brief Returns the matrix array
        operator T*();

        /// \brief Returns the const matrix array
        operator const T*() const;

        /// \brief Add the given matrix /p m to this matrix
        /// \param m The input matrix to be added
        /// \return The result of the addition of the matrices
        RCMatrix4x4T operator+(const RCMatrix4x4T& m) const;

        /// \brief Subtract the given matrix /p m from this matrix
        /// \param m The input matrix to be subtracted
        /// \return The result of the subtraction of the matrices
        RCMatrix4x4T operator-(const RCMatrix4x4T<T>& m) const;

        /// \brief Multiply this matrix by the given matrix /p m
        /// \param m The input matrix to be multiplied
        /// \return The result of the multiplication of the matrices
        /// \note This is an overloaded function.
        RCMatrix4x4T operator*(const RCMatrix4x4T& m) const;

        /// \brief Multiply each element in this matrix by the given scalar /p s
        /// \param s The input scalar to be multiplied
        /// \return The result of the matrix multiplication
        /// \note This is an overloaded function.
        RCMatrix4x4T operator*(const T s) const;

        /// \brief Multiply this matrix by the given vector /p v
        /// \param v The input vector to be multiplied
        /// \return The result of the matrix multiplication
        /// \note This is an overloaded function.
        RCVector4T<T> operator*(const RCVector4T<T>& v) const;

        /// \brief Add the given matrix /p m to this matrix
        /// \param m The input matrix to be added
        /// \return The result of the addition of the matrices
        RCMatrix4x4T& operator+=(const RCMatrix4x4T& m);

        /// \brief Subtract the given matrix /p m from this matrix
        /// \param m The input matrix to be subtracted
        /// \return The result of the subtraction of the matrices
        RCMatrix4x4T& operator-=(const RCMatrix4x4T& m);

        /// \brief Multiply this matrix by the given matrix /p m
        /// \param m The input matrix to be multiplied
        /// \return The result of the multiplication of the matrices
        /// \note This is an overloaded function.
        RCMatrix4x4T& operator*=(const RCMatrix4x4T& m);

        /// \brief Multiply each element in this matrix by the given scalar /p s
        /// \param s The input scalar to be multiplied
        /// \return The result of the matrix multiplication
        /// \note This is an overloaded function.
        RCMatrix4x4T& operator*=(const T s);

        /// \brief Multiply this matrix by the given matrix /p m in the
        /// row-major order \param m The input matrix to be multiplied \return
        /// The result of the multiplication of the matrices
        RCMatrix4x4T multiplyRowMajor(const RCMatrix4x4T& m) const;

        /// \brief Multiply this matrix by the given matrix /p m in the
        /// column-major order \param m The input matrix to be multiplied
        /// \return The result of the multiplication of the matrices
        RCMatrix4x4T multiplyColumnMajor(const RCMatrix4x4T& m) const;

        /// \brief Update this matrix by multiplying by the given matrix /p m in
        /// the row-major order \param m The input matrix to be multiplied
        void updateRowMajor(const RCMatrix4x4T& m);

        /// \brief Update this matrix by multiplying by the given matrix /p m in
        /// the column-major order \param m The input matrix to be multiplied
        void updateColumnMajor(const RCMatrix4x4T& m);

        /// \brief Multiply this matrix by the given vector /p v in the
        /// row-major order \param v The input vector to be multiplied \return
        /// The result of the multiplication \note This is an overloaded
        /// function.
        RCVector3T<T> multiplyRowMajor(const RCVector3T<T>& v) const;

        /// \brief Multiply this matrix by the given vector /p v in the
        /// row-major order \param v The input vector to be multiplied \return
        /// The result of the multiplication \note This is an overloaded
        /// function.
        RCVector4T<T> multiplyRowMajor(const RCVector4T<T>& v) const;

        /// \brief Check if each element of this matrix is equal to that of the
        /// given array /p values \param values The value array to compare with
        /// \return True if equal. False otherwise.
        bool operator==(const T* values) const;

        /// \brief Check if each element of this matrix is not equal to that of
        /// the given array /p values \param values The value array to compare
        /// with \return True if /p vals is null or they are not equal. False
        /// otherwise.
        bool operator!=(const T* values) const;

        /// \brief Check if this matrix is equal to the given /p matrix
        /// \param matrix The matrix to compare with
        /// \return True if equal. False otherwise.
        bool operator==(const RCMatrix4x4T& matrix) const;

        /// \brief Check if this matrix is not equal to the given /p matrix
        /// \param matrix The matrix to compare with
        /// \return True if not equal. False otherwise.
        bool operator!=(const RCMatrix4x4T& matrix) const;

        /// \brief Update this matrix as the identity matrix
        void identity();

        /// \brief Transpose this matrix
        void transpose();

        /// \brief Inverse this matrix
        RCMatrix4x4T& invert();

        /// \brief Inverse this matrix by row-major order
        void invertRowMajor();

        /// \brief Inverse this matrix by column-major order
        void invertColumnMajor();
    };

    //
    typedef RCMatrix4x4T<float> RCMatrix4x4f;
    typedef RCMatrix4x4T<double> RCMatrix4x4d;
}}}    // namespace Autodesk::RealityComputing::Foundation
