//  Copyright 2020 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form

#ifndef _PAL_FONT_PITCH_AND_FAMILY_H
#define _PAL_FONT_PITCH_AND_FAMILY_H

#ifdef PAL
#undef PAL
#endif

namespace Autodesk
{
    namespace AutoCAD
    {
        namespace PAL
        {
            namespace FontUtils
            {
                enum class FontPitch
                {
                    kDefault,
                    kFixed,
                    kVariable
                };
                enum class FontFamily
                {
                    kDoNotCare = 0,
                    kRoman = 16,
                    kSwiss = 32,
                    kModern = 48,
                    kScript = 64,
                    kDecorative = 80
                };
            }
        }
    }
};

#endif