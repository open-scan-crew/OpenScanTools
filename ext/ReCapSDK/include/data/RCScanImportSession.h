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

#include <data/RCPointBuffer.h>
#include <importexport/RCImportOptions.h>

namespace Autodesk { namespace RealityComputing { namespace Data {

    /// \brief RCScanImportSession provides asynchronous APIs for importing a scan via in-memory point buffers
    class RC_PROJECTIO_API RCScanImportSession final
    {
    private:
        RCScanImportSession();

    public:
        virtual ~RCScanImportSession();

        RCScanImportSession(const RCScanImportSession& other) = delete;
        RCScanImportSession& operator=(const RCScanImportSession& other) = delete;

        /// \brief Initialize a scan import session to import points into a scan.
        /// \note The returned scan import session instance must be kept alive until the import process completes, fails or is cancelled.
        /// Otherwise, the import process will be cancelled.
        /// \param[in] outputDirectory The output directory of the generated .rcc/.rcs files.
        /// \param[in] metadata Metadata of the scan to be imported.
        /// \param[in] importOptions Options to be applied during the import of the scan.
        /// \param[out] errorCode The returned error code.
        /// \return The newly created instance of RCScanImportSession if successful. \b nullptr otherwise.
        static Foundation::RCSharedPtr<RCScanImportSession> init(const Foundation::RCString& outputDirectory, const RCScanMetadata& metadata,
                                                                 const RCScanImportOptions& importOptions, Foundation::RCCode& errorCode);

        /// \brief Add points from point buffer into the scan.
        /// \param[in] buffer The scan data populated into an \b RCPointBuffer.
        /// \return An \b RCCode indicating the result of importing.
        Foundation::RCCode addPoints(const RCPointBuffer& buffer);

        /// \brief Indicate there are no more points to be added for this scan and that processing can be started.
        /// \note This is a non-blocking asynchronous API. The callbacks may be called in a different thread.
        /// \param[in] scanProgressCallback The optional callback function pointer for the scan import progress.
        /// \param[in] scanCompletionCallback The optional callback function pointer for the scan import completion.
        void process(RCImportProgressCallbackPtr scanProgressCallback = nullptr, RCScanImportCompletionCallbackPtr scanCompletionCallback = nullptr);

        /// \brief Cancel the scan import in progress.
        void cancel();

        /// \brief Block the current thread and wait for the asynchronous scan import to finish.
        void waitTillFinished();

    private:
        class Impl;
        Impl* mImpl;
    };

}}}    // namespace Autodesk::RealityComputing::Data
