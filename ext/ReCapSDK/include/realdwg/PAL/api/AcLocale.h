//  Copyright 2020 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form

#pragma once
#ifndef ACLOCALE_H
#define ACLOCALE_H

#include "PAL/api/def.h"
#include <string>
class AcLocaleImp;

class AcLocale
{
public:
    /// <description>
    ///  Create a locale class with language name and country name. If the input country name and
    /// language name is nullptr or empty, it will get the current system locale.
    /// </description>
    /// <param name="isoLangName">lower-case two-letter codes as defined by ISO-639-1.</param>
    /// <param name="isoCountryName">the upper-case two-letter codes as defined by ISO-3166. It can be nullptr.</param>
    ACPAL_PORT AcLocale(const wchar_t* iso2LangName, const wchar_t* iso2CountryName);
    /// <description>
    ///  Destructor
    /// </description>
    ACPAL_PORT ~AcLocale();
    /// <description>
    ///  Copy constructor
    /// </description>
    ACPAL_PORT AcLocale(const AcLocale& locale);
    /// <description>
    ///  Operator assignment
    /// </description>
    ACPAL_PORT AcLocale& operator =(const AcLocale& locale);
    /// <description>
    ///  Move constructor
    /// </description>
    ACPAL_PORT AcLocale(AcLocale&& locale);
    /// <description>
    ///  Move assignment
    /// </description>
    ACPAL_PORT AcLocale& operator =(AcLocale&& locale);
    /// <description>
    ///  Check if two locale objects are equal
    /// </description>
    ACPAL_PORT bool operator ==(const AcLocale& loc) const;
    /// <description>
    ///  Check if two locale objects are not equal
    /// </description>
    ACPAL_PORT bool operator !=(const AcLocale& loc) const;

    /// <description>
    ///  Get the ISO-639-1 language name
    /// </description>
    /// <returns>
    ///  Return a buffer contains iso language name
    /// </returns>
    ACPAL_PORT const wchar_t*   iso2LangName() const;
    /// <description>
    ///  Get the ISO-3166 country name
    /// </description>
    /// <returns>
    ///  Return a buffer contains iso country name
    /// </returns>
    ACPAL_PORT const wchar_t*   iso2CountryName() const;

    /// <description>
    ///  Check if the locale object is valid. If there is no language name
    ///  or it can't supported by system, it is invalid.
    /// </description>
    /// <returns>
    ///  Return true if it is a valide locale object, otherwise false
    /// </returns>
    ACPAL_PORT bool             isValid() const;

#if defined(AC_NON_CROSS_PLATFORM_API)
    using LocaleId = int;
    /// <description>
    /// allow implicit conversion from Windows Locale LCID
    /// </description>
    /// <param name="lcid">Windows locale ID</param>
    ACPAL_PORT AcLocale(LocaleId lcid); 
    /// <description>
    /// allow implicit conversion to Windows Locale LCID
    /// </description>
    ACPAL_PORT operator LocaleId(); 
#endif

private:
    friend class AcLocaleService;
    AcLocale(const AcLocaleImp* pImpLocale);

    AcLocaleImp* m_pImpAcLocale;
};
#endif