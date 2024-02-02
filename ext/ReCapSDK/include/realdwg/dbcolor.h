//////////////////////////////////////////////////////////////////////////////
//
//  Copyright 2020 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.   
//
//  dbcolor.h
//
//////////////////////////////////////////////////////////////////////////////
//
// DESCRIPTION: True Color Definitions

// headers
#ifndef AD_DBCOLOR_H
#define AD_DBCOLOR_H 1

#include "adesk.h"
#include "acadstrc.h"
#include "rxobject.h"
#include "AcDbCore2dDefs.h"     // ACDBCORE2D_PORT
#include <vector>
#pragma pack (push, 8)


class AcCmEntityColor;

// Define the following symbol to allow use of
// deprecated methods with compiler warnings
// note: ios DwgDumper source is not part of acad vault
// yet the code is part of the build and it is stale
// therefore we must provide depracated methods for the
// iOS fabric build because of DwgDumper at this time
#if defined(_ADESK_IOS_) || defined(LOCAL_SHARDER)
#define AcCmColor_DEPRECATED_METHODS
#endif

// It takes care of the color method, which is one of the following: 
// byBlock, byLayer, or byColor.
// Depending on this color method it stores RGB, ACI, or Layerindex.
//
// Note: To save memory I did the following:
//          It is not deriving from AcDbXObject. 
//          No virtual methods.
//          Color Method stored in last byte mRGBM.
class AcCmEntityColor
{
public:
    enum Color { kRed,
                 kGreen,
                 kBlue
    };

    // Color Method.
    enum ColorMethod {   kByLayer =0xC0, 
                         kByBlock,
                         kByColor,
                         kByACI,
                         kByPen,
                         kForeground,
                         kLayerOff,
                         // Run-time states
                         kLayerFrozen,
                         kNone
    };

    enum ACIcolorMethod {kACIbyBlock    = 0,
                         kACIforeground = 7,
                         kACIbyLayer    = 256,
                         // Run-time states
                         kACIclear      = 0,    
                         kACIstandard   = 7,
                         kACImaximum    = 255,
                         kACInone       = 257,
                         kACIminimum    = -255,
                         kACIfrozenLayer= -32700
    };

    ACDBCORE2D_PORT AcCmEntityColor     ();
    ACDBCORE2D_PORT AcCmEntityColor     (const AcCmEntityColor & color);
    ACDBCORE2D_PORT AcCmEntityColor     (Adesk::UInt8 red,
                                         Adesk::UInt8 green, 
                                         Adesk::UInt8 blue);
    ACDBCORE2D_PORT ~AcCmEntityColor() {};
    ACDBCORE2D_PORT AcCmEntityColor&    operator =  (const AcCmEntityColor& color);
    ACDBCORE2D_PORT bool                operator == (const AcCmEntityColor& color) const;
    ACDBCORE2D_PORT bool                operator != (const AcCmEntityColor& color) const;

    void setNone() { mRGBM.whole = 0; mRGBM.mdata.colorMethod = kNone; }
    void setByBlock() { mRGBM.whole = 0; mRGBM.mdata.colorMethod = kByBlock; }
    void setForeground() { mRGBM.whole = 0; mRGBM.mdata.colorMethod = kForeground; }
    void setByLayer() { mRGBM.whole = 0; mRGBM.mdata.colorMethod = kByLayer; }
    ACDBCORE2D_PORT void setLayerOff();

    static AcCmEntityColor None() { AcCmEntityColor ec; ec.setNone(); return ec; }
    static AcCmEntityColor ByBlock() { AcCmEntityColor ec; ec.setByBlock(); return ec; }
    static AcCmEntityColor ByLayer() { AcCmEntityColor ec; ec.setByLayer(); return ec; }
    static AcCmEntityColor Foreground() { AcCmEntityColor ec; ec.setForeground(); return ec; }
    static AcCmEntityColor white() { return AcCmEntityColor(255, 255, 255); }
    static AcCmEntityColor black() { return AcCmEntityColor(0, 0, 0); }
    // Set/get components
    ACDBCORE2D_PORT ColorMethod         colorMethod     () const;

    ACDBCORE2D_PORT Acad::ErrorStatus   setColorIndex   (Adesk::Int16 colorIndex);
    ACDBCORE2D_PORT Adesk::Int16        colorIndex      () const;

    // For internal use. Index can only use 24 bits..
    ACDBCORE2D_PORT Acad::ErrorStatus   setLayerIndex   (Adesk::Int32 layerIndex);
    ACDBCORE2D_PORT Adesk::Int32        layerIndex      () const;   

