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

// System includes
#include <cstdint>
#include <stddef.h>

// RC library includes
#include <foundation/RCCommonDef.h>

#include "RCUUID.h"
#include "RCVector.h"

namespace Autodesk { namespace RealityComputing { namespace Foundation {
    /// <description>
    /// A generic buffer class, can be used as dynamic array or stack.
    /// </description>
    template <class T>
    class RCBuffer
    {
    public:
        /// <description>
        /// Constructor.
        /// </description>
        RCBuffer() : mpData(nullptr), mSize(0), mCapacity(0)
        {
        }

        /// <description>
        /// Constructor with given size and default value.
        /// </description>
        RCBuffer(unsigned int size, const T& defaultT) : mSize(size), mCapacity(size)
        {
            mpData = new T[size];
            for (unsigned int i = 0; i < size; ++i)
                mpData[i] = defaultT;
        }
        /// <description>
        /// Copy constructor.
        /// </description>
        RCBuffer(const RCBuffer& buff) : mpData(nullptr), mSize(0), mCapacity(0)
        {
            *this = buff;
        }

        /// <description>
        /// Move constructor.
        /// </description>
        RCBuffer(RCBuffer&& buffer) : mpData(buffer.mpData), mSize(buffer.mSize), mCapacity(buffer.mCapacity)
        {
            buffer.mpData    = nullptr;
            buffer.mSize     = 0;
            buffer.mCapacity = 0;
        }

        /// <description>
        /// Constructor copying its data from a raw array pointer
        /// </description>
        /// <param name="dataPtr">The pointer to the raw array to be copied</param>
        /// <param name="size">The size of the array</param>
        RCBuffer(const T* dataPtr, std::uint64_t size) : mSize((unsigned int)size), mCapacity((unsigned int)size)
        {
            mpData                 = new T[size];
            const auto bytesToCopy = size * sizeof(T);
            memcpy_s(mpData, bytesToCopy, dataPtr, bytesToCopy);
        }

        /// <description>
        /// Constructor taking ownership of data from a raw array pointer
        /// </description>
        /// <param name="dataPtr">The pointer to the raw array to take ownership of</param>
        /// <param name="size">The size of the array</param>
        RCBuffer(T*&& dataPtr, std::uint64_t size) : mSize((unsigned int)size), mCapacity((unsigned int)size)
        {
            mpData  = dataPtr;
            dataPtr = nullptr;
        }

        /// <description>
        /// Destructor.
        /// </description>
        ~RCBuffer()
        {
            clear();
        }

        /// <description>
        /// Get a pointer to the data held by the buffer.
        /// </description>
        const T* getData() const
        {
            return mpData;
        }

        /// <description>
        /// Get the number of objects in the buffer
        /// </description>
        unsigned int getSize() const
        {
            return mSize;
        }

        /// <description>
        /// Returns true if the RCBuffer is empty.
        /// </description>
        bool isEmpty() const
        {
            return mSize == 0;
        }

        /// <description>
        /// Get the buffer's allocated capacity
        /// </description>
        unsigned int getCapacity() const
        {
            return mCapacity;
        }

        /// <description>
        /// Get the row pointer for a 2D buffer with the given \p rowIndex
        /// </description>
        /// <param name="rowIndex">The row index of the 2D buffer</param>
        /// <param name="columnSize">The number of columns in the 2D buffer</param>
        /// <returns>The row pointer</returns>
        const T* getRowOf2DBufferAt(std::uint64_t rowIndex, std::uint64_t columnSize) const
        {
            if (columnSize == 0)
                return nullptr;

            const auto rowSize = static_cast<std::uint64_t>(mSize) / columnSize;
            if (rowIndex >= rowSize)
                return nullptr;

            return mpData + rowIndex * columnSize;
        }

        /// <description>
        /// Append a data object to the buffer.
        /// </description>
        void append(const T& data)
        {
            // if reach maximum size
            if (mSize == mCapacity)
            {
                reserve(mCapacity == 0 ? 1 : mCapacity * 2);
            }

            // copy data.
            mpData[mSize++] = data;
        }

        /// <description>
        /// Insert a data object to the buffer at the given \p index
        /// </description>
        /// \note If \p is out of range, behavior is undefined
        void insertAt(unsigned int index, const T& data)
        {
            if (index == mSize)
            {
                append(data);
                return;
            }

            // if reach maximum size
            if (mSize == mCapacity)
            {
                reserve(mCapacity == 0 ? 1 : mCapacity * 2);
            }

            for (auto i = mSize - 1; i >= index; i--)
            {
                mpData[i + 1] = mpData[i];
            }
            mpData[index] = data;
            mSize++;
        }

