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
//        AUTODESK PROVIDES THIS PROGRAM 'AS IS' AND WITH ALL FAULTS.
//        AUTODESK SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTY OF
//        MERCHANTABILITY OR FITNESS FOR A PARTICULAR USE.  AUTODESK, INC.
//        DOES NOT WARRANT THAT THE OPERATION OF THE PROGRAM WILL BE
//        UNINTERRUPTED OR ERROR FREE.
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include <foundation/RCCommonDef.h>

#include <type_traits>

namespace Autodesk { namespace RealityComputing { namespace Foundation {

    template <typename T>
    class RCSharedPtr;

    /// \brief An atomically counted base class that RCSharedPtr requires for use.
    /// Any class that wants to be managed by RCSharedPtr *must* inherit from this class, since
    /// it provides the required reference counting interface.
    /// The class itself is not meant to be interacted with by the user
    class RC_COMMON_API RCRefCounted final
    {
    public:
        RCRefCounted(const RCRefCounted&);

        size_t getRefCount() const;

        void incrementRefCount();
        void decrementRefCount();

        static RCRefCounted* create();
        void destroy();

    private:
        RCRefCounted();
        ~RCRefCounted();

        struct Impl;
        Impl* mImpl = nullptr;
    };

    template <typename T>
    class RCSharedPtr
    {
        static_assert(!std::is_pointer<T>::value, "RCSharedPtr cannot manage a pointer");
        static_assert(!std::is_reference<T>::value, "RCSharedPtr cannot manage a reference");
        static_assert(!std::is_array<T>::value, "RCSharedPtr cannot manage an array (yet)");

    public:
        using ElementType = T;

        ///  \brief This is a helper function that functions similary to std::make_shared.
        ///  \param args All the arguments that should be forwarded to the constructor of the class, similarly to stl emplace_* functions
        ///  ```
        ///  RCSharedPtr<Foo> ptr = RCSharedPtr<Foo>::construct();
        ///  ```
        template <typename... Args>
        static RCSharedPtr construct(Args&&... args)
        {
            return RCSharedPtr(new ElementType(args...));
        }

        /// \brief Default constructor for RCSharedPtr.
        RCSharedPtr() noexcept : mPointer(nullptr), mRefCount(nullptr), mDeleter(&privateDeleter)
        {
        }

        /// \brief Constructor overload to allow the user to write RCSharedPtr ptr(nullptr);
        RCSharedPtr(decltype(nullptr)) : RCSharedPtr()
        {
        }

        /// \brief Takes in a pointer whose lifetime will then be managed by the RCSharedPtr.
        /// \param p The pointer to be managed. The type of the pointer must belong to a class that inherits from RCRefCounted
        /// If a pointer that is managed by RCSharedPtr is deleted externally, the behavior of RCSharedPtr becomes undefined.
        /// ```
        /// auto ptr = RCSharedPtr<Foo>(new Foo);
        /// ```
        RCSharedPtr(const ElementType* p) : mRefCount(RCRefCounted::create()), mDeleter(&privateDeleter)
        {
            acquire(p);
        }

        /// \brief Constructor to share ownership.
        template <typename Y>
        RCSharedPtr(const RCSharedPtr<Y>& rhs, ElementType* p) : mRefCount(rhs.mRefCount), mDeleter(rhs.mDeleter)
        {
            acquire(p);
        }

        /// \brief Constructor to share ownership.
        template <typename ChildType>
        RCSharedPtr(const RCSharedPtr<ChildType>& rhs) : mRefCount(rhs.mRefCount), mDeleter(rhs.mDeleter)
        {
            static_assert(std::is_base_of<ElementType, ChildType>::value, "Types do not match");
            if (rhs.mPointer != nullptr && rhs.mRefCount != nullptr && rhs.mRefCount->getRefCount() != 0)
                acquire(rhs.mPointer);
        }

        /// \brief Copy constructor for RCSharedPtr
        RCSharedPtr(const RCSharedPtr& rhs) noexcept : mRefCount(rhs.mRefCount), mDeleter(rhs.mDeleter)
        {
            if (rhs.mPointer != nullptr && rhs.mRefCount != nullptr && rhs.mRefCount->getRefCount() != 0)
                acquire(rhs.mPointer);
        }

        /// \brief The destructor for RCSharedPtr.
        /// If this is the last instance of an RCSharedPtr holding a pointer to a
        /// managed object, the object will be deleted
        inline ~RCSharedPtr() noexcept
        {
            release();
        }

        /// \brief Releases ownership of the current pointer being managed
        /// ```
        /// auto pFoo = RCSharedPtr<Foo>::construct();
        /// pFoo.reset(); // releases ownership of pointer being managed
        /// ```
        inline void reset() noexcept
        {
            release();
        }

        /// \brief Releases ownership of the current pointer being managed
        /// ```
        /// auto pFoo = RCSharedPtr<Foo>::construct();
        /// pFoo.reset(nullptr); // releases ownership of pointer being managed
        /// ```
        void reset(decltype(nullptr))
        {
            release();
        }