    ACDBCORE2D_PORT Acad::ErrorStatus   setPenIndex     (Adesk::UInt16 penIndex);
    ACDBCORE2D_PORT Adesk::UInt16       penIndex        () const;   

    // Set/get RGB components
    ACDBCORE2D_PORT Acad::ErrorStatus   setRGB  (Adesk::UInt8 red,
                                                 Adesk::UInt8 green, 
                                                 Adesk::UInt8 blue);
    ACDBCORE2D_PORT Adesk::UInt8        red     () const;
    ACDBCORE2D_PORT Adesk::UInt8        green   () const;
    ACDBCORE2D_PORT Adesk::UInt8        blue    () const;

    // 32-bit RGB value in Win32 COLORREF format 0x00bbggrr
    ACDBCORE2D_PORT Acad::ErrorStatus   setCOLORREF(Adesk::ColorRef cref);
    ACDBCORE2D_PORT Adesk::ColorRef     getCOLORREF() const;    

    // 32-bit RGB value with Win32 RGBQUAD format 0x00rrggbb 
    ACDBCORE2D_PORT Acad::ErrorStatus   setRGB(Adesk::RGBQuad rgbquad);
    ACDBCORE2D_PORT Adesk::RGBQuad      getRGB() const;

    // 32-bit color value in AutoCAD RGBM format 0xmmrrggbb
    ACDBCORE2D_PORT Acad::ErrorStatus   setRGBM(Adesk::UInt32 rgbmValue);
    ACDBCORE2D_PORT Adesk::UInt32       getRGBM() const;

    // Method check
    ACDBCORE2D_PORT bool                isByColor   () const;
    ACDBCORE2D_PORT bool                isByLayer   () const;
    ACDBCORE2D_PORT bool                isByBlock   () const;
    ACDBCORE2D_PORT bool                isByACI     ()   const;
    ACDBCORE2D_PORT bool                isByPen     ()  const;
    ACDBCORE2D_PORT bool                isForeground() const;
    ACDBCORE2D_PORT bool                isLayerOff  () const;
    // Run time states.
    ACDBCORE2D_PORT bool                isLayerFrozen() const;
    ACDBCORE2D_PORT bool                isNone      ()   const;
    ACDBCORE2D_PORT bool                isLayerFrozenOrOff() const;

    // returs true if the color is resolvable to RGB, false if not (byBlock, etc)
    ACDBCORE2D_PORT bool canResolveRGB() const;
    // return a copy as RGB (kByColor) - cannot resolve indirect colors (byBlock, etc) - will assert
    // foreground is resolved in terms of background color which defaults to black
    ACDBCORE2D_PORT AcCmEntityColor makeRGB(Adesk::ColorRef background_rgb = 0) const;
    // will resolve indirect colors to black - RGB(0,0,0)
    // foreground is resolved in terms of background color which defaults to black
    ACDBCORE2D_PORT AcCmEntityColor forceRGB(Adesk::ColorRef background_rgb = 0) const;
    // return a copy as TrueColor (not ACI - may be indirect, forground, etc.)
    ACDBCORE2D_PORT AcCmEntityColor makeTrueColor() const;

private:
    // Blue, green, red, and Color Method (byBlock, byLayer, byColor).
    // Is stored that way for better performance. 
    // This is an RGBQUAD layout
    // https://docs.microsoft.com/en-us/windows/desktop/api/wingdi/ns-wingdi-tagrgbquad
    //
    // Note that color chanels in RGBQUAD are reversed from COLORREF (0x00bbggrr) 
    // Note the dependency on struct layout: we assume that indirect does not
    // overlap with colorMethod!
    //
    union {
        Adesk::UInt32    whole;
        Adesk::Int16     indirect;
        struct {
            Adesk::UInt8 blue,
                         green,
                         red,
                         colorMethod;
        } mdata;
        Adesk::Int32    mnIndirect32;
    } mRGBM;

