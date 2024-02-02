//////////////////////////////////////////////////////////////////////////////
//
//                     (C) Copyright 2019 by Autodesk, Inc.
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

#include <foundation/RCSharedPtr.h>
#include <foundation/RCFilesystem.h>
#include <data/RCPointBuffer.h>
#include <importexport/RCImportOptions.h>

namespace Autodesk { namespace RealityComputing { namespace Data {

    /// \brief RCProjectImportSession provides asynchronous APIs to import multiple scans into a ReCap project via in-memory point buffers.
    class RC_PROJECTIO_API RCProjectImportSession final
    {
    private:
        /// \brief Default constructor
        RCProjectImportSession();

    public:
        /// \brief Default destructor
        virtual ~RCProjectImportSession();

        RCProjectImportSession(const RCProjectImportSession& other) = delete;
        RCProjectImportSession& operator=(const RCProjectImportSession& other) = delete;

    public:
        /// \brief Initialize a project import session to import multiple scans via in-memory point buffers into a ReCap project.
        /// \note The returned project import session instance must be kept alive until the import process completes, fails or
        /// is cancelled. Otherwise, the import process will be cancelled.
        /// \note The progress and completion callbacks may be called in a different thread.
        /// \param[in] projectfilePath The ReCap project file path to create or append to.
        /// \param[in] fileMode The file mode for project (New or Append).
        /// \param[out] errorCode The returned error code.
        /// \param[in] projectProgressCallback The optional callback function pointer for the project import progress.
        /// \param[in] projectCompletionCallback The optional callback function pointer for the project import completion.
        /// \return A newly created \b RCProjectImportSession instance if successful. \b nullptr otherwise.
        static Foundation::RCSharedPtr<RCProjectImportSession> init(const Foundation::RCString& projectFilePath, Foundation::RCFileMode fileMode,
                                                                    Foundation::RCCode& errorCode,
                                                                    RCImportProgressCallbackPtr projectProgressCallback     = nullptr,
                                                                    RCImportCompletionCallbackPtr projectCompletionCallback = nullptr);

        /// \brief Add a scan specified by \p metadata for import.
        ///        This scan will become the current scan for the following addPointsToScan() and processScan() methods.
        ///        Scan name is used as a unique identifier of the scan, so duplicate scan names are not allowed in metadata of different scans.
        /// \param[in] metadata The metadata for the scan to import.
        /// \param[in] importOptions The import options to use when importing the scan.
        /// \return An \b RCCode indicating the result of this operation.
        Foundation::RCCode addScan(const RCScanMetadata& metadata, const RCScanImportOptions& importOptions);

        /// \brief Add points from point buffer into the current scan.
        /// \param[in] buffer The scan data populated into an \b RCPointBuffer.
        /// \return An \b RCCode indicating the result of this operation.
        ///         If a scan is not initialized or the import session is cancelled, no points can be added into the scan.
        Foundation::RCCode addPointsToScan(const RCPointBuffer& buffer);

        /// \brief Indicate that all points have been added to the current scan and that processing of the scan can begin.
        ///        This scan will be locked and no more points can be added into this scan after processScan().
        /// \note This is an asynchronous API. Progress and completion callbacks may be called in a different thread.
        /// \param[in] scanProgressCallback The callback function pointer for the scan import progress.
        /// \param[in] scanCompletionCallback The callback function pointer for the scan import completion.
        void processScan(RCImportProgressCallbackPtr scanProgressCallback = nullptr, RCScanImportCompletionCallbackPtr scanCompletionCallback = nullptr);

        /// \brief Indicate that no more scans can be added for this session so that the project file can be finalized.
        /// \note After calling this API, any unprocessed scan will be discarded.
        void endSession();

        /// \brief Cancel the asynchronous import of scans and creation of a ReCap project.
        void cancel();

        /// \brief Cancel the asynchronous import of the specified scan.
        /// \param[in] scanName The name of the scan to be cancelled.
        void cancelScan(const Foundation::RCString& scanName);

        /// \brief Block the current thread and wait for the asynchronous scan imports to finish.
        void waitTillFinished();

    private:
        class Impl;
        Impl* mImpl;
    };
}}}    // namespace Autodesk::RealityComputing::Data