        /// \brief Replaces the current pointer being managed with a new instance.
        /// ```
        /// auto pFoo  = RCSharedPtr<Foo>::construct();
        /// auto pFoo2 = RCSharedPtr<Foo>::construct();
        /// pFoo.reset(pFoo2); // releases ownership of pointer being managed and shares ownership of the new pointer with pFoo2
        /// ```
        void reset(const RCSharedPtr& rhs)
        {
            mDeleter = rhs.mDeleter;
            if ((void*)mPointer != (void*)(rhs.mPointer))
            {
                release();
                if (rhs.mRefCount && rhs.mRefCount->getRefCount())
                    mRefCount = rhs.mRefCount;

                acquire(rhs.mPointer);
            }
        }

        /// \brief Replaces the current pointer being managed with a new instance.
        /// ```
        /// auto pFoo = RCSharedPtr<Foo>::construct();
        /// pFoo.reset(new Foo); // releases ownership of pointer being managed and takes ownership of the new pointer
        /// pFoo.reset(nullptr); // releases ownership of pointer being managed
        /// ```
        void reset(ElementType* p)
        {
            if ((void*)mPointer != (void*)p)
            {
                release();
                acquire(p);
            }
        }

        ///\brief Getter function that returns the raw pointer.
        ///```
        /// void FooFunction(Foo*);
        /// auto pFoo = RCSharedPtr<Foo>::construct();
        /// FooFunction(pFoo.get());
        ///```
        ElementType* get() const
        {
            return mPointer;
        }

        /// \brief Predicate to check if RCSharedPtr is currently managing an object
        /// ```
        /// auto pFoo = RCSharedPtr<Foo>::construct();
        /// pFoo.isNull(); // returns false
        /// ```
        bool isNull() const
        {
            return mPointer == nullptr;
        }

        /// \brief Returns the number of references RCSharedPtr is currently tracking
        /// \return 0 if RCSharedPtr is nor currently managing anything,
        /// the number of other RCSharedPtr instances managing the same
        /// instance otherwise
        /// ```
        /// auto pFoo = RCSharedPtr<Foo>::construct();
        /// auto pFoo2 = pFoo;
        /// pFoo.getRefCount(); // returns 2
        /// ```
        size_t getRefCount() const
        {
            return mRefCount ? mRefCount->getRefCount() : 0;
        }

        /// \brief Performs a safe way to static_cast<Y> for RCSharedPtr
        template <class Y>
        RCSharedPtr<Y> staticCast()
        {
            return RCSharedPtr<Y>(*this, static_cast<Y*>(get()));
        }

        /// \brief Performs a safe way to static_cast<Y> for RCSharedPtr
        template <class Y>
        const RCSharedPtr<Y> staticCast() const
        {
            auto non_const_this = const_cast<RCSharedPtr<T>*>(this);
            return non_const_this->staticCast<Y>();
        }

        /// \brief Performs a safe way to dynamic_cast<Y> for RCSharedPtr
        template <class Y>
        RCSharedPtr<Y> dynamicCast()
        {
            auto p = dynamic_cast<Y*>(get());
            if (nullptr != p)
                return RCSharedPtr<Y>(*this, p);
            else
                return RCSharedPtr<Y>();
        }

        /// \brief Performs a safe way to dynamic_cast<Y> for RCSharedPtr
        template <class Y>
        const RCSharedPtr<Y> dynamicCast() const
        {
            auto non_const_this = const_cast<RCSharedPtr<T>*>(this);
            return non_const_this->dynamicCast<Y>();
        }
        /// \brief Copy assignment operator for RCSharedPtr
        /// \param ptr The other RCSharedPtr to share ownership with
        /// If RCSharedPtr was managing a pointer before the assignment, and the
        /// pointer is the last reference to the object, the object would be
        /// deleted
        template <typename ChildType>
        RCSharedPtr& operator=(const RCSharedPtr<ChildType>& ptr)
        {
            static_assert(std::is_base_of<ElementType, ChildType>::value, "Types do not match");
            reset(ptr);
            return *this;
        }

        /// \brief Copy assignment operator for RCSharedPtr
        /// \param ptr The other RCSharedPtr to share ownership with
        /// If RCSharedPtr was managing a pointer before the assignment, and the
        /// pointer is the last reference to the object, the object would be
        /// deleted
        RCSharedPtr& operator=(const RCSharedPtr& ptr)
        {
            reset(ptr);
            return *this;
        }

        /// \brief Replaces the current pointer being managed with a new pointer.
        /// ```
        /// auto pFoo = RCSharedPtr<Foo>::construct();
        /// pFoo.reset(new Foo); // releases ownership of pointer being managed and takes ownership of the new pointer
        /// pFoo.reset(nullptr); // releases ownership of pointer being managed
        /// ```
        RCSharedPtr& operator=(const ElementType* ptr)
        {
            reset(ptr);
            return *this;
        }

