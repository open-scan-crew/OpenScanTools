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

#include <foundation/RCString.h>
#include <foundation/RCFileSystemUtilityDef.h>

namespace Autodesk { namespace RealityComputing { namespace Foundation {

	class IRCFileProtocol;

    using Autodesk::RealityComputing::Foundation::RCString;

    /// \brief The import file open mode
    enum class RCFileMode
    {
        /// \brief Indicating a new file will be created with an existing file with the same name (if any) being discarded.
        New,

        /// \brief Indicating an existing file to be opened for append, or a new file to be created if no existing file.
        Append,
    };

    class RC_FS_UTILITY_API RCFileSystem
    {
    public:
        /// \brief The standard folder paths
        enum class SystemLocation
        {
            /// \brief The desktop location i.e. C:\Users<USER>\Desktop
            Desktop,

            /// \brief The documents location i.e. C:\Users\<USER>\Documents
            Documents,

            /// \brief The Program files location i.e. C:\Program Files
            ProgramFiles,

            /// \brief The program data location i.e. C:\ProgramData
            ProgramData,

            /// \brief The roaming app data location i.e. C:\Users\<USER>\AppData\Roaming
            AppData,

            /// \brief The local app data location i.e. C:\Users\<USER>\AppData\Local
            LocalAppData,

            /// \brief The downloads location i.e. C:\Users\<USER>\Downloads
            Downloads,
        };

        struct Base1024
        {
            double coefficient;
            int exponent;
        };

        static bool getSystemPath(SystemLocation location, RCString& path, bool createFolderIfNotExist = false);

        static IRCFileProtocol* externalFileProtocol();
        static void externalFileProtocol(IRCFileProtocol* file_protocol);

        ///
        /// Path utilties
        ///

        /// \brief Returns the parent path of the specified path.
        static const RCString parentPath(const RCString& path);

        /// \brief Combines two paths with the required platform specific
        /// directory-separator.
        static const RCString combine(const RCString& path1, const RCString& path2);

        /// \brief Returns the filename (basename + extension) at the specified
        /// path.
        static const RCString filename(const RCString& path);

        /// \brief Returns the basename (filename without extension) at the
        /// specified path.
        static const RCString basename(const RCString& path);

        /// \brief Returns the lower case basename (filename without extension) given the specified path.
        static const RCString lowercaseBasename(const RCString& path);

        /// \brief Returns the filename's extension at the specified path.
        static const RCString extension(const RCString& path);

        /// \brief Add the extension to a given filename.
        static RCString addExtension(const RCString& fileName, const RCString& extension);

        /// \brief Returns the filename's extension in lowercase at the
        /// specified path.
        static const RCString lowercaseExtension(const RCString& path);

        /// \brief Replaces the filename's extension at the specified path.
        static const RCString replaceExtension(const RCString& path, const RCString& extension);

        /// \brief Checks if the specified filename has the specified extension;
        /// the comparison is case-insensitive.
        static bool hasExtension(const RCString& path, const RCString& extension);

        /// \brief Checks if two paths reference the same file or directory
        /// instance; the file or directory at the specified path must exist.
        static bool equivalent(const RCString& path1, const RCString& path2);

        ///
        /// File and directory attributes
        ///

        /// \brief Checks if a file or directory exists.
        static bool exists(const RCString& path);

        /// \brief Checks if the file or directory is empty.
        static bool isEmpty(const RCString& path);

        /// \brief Gets the last write time of the given file/directory.
        /// \param[in] path File/directory to check
        /// return Last write time in UTC if successful. -1 if failed.
        static std::int64_t lastWriteTime(const RCString& path);

        /// \brief Modify the last write time of the given file/directory
        /// \param[in] path File/directory to modify
        /// \param[in] time Time in UTC to set
        /// return true if successful. false otherwise.
        static bool modifyLastWriteTime(const RCString& path, std::uint64_t time);

        /// \brief Checks if the specified path is a regular file.
        static bool isFile(const RCString& path);

        /// \brief Checks if the specified path is a directory.
        static bool isDirectory(const RCString& path);

        /// \brief Returns the size of a file; returns -1 if the specified path
        /// is not for a regular file (e.g. a directory). The return value is
        /// undefined for file sizes greater than or equal to 4 exabytes.
        static long long fileSize(const RCString& path);

        /// \brief Returns the size of a list of files; will not include the
        /// sizes of directories or files it can not locate. The return value is
        /// undefined for file sizes greater than or equal to 4 exabytes.
        static long long fileSize(const RCBuffer<RCString>& files);

        /// \brief checks if the file is read only
        static bool isReadOnly(const RCString& path);

        /// \brief checks if a file can be created in the directory
        static bool canWriteDirectory(const RCString& folder);
        ///
        /// Filesystem operations
        ///

        /// \brief Lists files and directories in the specified path; the listed
        /// paths include the specified parent path.
        static const RCBuffer<RCString> list(const RCString& path, bool recursive = false);

        /// \brief Lists only the names of files and directories in the
        /// specified path; the listed paths include the specified parent path.
        static const RCBuffer<RCString> listFilenames(const RCString& path, bool recursive = false);

        /// \brief Lists only files contained by the specified path; the listed
        /// paths include the specified parent path.
        static const RCBuffer<RCString> listFiles(const RCString& path, bool recursive = false);