        /// <description>
        /// Get a const reference to the data object in the buffer at the given \p index
        /// </description>
        /// \note If \p is out of range, behavior is undefined
        const T& getAt(unsigned int index)
        {
            return mpData[index];
        }

        /// <description>
        /// Set a data object to the buffer at the given \p index
        /// </description>
        /// \note If \p is out of range, behavior is undefined
        void setAt(unsigned int index, const T& data)
        {
            mpData[index] = data;
        }

        /// <description>
        /// Remove the object at the given \p index from the buffer. This does not preserve the order
        /// of the list.
        /// </description>
        /// \note If \p is out of range, behavior is undefined
        void removeFastAt(unsigned int index)
        {
            _removeFast(index);
        }

        /// <description>
        /// Remove an object from the buffer, if found. This does not preserve the order
        /// of the list.
        /// </description>
        void removeFast(const T& t)
        {
            auto index = findIndexOf(t);
            if (index < getSize())
                _removeFast(index);
        }

        /// <description>
        /// Remove an object fat the given \p index from the buffer. This does preserve the order of
        /// the list.
        /// </description>
        /// \note If \p is out of range, behavior is undefined
        void removeAt(unsigned int index)
        {
            _remove(index);
        }

        /// <description>
        /// Remove an object from the buffer, if found.
        /// </description>
        void remove(const T& t)
        {
            _remove(findIndexOf(t));
        }

        /// <description>
        /// Remove all objects equal to \p t from the buffer.
        /// </description>
        void removeAll(const T& t)
        {
            while (true)
            {
                const auto index = findIndexOf(t);
                if (index == mSize)
                    break;

                _remove(index);
            }
        }

        /// <description>
        /// Finds the index of the given object, or returns the same as
        /// getSize() if not found.
        /// </description>
        unsigned int findIndexOf(const T& t)
        {
            for (unsigned int i = 0; i != mSize; ++i)
            {
                if (mpData[i] == t)
                    return i;
            }
            return mSize;
        }

        using CompareOp = bool (*)(const T& t1, const T& t2);

        /// <description>
        /// Checks if the buffer contains the given item \p t
        /// </description>
        bool contains(const T& t, CompareOp compareOp = nullptr)
        {
            for (unsigned int i = 0; i != mSize; ++i)
            {
                if (nullptr != compareOp)
                {
                    if (compareOp(mpData[i], t))
                        return true;
                }
                else if (mpData[i] == t)
                    return true;
            }
            return false;
        }

        /// <description>
        /// Get an element of this buffer at a given index.
        /// </description>
        const T& operator[](unsigned int index) const
        {
            return mpData[index];
        }

        /// <description>
        /// Get an element of this buffer at a given index.
        /// </description>
        T& operator[](unsigned int index)
        {
            return mpData[index];
        }

        /// <description>
        /// Append a group of data object to end of the buffer.
        /// </description>
        void append(const T* data, unsigned int size)
        {
            // check if need to resize the buffer.
            if (mSize + size > mCapacity)
                reserve((mSize + size) * 2);

            // copy data.
            for (unsigned int i = 0; i < size; ++i)
                mpData[mSize + i] = data[i];

            mSize += size;
        }

        /// <description>
        /// Get a const reference to the first object in the buffer.
        /// </description>
        const T& getFirst() const
        {
            return mpData[0];
        }

        /// <description>
        /// Get a reference to the first object in the buffer.
        /// </description>
        T& getFirst()
        {
            return mpData[0];
        }

        /// <description>
        /// Get a const reference to the last object in the buffer.
        /// </description>
        const T& getLast() const
        {
            return mpData[mSize - 1];
        }

        /// <description>
        /// Get a reference to the last object in the buffer.
        /// </description>
        T& getLast()
        {
            return mpData[mSize - 1];
        }

        /// <description>
        /// Allocate and reserve memory for the given capacity.
        /// </description>
        void reserve(unsigned int capacity)
        {
            if (capacity > mCapacity)
            {
                auto* pNewData = new T[capacity];
                for (unsigned int i = 0; i < mSize; ++i)
                    pNewData[i] = mpData[i];

                delete[] mpData;
                mpData = pNewData;

                mCapacity = capacity;
            }
        }

        /// <description>
        /// Resize the current buffer to the given size.
        /// </description>
        void resize(unsigned int size)
        {
            if (mSize < size)
            {
                reserve(size);
            }
            mSize = size;
        }

