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

#include <foundation/RCString.h>

namespace Autodesk { namespace RealityComputing { namespace Foundation {

    using Autodesk::RealityComputing::Foundation::RCBuffer;
    using Autodesk::RealityComputing::Foundation::RCString;

    class RCCompression
    {
    public:
        /// \brief  Compress the specified input buffer to the output buffer. This function can also
        ///         be used to determine the necessary buffer size prior to the actual compression.
        /// \param  inputBuffer Input buffer whose contents are to be compressed into the output
        ///         buffer. This is a mandatory parameter.
        /// \param  inputBytes The size of the input buffer in number of bytes. This is a mandatory
        ///         parameter whose value must be an integer greater than 0.
        /// \param  outputBuffer The output buffer to store the compressed data. If this parameter
        ///         is set to nullptr, the required output buffer size in number of bytes is returned
        ///         from \p outputBytes.
        /// \param  outputBytes If \p outputBuffer is nullptr, this parameter will contain the
        ///         estimated number of bytes required in \p outputBuffer for the compression. If
        ///         \p outputBuffer is not nullptr, then this parameter will be set to the actual
        ///         number of bytes written to the \p outputBuffer after the compression. Note that
        ///         the values for estimated and actual byte count may be different so it is important
        ///         to consume only the actual number of bytes when \p outputBuffer is not nullptr.
        /// \return This function returns false if the target size ends up being more than input size
        ///         (e.g. no compression achieved).
        static bool compressBuffer(const unsigned char* inputBuffer, size_t inputBytes, unsigned char* outputBuffer, size_t& outputBytes);

        /// \brief  Decompress the specified input buffer to the output buffer.
        /// \param  inputBuffer Input buffer whose contents are to be decompressed into the output
        ///         buffer. This is a mandatory parameter.
        /// \param  inputBytes The size of the input buffer in number of bytes. This is a mandatory
        ///         parameter whose value must be an integer greater than 0.
        /// \param  outputBuffer The output buffer to store the decompressed data.
        /// \param  outputBytes The maximum number of bytes that can be stored in \p outputBuffer.
        ///         This parameter is mandatory and must be of an integer value greater than zero.
        static void decompressBuffer(const unsigned char* inputBuffer, size_t inputBytes, unsigned char* outputBuffer, const size_t outputBytes);

        /// \brief  Compress the 2D input RGB buffer into a JPEG image file.
        /// \param  outputFilePath Absolute path to the output JPEG file to be generated. The output
        ///         path must point to a valid location where the calling process has write access to.
        /// \param  inputRgbBuffer The input buffer that contains a contiguous memory chunk of RGB
        ///         values. Each pixel in this the buffer is assumed to be 3-byte in size (i.e. RGB).
        /// \param  imageHeight The width of the input image to compress, in number of pixels.
        /// \param  imageWidth The height of the input image to compress, in number of pixels.
        /// \param  quality A quality percentage value range from 0 to 100. Default value is 98
        /// \return Returns true if the output JPEG file is successfully created, or false otherwise.
        static bool compressRGBImagetoJPEG(const RCString& outputFilePath, const unsigned char* inputRgbBuffer, size_t imageHeight, size_t imageWidth,
                                           const int quality = 98);

        /// \brief  Compress an in-memory RGB image buffer as an in-memory JPEG buffer.
        /// \param  inputRgbBuffer The input buffer that contains a contiguous memory chunk of RGB
        ///         values. Each pixel in this the buffer is assumed to be 3-byte in size (i.e. RGB).
        /// \param  imageHeight The width of the input image to compress, in number of pixels.
        /// \param  imageWidth The height of the input image to compress, in number of pixels.
        /// \param  outputImageBuffer Output buffer where resulting JPEG contents are written to. The
        ///         caller should use 'delete []' to free the allocated buffer after consuming its
        ///         contents.
        /// \param  outputBytes The number of bytes written into \p outputImageBuffer output buffer.
        /// \param  quality A quality percentage value range from 0 to 100. Default value is 98.
        /// \return Returns true if the JPEG content is successfully written to \p outputImageBuffer.
        static bool compressRGBImagetoJPEG(const unsigned char* inputRgbBuffer, size_t imageHeight, size_t imageWidth, unsigned char** outputImageBuffer,
                                           size_t& outputBytes, const int quality = 98);

        /// \brief  Decompress an in-memory JPEG data into a buffer as an RGB image. It is crucial for
        ///         the output buffer to be of sufficient size to accommodate the decompressed RGB data.
        /// \param  inputJpegData Input buffer containing data loaded from a Jpeg file. This parameter
        ///         is mandatory.
        /// \param  inputBytes The size of \p inputJpegData buffer in bytes.
        /// \param  outputRgbBuffer The output buffer in which decompressed RGB image data is to be
        ///         written to. If this parameter is nullptr, then the required buffer size is returned
        ///         through \p outputStride and \p outputHeight parameters. The total required buffer
        ///         size in bytes is equal to outputStride * outputHeight.
        /// \param  outputHeight The number of scan lines that can be stored in \p outputRgbBuffer. If
        ///         \p outputRgbBuffer is nullptr, this parameter contains the height of the resulting
        ///         RGB image.
        /// \param  outputStride The number of bytes in a single scan line of \p outputRgbBuffer. This
        ///         is the value used to move write pointer from one scan line to the next. If \p
        ///         outputRgbBuffer is nullptr, this parameter contains the number of bytes required by
        ///         a single scan line in the resulting RGB image.
        /// \return Returns true if the compression is successful, or false otherwise.
        static bool decompressJPEGImageToRGB(const unsigned char* inputJpegData, size_t inputBytes, unsigned char* outputRgbBuffer, size_t& outputHeight,
                                             size_t& outputStride);

        /// \brief  Compress the 2D input RGBA buffer into a PNG image file.
        /// \param  outputFilePath Absolute path to the output PNG file to be generated. The output
        ///         path must point to a valid location where the calling process has write access to.
        /// \param  inputRgbaBuffer The input buffer that contains a contiguous memory chunk of RGBA
        ///         values. Each pixel in this the buffer is assumed to be 4-byte in size (i.e. RGBA).
        /// \param  imageHeight The width of the input image to compress, in number of pixels.
        /// \param  imageWidth The height of the input image to compress, in number of pixels.
        /// \return Returns true if the output PNG file is successfully created, or false otherwise.
        static bool compressRGBAImageToPNG(const RCString& outputFilePath, const unsigned char* inputRgbaBuffer, size_t imageHeight, size_t imageWidth);
    };

    class RCZip
    {
    public:
        /// \brief Compress the given file: create a corresponding .gz file and remove the original
        static bool compressFile(const wchar_t* outputFilePath, const wchar_t* fileToCompress, bool append = false, bool excludePath = true,
                                 const wchar_t* subFolder = nullptr, bool isToCompressUnicode = false);

        /// \brief Uncompress the given zip file to target folder
        static bool decompressFile(const wchar_t* inputFileNameZip, const wchar_t* outputFolder);
    };    // namespace Zip

}}}    // namespace Autodesk::RealityComputing::Foundation