    Adesk::Int32 indirect24() const;
    Adesk::UInt8 lookUpACI() const;
    // The Look Up Table (LUT) provides a mapping between ACI and RGB 
    // and contains each ACI's representation in RGB.
    const std::vector<AcCmEntityColor>& lookup_table() const;

#ifdef AcCmColor_DEPRECATED_METHODS
  public:
    [[deprecated("Use default constructor followed by call to setNone(), setByBlock(), setByLayer(), etc")]] ACDBCORE2D_PORT AcCmEntityColor(ColorMethod eColorMethod);
    [[deprecated("Prefer explicit setNone(), setByBlock(), setByLayer(), etc calls to this generic call")]] ACDBCORE2D_PORT Acad::ErrorStatus setColorMethod(ColorMethod eColorMethod);
    [[deprecated("Use setRGBM(), setRGB(), or setCOLORREF()")]] ACDBCORE2D_PORT Acad::ErrorStatus setColor(Adesk::UInt32 color);
    [[deprecated("Use getRGBM(), getRGB() or getCOLORREF()")]] ACDBCORE2D_PORT Adesk::UInt32 color() const;
    [[deprecated("Use setRGB()")]] ACDBCORE2D_PORT Acad::ErrorStatus setRed(Adesk::UInt8 red);
    [[deprecated("Use setRGB()")]] ACDBCORE2D_PORT Acad::ErrorStatus setGreen(Adesk::UInt8 green);
    [[deprecated("Use setRGB()")]] ACDBCORE2D_PORT Acad::ErrorStatus setBlue(Adesk::UInt8 blue);
    // conversion
    [[deprecated("use makeTrueColor()")]] ACDBCORE2D_PORT Adesk::UInt32 trueColor() const;
    [[deprecated("use colorMethod()")]] ACDBCORE2D_PORT Adesk::UInt8 trueColorMethod() const;
    [[deprecated("use makeTrueColor()")]] ACDBCORE2D_PORT Acad::ErrorStatus setTrueColor();
    [[deprecated("use makeTrueColor()")]] ACDBCORE2D_PORT Acad::ErrorStatus setTrueColorMethod();
    [[deprecated("use AcCmEntityColor::makeRGB()")]]
    static ACDBCORE2D_PORT Adesk::UInt32 lookUpRGB(Adesk::UInt8 colorIndex);
    [[deprecated("use AcCmEntityColor::colorIndex()")]]
    static ACDBCORE2D_PORT Adesk::UInt8 lookUpACI(Adesk::UInt8 red,
                                                  Adesk::UInt8 green,
                                                  Adesk::UInt8 blue);
#endif
};

// AcCmEntityColor inline
//

// Default color method is kByColor.
inline 
AcCmEntityColor::AcCmEntityColor()
{
    mRGBM.whole = 0;
    mRGBM.mdata.colorMethod = kByColor;
}

inline
AcCmEntityColor::AcCmEntityColor (const AcCmEntityColor & color)
{
    mRGBM.whole = color.mRGBM.whole;
}

// Default color method is kByColor.
// parameter:   red, green, blue
inline  
AcCmEntityColor::AcCmEntityColor(Adesk::UInt8 red, 
                                 Adesk::UInt8 green, 
                                 Adesk::UInt8 blue)
{
    mRGBM.mdata.red   = red;
    mRGBM.mdata.green = green;
    mRGBM.mdata.blue  = blue;
    mRGBM.mdata.colorMethod = kByColor;
}

inline AcCmEntityColor& 
AcCmEntityColor::operator= (const AcCmEntityColor & color)
{
    mRGBM.whole = color.mRGBM.whole;
    return *this;
}

inline bool
AcCmEntityColor::operator==(const AcCmEntityColor& color) const
{
    return mRGBM.whole == color.mRGBM.whole;
}

inline bool
AcCmEntityColor::operator!=(const AcCmEntityColor& color) const
{
    return mRGBM.whole != color.mRGBM.whole;
}

// get Color Method
inline AcCmEntityColor::ColorMethod     
AcCmEntityColor::colorMethod() const
{
    return (ColorMethod) mRGBM.mdata.colorMethod;
}

// Return signed 32-bit int obtained from low 3 bytes (24 bits) of the rgb struct
//
inline Adesk::Int32 AcCmEntityColor::indirect24() const
{
    // We can only store a 24-bit index, because of the colorMethod field using bits 24..31
    Adesk::Int32 nRet = mRGBM.mnIndirect32;
    // Do sign extension if bit 23 is set
    if ((nRet & 0x800000) != 0)
        nRet |= 0xff000000;     // negative: set bits 24 through 31
    else
        nRet &= ~0xff000000;    // positive: clear bits 24 through 31
    return nRet;
}

// return value:    Layer index.
inline Adesk::Int32 AcCmEntityColor::layerIndex() const
{
    if (mRGBM.mdata.colorMethod != kByLayer && mRGBM.mdata.colorMethod != kLayerOff)
        return -1;      // error

    return indirect24();
}