        /// \brief Implicit cast operator that allows RCSharedPtr to pretend it's a pointer
        /// ```
        /// void FooFunction(Foo*);
        /// auto pFoo = RCSharedPtr<Foo>::construct();
        /// FooFunction(pFoo);
        /// ```
        operator const ElementType*() const
        {
            return mPointer;
        }

        /// \brief Implicit cast operator that allows RCSharedPtr to pretend it's a pointer
        /// ```
        /// void FooFunction(const Foo*);
        /// auto pFoo = RCSharedPtr<Foo>::construct();
        /// FooFunction(pFoo);
        /// ```
        operator ElementType*()
        {
            return mPointer;
        }

        /// \brief Dereference operator
        /// This allows RCSharedPtr to pretend it's a pointer better
        /// ```
        /// auto pFoo = RCSharedPtr<Foo>::construct();
        /// Foo other = *pFoo;
        /// ```
        ElementType& operator*() const
        {
            return *mPointer;
        }

        /// \brief Member dereference operator
        /// This allows RCSharedPtr to pretend it's a pointer better
        /// ```
        /// struct Foo : RCRefCounted{
        /// void print() const;
        /// };
        /// auto pFoo = RCSharedPtr<Foo>::construct();
        /// pFoo->print();
        /// ```
        ElementType* operator->() const
        {
            return mPointer;
        }

        /// \brief Conversion operator to bool.
        /// ```
        /// auto pFoo = RCSharedPtr<Foo>::construct();
        /// if (pFoo) {} else {}
        /// ```
        inline operator bool() const noexcept
        {
            if (nullptr == mRefCount)
                return false;

            return (0 < mRefCount->getRefCount());
        }

        /// \brief Comparison operator between two RCSharedPtrs.
        /// \return true if both point to the same instance, false otherwise
        template <typename Y>
        bool operator==(const RCSharedPtr<Y>& rhs) const
        {
            static_assert(std::is_base_of<ElementType, Y>::value || std::is_base_of<Y, ElementType>::value, "Types do not match");
            return get() == rhs.get();
        }

        /// \brief Negative comparison operator between two RCSharedPtrs.
        /// \return false if both point to the same instance, true otherwise
        template <typename Y>
        bool operator!=(const RCSharedPtr<Y>& rhs) const
        {
            static_assert(std::is_base_of<ElementType, Y>::value || std::is_base_of<Y, ElementType>::value, "Types do not match");
            return !(*this == rhs);
        }

        /// \brief Comparison operator between two RCSharedPtrs.
        /// \return true if the pointer owned by this instance to is smaller than that owned by the other, false otherwise
        template <typename Y>
        bool operator<(const RCSharedPtr<Y>& rhs) const
        {
            static_assert(std::is_base_of<ElementType, Y>::value || std::is_base_of<Y, ElementType>::value, "Types do not match");
            return this->get() < rhs.get();
        }

        /// \brief Comparison operator between two RCSharedPtrs.
        /// \return true if the pointer owned by this instance to is not smaller than that owned by the other, false otherwise
        template <typename Y>
        bool operator>=(const RCSharedPtr<Y>& rhs) const
        {
            static_assert(std::is_base_of<ElementType, Y>::value || std::is_base_of<Y, ElementType>::value, "Types do not match");
            return !(*this < rhs);
        }

        /// \brief Comparison operator between two RCSharedPtrs.
        /// \return true if the pointer owned by this instance to is greater than that owned by the other, false otherwise
        template <typename Y>
        bool operator>(const RCSharedPtr<Y>& rhs) const
        {
            static_assert(std::is_base_of<ElementType, Y>::value || std::is_base_of<Y, ElementType>::value, "Types do not match");
            return this->get() > rhs.get();
        }

        /// \brief Comparison operator between two RCSharedPtrs.
        /// \return true if the pointer owned by this instance to is not greater than that owned by the other, false otherwise
        template <typename Y>
        bool operator<=(const RCSharedPtr<Y>& rhs) const
        {
            static_assert(std::is_base_of<ElementType, Y>::value || std::is_base_of<Y, ElementType>::value, "Types do not match");
            return !(*this > rhs);
        }

    private:
        inline void acquire(const ElementType* p)
        {
            if (nullptr != p)
            {
                if (nullptr == mRefCount)
                    mRefCount = RCRefCounted::create();

                mRefCount->incrementRefCount();
            }

            mPointer = (ElementType*)p;
        }

        void release() noexcept
        {
            if (nullptr != mRefCount)
            {
                mRefCount->decrementRefCount();
                if (mRefCount->getRefCount() == 0)
                {
                    if (mDeleter)
                        mDeleter((void*)mPointer);

                    mRefCount->destroy();
                }
            }

            mRefCount = nullptr;
            mPointer  = nullptr;
        }

    private:
        template <class Y>
        friend class RCSharedPtr;

        static void privateDeleter(void* ptr)
        {
            delete reinterpret_cast<ElementType*>(ptr);
        }

    private:
        ElementType* mPointer   = nullptr;
        RCRefCounted* mRefCount = nullptr;
        void (*mDeleter)(void*) = nullptr;
    };

}}}    // namespace Autodesk::RealityComputing::Foundation
