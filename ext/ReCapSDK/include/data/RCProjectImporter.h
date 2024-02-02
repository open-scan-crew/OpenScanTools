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

#include <data/RCProjectIODef.h>

#include <foundation/RCBuffer.h>
#include <foundation/RCCode.h>
#include <foundation/RCString.h>
#include <data/RCPointBuffer.h>
#include <importexport/RCIOStatus.h>
#include <importexport/RCImportOptions.h>
#include <data/RCProjectIODef.h>
#include <data/RCScan.h>

namespace Autodesk { namespace RealityComputing { namespace Data {

    ///
    /// \brief Represents a class to import files or point clouds
    ///
    class RC_PROJECTIO_API RCProjectImporter final
    {
    public:
        /// \brief Constructor
        /// \param outputFilePath The full path of the output RCP file to be generated. All support
        ///                       files for this RCP will also be generated alongside the file.
        RCProjectImporter(const Foundation::RCString& outputFilePath);

        /// \brief Default destructor
        virtual ~RCProjectImporter();

        /// \brief Add a custom path to the search paths where additional custom plug-ins can be found
        /// \param path The custom path to consider, in addition to the predefined search path, when
        ///        custom plug-ins look-up occurs
        static void addCustomPluginPath(const Foundation::RCString& path);

        /// \brief Import the input files synchronously to produce the an output RCP file with the given settings
        /// \param inputFiles The list of input file paths to be imported, this list cannot be empty.
        ///                   The list should also not contain two files with the same name even they
        ///                   reside in different directories.
        /// \note             You can also import .rcc and .rcs files into the project.
        /// \param importSettings Various settings that control the import process.
        /// \param importStatus The import status of each file specified in \p inputFiles argument.
        /// \param progressCallback The optional callback function. This callback function will be called
        ///                 during the import process with progress information.
        /// \return The \b RSImportResultCode
        Foundation::RCCode importFilesSynchronously(const Foundation::RCBuffer<Foundation::RCString>& inputFiles, const RCImportFilesSettings& importSettings,
                                                    Foundation::RCBuffer<RCScanImportStatus>& importStatus,
                                                    RCImportProgressCallbackPtr progressCallback = nullptr);

        /// \brief Import the input files asynchronously to produce the an output RCP file with the given settings
        /// \param inputFiles The list of input file paths to be imported, this list cannot be empty.
        ///                   The list should also not contain two files with the same name even they
        ///                   reside in different directories.
        /// \note             You can also import .rcc and .rcs files into the project.
        /// \param importSettings Various settings that control the import process.
        /// \param progressCallback The optional callback function. This callback function will be called
        ///                 during the import process with progress information.
        /// \param completionCallback The optional callback function. This callback function will be called
        ///                 after the import process is done.
        void importFiles(const Foundation::RCBuffer<Foundation::RCString>& inputFiles, const RCImportFilesSettings& importSettings,
                         RCImportProgressCallbackPtr progressCallback = nullptr, RCImportCompletionCallbackPtr completionCallback = nullptr);

        /// \brief                      Asynchronously import a subset of scans given an existing parent RCP file. This
        ///                             function returns immediately without waiting for the child project creation to
        ///                             be completed. To properly handle the completion, see completionCallback parameter
        ///                             for details.
        /// \param parentRcpPath        The parent RCP (source) file path to import from.
        /// \param scanNames            A list of scan names to include as part of the import process. This list is
        ///                             mandatory and cannot be left empty. Each entry in the list should contain the scan
        ///                             name without the extension (e.g. 'site_f01' instead of 'site_f01.rcs'). If this
        ///                             list is empty, the call fails and completionCallback function is invoked with an
        ///                             error code of RCCode::BadArgument.
        /// \param cloneDataFiles       Set this parameter to true to have the selected subset of data files (e.g. *.rcs,
        ///                             *.rcc) copied into the support folder of the newly created child project. If this
        ///                             parameter is set to false, data files will remain in the support folder of the
        ///                             parent project, and the child.rcp will be created referencing these data files
        ///                             in-place. In this case, support folders for both parent and child projects need to
        ///                             be bundled with the child.rcp for redistribution.
        /// \param cloneCacheFiles      Set this parameter to true to have all temporary cache files copied from support
        ///                             folder of parent project to that of the child project. If the temporary cache files
        ///                             are not copied, they will be generated on-demand under the support folder of the
        ///                             child project.
        /// \param progressCallback     The optional callback function. This callback function will be called during the
        ///                             import process with progress information.
        /// \param completionCallback   The optional callback function. This callback function will be called after the
        ///                             import process is completed. It is invoked for both successful and failed imports.
        ///                             Note that this callback in invoked in the context of the internal import thread,
        ///                             modifications of shared data in the calling thread will have to be protected by
        ///                             a mutex to avoid data race condition.
        void createChildProject(const Foundation::RCString& parentRcpPath, const Foundation::RCBuffer<Foundation::RCString>& scanNames,
                                bool cloneDataFiles = false, bool cloneCacheFiles = false, RCImportProgressCallbackPtr progressCallback = nullptr,
                                RCImportCompletionCallbackPtr completionCallback = nullptr);

