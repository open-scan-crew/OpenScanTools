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

#include <cstddef>

//-------------------
// New implementation
//-------------------

namespace Autodesk { namespace RealityComputing { namespace Foundation {
    /// \brief Generic base image class.
    ///
    class RC_COMMON_API RCImage
    {
    public:
        RCImage() : mDimX(0), mDimY(0)
        {
        }
        RCImage(const RCImage& ima) = default;
        RCImage(size_t width, size_t height) : mDimX(width), mDimY(height)
        {
        }
        virtual ~RCImage();
        RCImage& operator=(const RCImage& ima);

        size_t getWidth() const
        {
            return mDimX;
        }    // Retrieve the width of the image
        size_t getHeight() const
        {
            return mDimY;
        }                                                     // Retrieve the height of the image
        virtual int getNumberOfChannels() const = 0;          // Returns the number of
                                                              // channels
        virtual int getNumberOfBitsPerChannel() const = 0;    // Returns the number of
                                                              // bits per channel
        virtual int getLineLength() const = 0;                // Returns the true Line length
                                                              // (in bytes) of the image
        virtual int getType() const = 0;                      // Return the image type
                                                              // (RGB,GRAY,LUT,...)

        enum
        {
            UNKNOWN = 0,
            GRAY    = 1,
            RGB     = 2,
            RGBA    = 4,
            GRAY16  = 8,
            // RGB16   = 16, // NIKO TODO
            // RGBA16  = 32,
            FLOAT = 64
            // RGBA32    = 128,
            // FLOATRGB = 256,
            // FLOATRGBA = 512,
            // TILEGRAY = 1024,
            // TILERGB = 2048
        };

        // types of sub-pixel access
        typedef enum {
            INTER_UNDEF = 0,
            INTER_NEAREST,
            INTER_BILINEAR
            // INTER_BICUBIC, // NIKO TODO
            // INTER_MITCHELL,
            // INTER_CATMULLROM,
            // INTER_BSPLINE,
            // INTER_LANCZOS3,
            // INTER_LANCZOS4,
            // INTER_LANCZOS5
        } Tinterpolation;

    protected:
        size_t mDimX;    // The width of the image
        size_t mDimY;    // The height of the image
    };

    /// \brief Template image class.
    ///
    template <class T>
    class RC_COMMON_API RCImageT : public RCImage
    {
    public:
        using Tpixel = T;

        inline RCImageT();
        RCImageT(size_t width, size_t height, bool fInit = true);
        RCImageT(const RCImageT& ima);
        RCImageT& operator=(const RCImageT& ima);
        RCImageT(RCImageT&& other);
        RCImageT& operator=(RCImageT&& other);
        virtual ~RCImageT() override;

        // Conversion constructors
        explicit RCImageT(const RCImage& ima);

        T* getData()
        {
            return data;
        }
        const T* getData() const
        {
            return data;
        }
        void fill(const T& val);

        // Get the Line length (in bytes) of the image
        //
        // (use this for alignment pb under windows)
        virtual inline int getLineLength() const override;

        virtual inline int getType() const override;
        virtual inline int getNumberOfChannels() const override;
        virtual inline int getNumberOfBitsPerChannel() const override;

        // access to the image line r
        inline const T* operator[](size_t r) const;
        // access to the image line r
        inline T* operator[](size_t r);
        // pixel access
        inline const T operator()(size_t x, size_t y) const;
        // non-const pixel access
        inline T& operator()(size_t x, size_t y);

        // sub-pixel access
        // T operator()(double x, double y, Tinterpolation eInterpMode =
        // CrzImage::INTER_BILINEAR) const; // NIKO TODO

    protected:
        // Image data stored in a linear form
        T* data;    // This pointer is always aligned on a 128-bits bound addr for
                    // a RCImageT<float> (due to SIMD code)
        // INT_PTR   real_data_addr; // This pointer is used to restore the data
        // pointer for RCImageT<float> // NIKO TODO

        // 2D access to the image data
        T** fastdata;
        // the actual len in bytes of a line
        int m_iLineLen;
        // Are we the owner of the data?
        int m_fOwner;
        // Are we the owner of the fastdata ? (yes if we are in the app, no if
        // it is the API and the fastdata is created thru PrepareData())
        bool m_bFastdataOwner;

        void init(size_t width, size_t height, bool fInit = true);
        void destroy();
    };

    // Forward declarations of pixel types for conversion constructors
    class RC_COMMON_API RCColor;
    class RC_COMMON_API RCRGBA;

    /// \brief Template class to define one channel pixel used by RCImageT
    ///
    template <class T>
    class RC_COMMON_API RCGrayT
    {
    public:
        using Tvalue = T;
        static unsigned char getNumberOfChannels()
        {
            return 1;
        }

        // we allow implicit conversion on this class
        RCGrayT(const T& val) : y(val)
        {
        }
        explicit RCGrayT() : y(0)
        {
        }
        operator T() const
        {
            return y;
        }
        inline Tvalue& operator[](size_t /*i*/)
        {
            return y;
        }
        inline const Tvalue& operator[](size_t /*i*/) const
        {
            return y;
        }

        T y;
    };

    using RCGray    = RCGrayT<unsigned char>;
    using RCGray16  = RCGrayT<unsigned short>;
    using RCGrayFlt = RCGrayT<float>;

    /// \brief Color class (RGB 8bits/channel)
    ///
    class RC_COMMON_API RCColor
    {
    public:
        using Tvalue = unsigned char;
        static unsigned char getNumberOfChannels()
        {
            return 3;
        }

        RCColor() : r(0), g(0), b(0)
        {
        }
        RCColor(unsigned char red, unsigned char grn, unsigned char blu) : r(red), g(grn), b(blu)
        {
        }