inline Adesk::UInt16 AcCmEntityColor::penIndex() const
{
    if (mRGBM.mdata.colorMethod != kByPen)
        return 0xffff;  // error
    return (Adesk::UInt16)mRGBM.indirect;
}

inline bool
AcCmEntityColor::isByColor() const
{
    return mRGBM.mdata.colorMethod == kByColor; 
}

inline bool
AcCmEntityColor::isByLayer() const
{
    return (mRGBM.mdata.colorMethod  == kByLayer ||
            (mRGBM.mdata.colorMethod == kByACI   && 
            mRGBM.indirect == kACIbyLayer)) ? true : false;
}

inline bool
AcCmEntityColor::isByBlock() const
{
    return (mRGBM.mdata.colorMethod  == kByBlock ||
            (mRGBM.mdata.colorMethod == kByACI   && 
            mRGBM.indirect == kACIbyBlock)) ? true : false;
}

inline bool
AcCmEntityColor::isByACI()   const
{
    return mRGBM.mdata.colorMethod == kByACI;
}

inline bool
AcCmEntityColor::isByPen()   const
{
    return mRGBM.mdata.colorMethod == kByPen;
}

inline bool
AcCmEntityColor::isForeground()   const
{
    return (mRGBM.mdata.colorMethod  == kForeground ||
            (mRGBM.mdata.colorMethod == kByACI      && 
            mRGBM.indirect == kACIforeground)) ? true : false;
}

inline bool
AcCmEntityColor::isLayerOff() const
{   
    return (mRGBM.mdata.colorMethod  == kLayerOff ||
            (mRGBM.mdata.colorMethod == kByACI && 
            mRGBM.indirect    <  0 && // layer off for ACI is negative
            mRGBM.indirect != kACIfrozenLayer)) ? true : false;
}

inline bool
AcCmEntityColor::isLayerFrozen() const
{   
    return (mRGBM.mdata.colorMethod  == kLayerFrozen ||
            (mRGBM.mdata.colorMethod == kByACI       && 
            mRGBM.indirect == kACIfrozenLayer)) ? true : false;
}

inline bool
AcCmEntityColor::isLayerFrozenOrOff() const
{
    return isLayerFrozen() || isLayerOff();
}

inline bool
AcCmEntityColor::isNone()   const
{
    return (mRGBM.mdata.colorMethod  == kNone ||
            (mRGBM.mdata.colorMethod == kByACI       && 
            mRGBM.indirect == kACInone)) ? true : false;
}

// Base interface class for AcCmColor
class ADESK_NO_VTABLE AcCmColorBase
{
public:

    virtual ~AcCmColorBase() {}

    virtual AcCmEntityColor::ColorMethod  colorMethod() const = 0;

    virtual void                setNone() = 0;
    virtual void                setByBlock() = 0;
    virtual void                setForeground() = 0;
    virtual void                setLayerOff() = 0;
    virtual void                setByLayer() = 0;

    virtual bool                isByColor() const = 0;
    virtual bool                isByLayer() const = 0;
    virtual bool                isByBlock() const = 0;
    virtual bool                isByACI()   const = 0;
    virtual bool                isByPen () const = 0;
    virtual bool                isForeground()   const = 0;

    virtual Acad::ErrorStatus   setRGB  (Adesk::UInt8 red, 
                                         Adesk::UInt8 green, 
                                         Adesk::UInt8 blue) = 0;
    virtual Adesk::UInt8        red  () const = 0;
    virtual Adesk::UInt8        green() const = 0;
    virtual Adesk::UInt8        blue () const = 0;

    // 32-bit RGB value in Win32 COLORREF format 0x00bbggrr
    virtual Acad::ErrorStatus   setCOLORREF(Adesk::ColorRef cref) = 0;
    virtual Adesk::ColorRef     getCOLORREF() const = 0;

    // 32-bit RGB value with Win32 RGBQUAD format 0x00rrggbb 
    virtual Acad::ErrorStatus   setRGB(Adesk::RGBQuad rgbquad) = 0;
    virtual Adesk::RGBQuad      getRGB() const = 0;

    // 32-bit color value in AutoCAD RGBM format 0xmmrrggbb
    virtual Acad::ErrorStatus   setRGBM(Adesk::UInt32 rgbmValue) = 0;
    virtual Adesk::UInt32       getRGBM() const = 0;

