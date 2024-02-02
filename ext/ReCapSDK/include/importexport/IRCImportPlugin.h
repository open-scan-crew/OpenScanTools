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

#include <foundation/RCBuffer.h>
#include <foundation/RCString.h>
#include "RCImportPluginDef.h"

#define CCK_VERSION_MAJOR 2
#define CCK_VERSION_MINOR 0
#define CCK_VERSION_PATCH 0

namespace Autodesk { namespace RealityComputing { namespace ImportExport {
    using Autodesk::RealityComputing::Foundation::RCBuffer;
    using Autodesk::RealityComputing::Foundation::RCString;
    class IRCImportPluginFileParser;

    ///\brief Interface for import plugin.
    class IRCImportPlugin
    {
    public:
        ///\brief Returns the supported file extensions information
        virtual RCBuffer<ImportFileInfo> getSupportFileInfo() = 0;

        ///\brief Get the file parser using the file extension
        virtual IRCImportPluginFileParser* getFileParser(const RCString& fileExtension) = 0;

        ///\brief Returns true if the input file could be imported by this plugin
        ///\param fileName Input file name
        ///\return True if this input file can be imported. Otherwise False
        virtual bool canImport(const RCString& fileName) = 0;

        ///\brief Fill the provided fields with the major/minor/patch version of the CCK you're compiling against.
        virtual void getCompatibleVersion(unsigned int& major, unsigned int& minor, unsigned int& patch) = 0;
    };

}}}    // namespace Autodesk::RealityComputing::ImportExport

extern "C" {
// Must be implemented to create the plugin
#ifdef _MSC_VER
__declspec(dllexport)
#endif
    Autodesk::RealityComputing::ImportExport::IRCImportPlugin* createPlugin();

// Must be implemented to destory the plugin
#ifdef _MSC_VER
__declspec(dllexport)
#endif
    bool destroyPlugin(Autodesk::RealityComputing::ImportExport::IRCImportPlugin* codec);
}