        explicit inline RCColor(const RCGray& val);
        explicit inline RCColor(const RCRGBA& col);
        explicit inline RCColor(const RCGray16& val);
        explicit inline RCColor(const RCGrayFlt& val);

        inline Tvalue& operator[](size_t i)
        {
            switch (i)
            {
            case 0:
                return r;
            case 1:
                return g;
            case 2:
                return b;
            default:
                return b;
            }
        }
        inline const Tvalue& operator[](size_t i) const
        {
            switch (i)
            {
            case 0:
                return r;
            case 1:
                return g;
            case 2:
                return b;
            default:
                return b;
            }
        }

        unsigned char r, g, b;    // red, green, blue
    };

    // For naming coherence
    using RCRGB = RCColor;

    /// \brief Color class with alpha channel (RGBA - 8bits/channel)
    ///
    class RC_COMMON_API RCRGBA
    {
    public:
        using Tvalue = unsigned char;
        static unsigned char getNumberOfChannels()
        {
            return 4;
        }

        RCRGBA() : r(0), g(0), b(0), a(0)
        {
        }
        RCRGBA(unsigned char red, unsigned char grn, unsigned char blu, unsigned char alpha) : r(red), g(grn), b(blu), a(alpha)
        {
        }

        explicit RCRGBA(const RCGray& val) : r(val.y), g(val.y), b(val.y), a(255)
        {
        }
        explicit RCRGBA(const RCColor& col) : r(col.r), g(col.g), b(col.b), a(255)
        {
        }
        explicit RCRGBA(const RCGray16& val) : r(val.y >> 8), g(val.y >> 8), b(val.y >> 8), a(255)
        {
        }
        explicit RCRGBA(const RCGrayFlt& val)
            : r(static_cast<unsigned char>(val * 255)), g(static_cast<unsigned char>(val * 255)), b(static_cast<unsigned char>(val * 255))
        {
        }

        inline Tvalue& operator[](size_t i)
        {
            switch (i)
            {
            case 0:
                return r;
            case 1:
                return g;
            case 2:
                return b;
            case 3:
                return a;
            default:
                return a;
            }
        }
        inline const Tvalue& operator[](size_t i) const
        {
            switch (i)
            {
            case 0:
                return r;
            case 1:
                return g;
            case 2:
                return b;
            case 3:
                return a;
            default:
                return a;
            }
        }

        unsigned char r, g, b;    // red, green, blue
        unsigned char a;          // alpha channel;
    };

    //
    // Pixel types inline implementation
    //
    inline RCColor::RCColor(const RCGray& val) : r(val.y), g(val.y), b(val.y)
    {
    }
    inline RCColor::RCColor(const RCRGBA& col) : r(col.r), g(col.g), b(col.b)
    {
    }
    inline RCColor::RCColor(const RCGray16& val) : r(val.y >> 8), g(val.y >> 8), b(val.y >> 8)
    {
    }

    //
    // Specializations of some RCImageT methods
    //

    // NIKO TODO: error C2491
    template <class T>
    inline int RCImageT<T>::getType() const
    {
        return RCImage::UNKNOWN;
    }

    template <>
    inline int RCImageT<RCGray>::getType() const
    {
        return RCImage::GRAY;
    }
    template <>
    inline int RCImageT<RCColor>::getType() const
    {
        return RCImage::RGB;
    }
    template <>
    inline int RCImageT<RCRGBA>::getType() const
    {
        return RCImage::RGBA;
    }
    template <>
    inline int RCImageT<RCGray16>::getType() const
    {
        return RCImage::GRAY16;
    }
    template <>
    inline int RCImageT<RCGrayFlt>::getType() const
    {
        return RCImage::FLOAT;
    }

    //
    // RCImageT inline implementation
    //

    // Default constructor
    template <class T>
    inline RCImageT<T>::RCImageT() : RCImage(), data(nullptr), fastdata(nullptr), m_iLineLen(0), m_fOwner(false), m_bFastdataOwner(false)
    {
    }

    template <class T>
    inline int RCImageT<T>::getLineLength() const
    {
        return m_iLineLen;
    }

    template <class T>
    inline int RCImageT<T>::getNumberOfChannels() const
    {
        return Tpixel::getNumberOfChannels();
    }

    template <class T>
    inline int RCImageT<T>::getNumberOfBitsPerChannel() const
    {
        return sizeof(typename Tpixel::Tvalue) * 8;
    }

    // operator[], see description for usage
    // NIKO TODO: assert

    template <class T>
    inline const T* RCImageT<T>::operator[](size_t r) const
    { /*assert(fastdata!=nullptr);*/
        return fastdata[r];
    }

    template <class T>
    inline T* RCImageT<T>::operator[](size_t r)
    { /*assert(fastdata!=nullptr);*/
        return fastdata[r];
    }

    template <class T>
    inline const T RCImageT<T>::operator()(size_t x, size_t y) const
    { /*assert(fastdata!=NULL);*/
        return fastdata[y][x];
    }

    template <class T>
    inline T& RCImageT<T>::operator()(size_t x, size_t y)
    { /*assert(fastdata!=NULL);*/
        return fastdata[y][x];
    }

    //
    typedef class RCImageT<RCGray> RCImageGray;
    typedef class RCImageT<RCColor> RCImageRGB;
    typedef class RCImageT<RCRGBA> RCImageRGBA;
    typedef class RCImageT<RCGray16> RCImageGray16;
    typedef class RCImageT<RCGrayFlt> RCImageGrayFlt;

    class RC_COMMON_API IRCImageFactory
    {
    public:
        virtual ~IRCImageFactory()                                                        = default;
        virtual RCImage* create(int type, size_t width, size_t height, bool fInit = true) = 0;
    };
}}}    // namespace Autodesk::RealityComputing::Foundation