        /// <description>
        /// Resize the current buffer to the given size, with the given default value.
        /// </description>
        void resize(unsigned int size, const T& defaultT)
        {
            unsigned int oldSize = mSize;
            resize(size);
            for (unsigned int i = oldSize; i < mSize; ++i)
            {
                mpData[i] = defaultT;
            }
        }

        /// <description>
        /// Reset the buffer, only change size and doesn't deallocate buffer.
        /// </description>
        void reset()
        {
            mSize = 0;
        }

        /// <description>
        /// clear all data in the buffer.
        /// </description>
        void clear()
        {
            if (mpData != nullptr)
            {
                delete[] mpData;
                mpData = nullptr;
            }

            mCapacity = 0;
            mSize     = 0;
        }

        bool operator==(const RCBuffer& buff) const
        {
            unsigned int size = getSize();
            if (buff.getSize() != size)
                return false;

            for (unsigned int i = 0; i < size; ++i)
                if (!(mpData[i] == buff.mpData[i]))
                    return false;

            return true;
        }

        RCBuffer& operator=(const RCBuffer& buff)
        {
            unsigned int size = buff.getSize();
            resize(size);

            for (unsigned int i = 0; i < size; ++i)
                mpData[i] = buff.mpData[i];

            return *this;
        }

        /// <description>
        /// Move assignment operator.
        /// </description>
        RCBuffer& operator=(RCBuffer&& buffer)
        {
            if (this != &buffer)
            {
                clear();

                mpData    = buffer.mpData;
                mSize     = buffer.mSize;
                mCapacity = buffer.mCapacity;

                buffer.mpData    = nullptr;
                buffer.mSize     = 0;
                buffer.mCapacity = 0;
            }
            return *this;
        }

        void swap(RCBuffer& buff)
        {
            {
                T* temp     = mpData;
                mpData      = buff.mpData;
                buff.mpData = temp;
            }
            {
                unsigned int temp = mSize;
                mSize             = buff.mSize;
                buff.mSize        = temp;
            }
            {
                unsigned int temp = mCapacity;
                mCapacity         = buff.mCapacity;
                buff.mCapacity    = temp;
            }
        }

        /// \brief Returns a pointer to the start of the range of elements that the buffer contains.
        T* begin()
        {
            return mpData;
        }

        /// \brief Returns a pointer to the start of the range of elements that the buffer contains.
        const T* begin() const
        {
            return mpData;
        }

        /// \brief Returns a pointer to one past the last element, such that if begin() == end() the buffer is empty
        T* end()
        {
            return mpData + mSize;
        }

        /// \brief Returns a pointer to one past the last element, such that if begin() == end() the buffer is empty
        const T* end() const
        {
            return mpData + mSize;
        }

    private:
        inline void _removeFast(unsigned int index)
        {
            mpData[index] = mpData[mSize - 1];
            mSize--;
        }

        inline void _remove(unsigned int index)
        {
            mSize--;

            // if the index is not the last element, copy the last part of the
            // buffer down
            if (index < mSize)
            {
                for (unsigned int i = 0; i < mSize - index; ++i)
                    mpData[index + i] = mpData[index + i + 1];
            }
        }

    protected:
        T* mpData;
        unsigned int mSize;
        unsigned int mCapacity;
    };

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4619)
#pragma warning(disable : 4231)
#endif

    RC_COMMON_API_T template class RC_COMMON_API RCBuffer<bool>;
    RC_COMMON_API_T template class RC_COMMON_API RCBuffer<unsigned int>;
    RC_COMMON_API_T template class RC_COMMON_API RCBuffer<float>;

    RC_COMMON_API_T template class RC_COMMON_API RCBuffer<RCVector2f>;
    RC_COMMON_API_T template class RC_COMMON_API RCBuffer<RCVector2d>;
    RC_COMMON_API_T template class RC_COMMON_API RCBuffer<RCVector3f>;
    RC_COMMON_API_T template class RC_COMMON_API RCBuffer<RCVector3d>;
    RC_COMMON_API_T template class RC_COMMON_API RCBuffer<RCVector4f>;
    RC_COMMON_API_T template class RC_COMMON_API RCBuffer<RCVector4d>;

    RC_COMMON_API_T template class RC_COMMON_API RCBuffer<RCUUID>;

#ifdef _MSC_VER
#pragma warning(pop)
#endif

}}}    // namespace Autodesk::RealityComputing::Foundation