        /// \brief Cancel the asynchronous process of importing files
        void cancel();

        /// \brief Block the current thread and wait for the asynchronous process of importing files to finish
        void waitTillFinished();

        /// \brief [DEPRECATED] Import point cloud to generate an output RCP file with its associated RCS files.
        /// \param inputPointBuffer The buffer containing point cloud data to be imported.
        /// \param projectFilePath The full path of the output RCP file to be generated. All support
        ///                       files for this RCP will also be generated alongside the file.
        /// \param importSettings Various settings that control the import process.
        /// \param progressCallback The optional callback function. This callback function will be called
        ///                 during the import process with progress information.
        /// \return The \b RSImportResultCode
        /// \note This method is deprecated, and is replaced by RCProjectImportSession, which supports asynchronous
        /// out-of-core import of multiple point buffers into multiple scans.
        [[deprecated("Replaced by RCProjectImportSession, which supports asynchronous out-of-core import of multiple point buffers into multiple "
                     "scans.")]] static Foundation::RCCode
            createProjectFromPoints(const RCPointBuffer& inputPointBuffer, const Foundation::RCString& projectFilePath,
                                    const RCImportPointCloudSettings& importSettings, RCImportProgressCallbackPtr progressCallback = nullptr);

        /// \brief [DEPRECATED] Import point cloud to generate an output RCS file
        /// \param inputPointBuffer The buffer containing point cloud data to be imported.
        /// \param outputDirectory The directory of the output RCS file to be generated.
        /// \param importSettings Various settings that control the import process.
        /// \param progressCallback The optional callback function. This callback function will be called
        ///                 during the import process with progress information.
        /// \return The \b RSImportResultCode
        /// \note This method is deprecated, and is replaced by RCScanImportSession, which supports asynchronous
        /// out-of-core import of multiple point buffers.
        [[deprecated("Replaced by RCScanImportSession, which supports asynchronous out-of-core import of multiple point buffers.")]] static Foundation::RCCode
            createScanFromPoints(const RCPointBuffer& inputPointBuffer, const Foundation::RCString& outputDirectory,
                                 const RCImportPointCloudSettings& importSettings, RCImportProgressCallbackPtr progressCallback = nullptr);

        /// \brief Create a project (.rcp) file from the given \p rcsFiles
        /// \note  This API simply assembles the .rcs files into one project,
        ///        .rcs files won't be copied into the support folder of the project,
        ///        and .rcc file won't be associated with .rcs file in this project.
        ///        If you want to import .rcs file together with .rcc files into the project,
        ///        you can use importFiles() API.
        /// \param rcsFiles The input scan (.rcs) files
        /// \param projectFilePath The full path of the project to be created
        /// \return The \b RCCode representing the project creation result.
        static Foundation::RCCode createProjectFromScans(const Foundation::RCBuffer<Foundation::RCString>& rcsFilePaths,
                                                         const Foundation::RCString& projectFilePath);

    private:
        class Impl;
        Impl* mImpl;
    };
}}}    // namespace Autodesk::RealityComputing::Data