    virtual Adesk::UInt16       colorIndex() const = 0;
    virtual Acad::ErrorStatus   setColorIndex(Adesk::UInt16 colorIndex) = 0;
    virtual Adesk::UInt16       penIndex() const = 0;
    virtual Acad::ErrorStatus   setPenIndex (Adesk::UInt16 penIndex) = 0;

    virtual Acad::ErrorStatus   setNames(const ACHAR *colorName,
                                         const ACHAR *bookName = NULL) = 0;
    virtual const ACHAR *       colorName(void) const = 0;
    virtual const ACHAR *       bookName(void) const = 0;
    virtual const ACHAR *       colorNameForDisplay(void) = 0;
    virtual bool                hasColorName(void) const = 0;
    virtual bool                hasBookName(void) const = 0;
#ifdef AcCmColor_DEPRECATED_METHODS
    virtual Acad::ErrorStatus setColorMethod(AcCmEntityColor::ColorMethod eColorMethod) = 0;
    virtual Adesk::UInt32     color() const                                             = 0;
    virtual Acad::ErrorStatus setColor(Adesk::UInt32 color)                             = 0;
    virtual Acad::ErrorStatus setRed(Adesk::UInt8 red)                                  = 0;
    virtual Acad::ErrorStatus setGreen(Adesk::UInt8 green)                              = 0;
    virtual Acad::ErrorStatus setBlue(Adesk::UInt8 blue)                                = 0;
#endif
};


class AcCmTransparency {
public:

    enum transparencyMethod {
                kByLayer = 0,
                kByBlock,
                kByAlpha,


                kErrorValue     // must be last in enum
            };

    // Some Transparency constants
    enum {  kTransparencyByLayer    = (unsigned long)kByLayer,
            kTransparencyByBlock    = (unsigned long)kByBlock,
            kTransparencySolid      = ((unsigned long)(kByAlpha | (0xff << 24))),
            kTransparencyClear      = (unsigned long)kByAlpha
    };

    AcCmTransparency() { mAM.whole = kTransparencyByLayer; } // set all bytes
    AcCmTransparency(Adesk::UInt8 alpha);
    AcCmTransparency(double alphaPercent);
    AcCmTransparency(const AcCmTransparency& other) { mAM.whole = other.mAM.whole; }
    ~AcCmTransparency() {}

    AcCmTransparency& operator=(const AcCmTransparency& other);
    bool operator==(const AcCmTransparency& other) const;
    bool operator!=(const AcCmTransparency& other) const;

    void setAlpha(Adesk::UInt8 alpha);
    void setAlphaPercent(double alphaPercent);

    void setMethod(transparencyMethod method);

    Adesk::UInt8 alpha(void) const;
    double alphaPercent(void) const;

    bool isByAlpha(void) const { return (mAM.mdata.method == kByAlpha); }
    bool isByBlock(void) const { return (mAM.mdata.method == kByBlock); }
    bool isByLayer(void) const { return (mAM.mdata.method == kByLayer); }
    bool isInvalid(void) const { return (mAM.mdata.method == kErrorValue); }
    bool isClear(void) const;
    bool isSolid(void) const;

    Adesk::UInt32   serializeOut(void) const { return mAM.whole; }
    void            serializeIn(Adesk::UInt32);

private: 

    union AM {
        struct {
            Adesk::UInt8    alpha;          // low byte
            Adesk::UInt8    reserved1;    
            Adesk::UInt8    reserved2;
            Adesk::UInt8    method;         // high byte
        } mdata;  // byte data items
        Adesk::UInt32       whole;
    };

    AM mAM;
};

namespace AcCm
{
    enum DialogTabs
    {   
        //these flags can be OR-ed 
        kACITab = 1,
        kTrueColorTab = 2,
        kColorBookTab = 4,
    };

    enum ACIColors
    {   
        kACIByBlock     = 0,
        kACIRed         = 1,
        kACIYellow      = 2,
        kACIGreen       = 3,
        kACICyan        = 4,
        kACIBlue        = 5,
        kACIMagenta     = 6,
        kACIForeground  = 7,
        kACIByLayer     = 256,
    };
}

#pragma pack (pop)

class AcDbObjectId;

ACDBCORE2D_PORT AcCmEntityColor
accmResolveEffectiveColorToRGB(const AcCmEntityColor& effectiveColor, AcDbObjectId entLayerId);

ACDBCORE2D_PORT AcCmEntityColor
accmAttenuateRGB(const AcCmEntityColor& in);

#define ADSK_ACCMENTITYCOLOR_DEFINED
#include "acarrayhelper.h"

#endif // AD_DBCOLOR_H 