        /// \brief Lists only directories contained by the specified path; the
        /// listed paths include the specified parent path.
        static const RCBuffer<RCString> listDirectories(const RCString& path, bool recursive = false);

        /// \brief Creates the specified directory; returns false if the
        /// directory cannot be be created or if it already exists. The create
        /// intermediates parameter must be set set to true if the intermediate
        /// directories do not exist.
        static bool createDirectory(const RCString& path, bool createIntermediates = false);

        /// \brief Copies a file or directory to the specified file path; fails
        /// if the target file or directory already exists.
        static bool copy(const RCString& from, const RCString& to);

        /// \brief Copies a file to the specified file path.
        static bool copyFile(const RCString& from, const RCString& to, bool overwrite = false);

        /// \brief Copies a folder to the specified folder path recursively.
        static bool copyFolderRecursively(const RCString& from, const RCString& to, bool overwriteFiles = false);

        /// \brief Removes the file or directory at the specified path; fails if
        /// the directory is not empty.
        static bool remove(const RCString& path);

        /// \brief Removes the specified file or directory; if a directory is
        /// specified, its contents will be removed as well.
        static bool removeAll(const RCString& path);

        /// \brief Renames the file or directory at the specified path.
        static bool rename(const RCString& from, const RCString& to);

        /// \brief set the write permissions a specific file or directory (read
        /// only or write access)
        static bool setWritePermissions(const RCString& path, bool write = true);

        ///
        /// Filesystem state
        ///

        /// \brief Returns a directory path suitable for temporary files.
        static const RCString tempPath();

        /// \brief Returns the application's current working directory.
        static const RCString currentPath();

        /// \brief Sets the application's current working directory; the
        /// specified path must be an absolute path.
        static bool currentPath(const RCString& path);

        /// \brief Gets the path to the application's executable file (.exe in
        /// Windows); the specified path must be an absolute path.
        static RCString executablePath();

        /// \brief Gets the path to the application's executable (.exe in
        /// Windows) folder path; the specified path must be an absolute path.
        static RCString executableFolderPath();

        ///
        /// Miscellaneous
        ///
        /// \brief: Create a unique subdirectory of the specified parent.
        /// Returns new path in subdirOut.
        static bool createUniqueSubdir(const RCString& parentDir, RCString& subdirOut);

        /// \brief Creates a unique temporary directory in the specified
        /// sub-folder under the system's temporary directory.
        static bool createTempDirectory(RCString& folderName, const RCString subFolder = L"");

        /// \brief Checks if the specified file or directory exists; searches
        /// for the first existing file or directory in the specified directory
        /// paths otherwise.
        static bool findFirstExisting(RCString& result, const RCString& path, const RCBuffer<RCString>& searchPaths);

        // \brief Returns the given filename if it is unique (doesn't yet exist
        // on disk); otherwise, returns a filename with the same parent
        // directory and extension and a similar-but-unique basename.
        static RCString getUniqueFileRename(const RCString& path);

        /// \brief Returns the canonical path (without redundant "." or ".."
        /// elements); the specified path must exist. IMPORTANT: This function
        /// supports paths with a Windows long/extended-length paths prefix
        /// (\\?\), but only if the path length is within the short path length
        /// limit of 259 characters, i.e. actual long file paths that exceed 259
        /// characters are NOT supported; see additional notes in the
        /// implementation for more details.
        static const RCString canonical(const RCString& path, const RCString& base = currentPath());

        /// \brief Creates a relative path from one file or directory to
        /// another. The paths do not have to be fully qualified, but they must
        /// have a common prefix and must be in canonical form; returns an empty
        /// string on failure. IMPORTANT: This function supports paths with a
        /// Windows long/extended-length paths prefix (\\?\), but only if the
        /// path length is within the short path length limit of 259 characters,
        /// i.e. actual long file paths that exceed 259 characters are NOT
        /// supported; see additional notes in the implementation for more
        /// details.
        static const RCString makeRelative(const RCString& from, const RCString& to, bool fromIsFile = true);

        /// \brief get the total number of bytes of all the files inside the
        /// root folder
        static void getFolderSize(const RCString& rootFolder, unsigned long long& folderSize);

        /// \brief get the total number of bytes of all the files inside the
        /// list of folders
        static void getFolderSizes(const RCBuffer<RCString>& folders, unsigned long long& folderSizes);

        /// \brief Convert a Windows-style path to whatever format the current
        /// OS expects
        static RCString nativePathFromWindowsPath(const RCString& path);

        /// \brief Convert a Windows-style path to whatever format the current
        /// OS expects
        static RCString pathSeparator();

        /// \brief Returns (coefficient, exponent) tuple where n = coefficient *
        /// (1024 ^ exponent)
        static Base1024 scientificNotationBase1024(const uint64_t& n);

        /// \brief Convert size in bytes to human readable sizes (ex: in B, KB,
        /// MB, GB, etc) as string
        static RCString getHumanReadableSize(const uint64_t& sizeInBytes);

        /// \brief Set path of folder that is used to store the temporary files generated from ReCap
        ///        The temporary folder is required to be writable
        /// return true if set successfully, otherwise return false
        static bool setTempPath(const RCString& temporaryFolder);

        /// \brief Reset to use default temporary folder for ReCap
        static void resetTempPath();
    };
}}}    // namespace Autodesk::RealityComputing::Foundation
