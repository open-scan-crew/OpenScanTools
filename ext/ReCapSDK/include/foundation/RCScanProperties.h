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
#include <iostream>

#include <foundation/RCCommonDef.h>
#include <foundation/RCString.h>

#define PTS_FILE_EXTENSION /*NOXLATE*/ L".pts"
#define XYZ_FILE_EXTENSION /*NOXLATE*/ L".xyz"
#define TXT_FILE_EXTENSION /*NOXLATE*/ L".txt"
#define PTG_FILE_EXTENSION /*NOXLATE*/ L".ptg"
#define ZFS_FILE_EXTENSION /*NOXLATE*/ L".zfs"
#define FLS_FILE_EXTENSION /*NOXLATE*/ L".fls"
#define LAS_FILE_EXTENSION /*NOXLATE*/ L".las"
#define LAZ_FILE_EXTENSION /*NOXLATE*/ L".laz"
#define E57_FILE_EXTENSION /*NOXLATE*/ L".e57"
#define RDBX_FILE_EXTENSION /*NOXLATE*/ L".rdbx"    // riegl: individual scan
#define CL3_FILE_EXTENSION /*NOXLATE*/ L".cl3"      // topcon: structured
#define CLR_FILE_EXTENSION /*NOXLATE*/ L".clr"      // topcon: raw
#define IJ_FILE_EXTENSION /*NOXLATE*/ L".ij"        // topcon: row/col grid info
#define ALG_FILE_EXTENSION /*NOXLATE*/ L".alg"      // topcon: transform
#define PTX_FILE_EXTENSION /*NOXLATE*/ L".ptx"

#define FWS_FILE_EXTENSION /*NOXLATE*/ L".fws"
#define LSPROJ_FILE_EXTENSION /*NOXLATE*/ L".lsproj"
#define ZFPRJ_FILE_EXTENSION /*NOXLATE*/ L".zfprj"
#define RSP_FILE_EXTENSION /*NOXLATE*/ L".rsp"    // riegl: project file
#define PRJ_FILE_EXTENSION /*NOXLATE*/ L".prj"

namespace Autodesk { namespace RealityComputing { namespace Foundation {

    enum class RCScanProvider : std::uint32_t
    {
        // below are unstructured data sets
        Unknown = 0,
        LASData,
        Ascii,
        Topcon,    // NOTE: CLR and CL3 with IJ files are structured
                   // scans
        Mantis,
        Unified,
        ExternalPlugin,

        // below are structured data sets
        ZFS = 128,    //

        OPTech,
        Faro,    //

        LeicaPTG,
        LeicaPTX,
        Leica,

        E57,
        Riegl,
        ReCap,

        // ReCap mobile HW providers
        ReCapMobileBlk360 = 256
    };

    class RC_COMMON_API RCProviderUtils
    {
    public:
        static RCString getProviderName(RCScanProvider providerId);
        static RCString getProviderName(const RCString& extension);
        static RCScanProvider getProviderId(const RCString& extension);
    };

}}}    // namespace Autodesk::RealityComputing::Foundation

RC_COMMON_API_INLINE
std::ostream& operator<<(std::ostream& os, Autodesk::RealityComputing::Foundation::RCScanProvider prov)
{
    return os << static_cast<std::underlying_type<Autodesk::RealityComputing::Foundation::RCScanProvider>::type>(prov);
}
