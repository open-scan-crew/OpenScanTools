#include "io/ImageWriter.h"
#include "utils/Logger.h"
#include "utils/Utils.h"
#include "vulkan/VulkanManager.h"

#include <cmath>
#include <fstream>
#include <limits>

#include <QtGui/qimage.h>

ImageWriter::ImageWriter()
{}

ImageWriter::~ImageWriter()
{
    delete[] image_buffer_;
    image_buffer_ = nullptr;
    buffer_size_ = 0;
}

bool ImageWriter::saveScreenshot(const std::filesystem::path& filepath, ImageFormat format, ImageTransferEvent transfer, uint32_t width, uint32_t height, bool includeAlpha)
{
    width_ = width;
    height_ = height;
    format_ = format;
    byte_per_pixel_ = format == ImageFormat::PNG16 ? 8u : 4u;
    include_alpha_ = includeAlpha;

    if (!allocateBuffer())
        return false;

    VulkanManager::getInstance().doImageTransfer(transfer, width_, height_, image_buffer_, buffer_size_, 0u, 0u, 0u, format == ImageFormat::PNG16);
    saveImage(filepath);

    return true;
}

bool ImageWriter::startCapture(ImageFormat format, uint32_t width, uint32_t height, bool includeAlpha)
{
    width_ = width;
    height_ = height;
    format_ = format;
    byte_per_pixel_ = format == ImageFormat::PNG16 ? 8u : 4u;
    include_alpha_ = includeAlpha;
    return allocateBuffer();
}

void ImageWriter::transferImageTile(ImageTransferEvent transfer, uint32_t dstOffsetW, uint32_t dstOffsetH, uint32_t border)
{
    VulkanManager::getInstance().doImageTransfer(transfer, width_, height_, image_buffer_, buffer_size_, dstOffsetW, dstOffsetH, border, format_ == ImageFormat::PNG16);
}

void ImageWriter::writeTile(const void* tileBuffer, uint32_t tileW, uint32_t tileH, uint32_t dstOffsetW, uint32_t dstOffsetH)
{
    if (!tileBuffer || !image_buffer_ || tileW == 0 || tileH == 0)
        return;

    const size_t bytesPerPixel = byte_per_pixel_;
    const size_t dstRowStride = bytesPerPixel * width_;
    const size_t srcRowStride = bytesPerPixel * tileW;

    for (uint32_t h = 0; h < tileH; ++h)
    {
        const size_t dstOffset = bytesPerPixel * (dstOffsetW + static_cast<size_t>(dstOffsetH + h) * width_);
        const char* srcRow = static_cast<const char*>(tileBuffer) + srcRowStride * h;
        memcpy_s(image_buffer_ + dstOffset, buffer_size_ - dstOffset, srcRow, srcRowStride);
    }
}

bool ImageWriter::save(const std::filesystem::path& file_path, ImageHDMetadata metadata)
{
    bool result = true;
    result &= saveImage(file_path);
    result &= saveMetadata(file_path, metadata);
    //m_dataDispatcher.updateInformation(new GuiDataProcessingSplashScreenLogUpdate(TEXT_SCREENSHOT_TEXT_FAILED));
    return result;
}

bool ImageWriter::saveMetadata(const std::filesystem::path& file_path, ImageHDMetadata metadata)
{
    if (!metadata.ortho)
        return true;

    std::filesystem::path metadata_path = file_path;

    std::ofstream os = std::ofstream(metadata_path.replace_extension(".txt"));
    if (os.is_open() == false)
    {
        return false;
    }

    os << "Image resolution" << std::endl;
    os << width_ << " x " << height_ << std::endl;
    os << std::endl;

    os << "Image date" << std::endl;
    os << metadata.date << std::endl;
    os << std::endl;

    os << "Image ratio" << std::endl;
    os << metadata.imageRatioLabel << std::endl;
    os << std::endl;

    os << "Image dimension L x H (m)" << std::endl;
    os << Utils::roundFloat(metadata.orthoSize.x, 3) << " x " << Utils::roundFloat(metadata.orthoSize.y, 3) << std::endl;
    os << std::endl;

    os << "Image scale" << std::endl;
    os << "1/" << (long)round(metadata.scaleInv) << std::endl;
    os << std::endl;

    os << "Image DPI" << std::endl;
    os << (long)round(metadata.dpi) << std::endl;
    os << std::endl;

    if (metadata.hasBottomZ)
    {
        os << "Z coordinate (m) - image bottom" << std::endl;
        os << Utils::roundFloat(metadata.imageBottomZ, 3) << std::endl;
        if (std::abs(metadata.importScanTranslation.z) > std::numeric_limits<double>::epsilon())
        {
            os << "Z original coordinate (m) - image bottom" << std::endl;
            os << Utils::roundFloat(metadata.imageBottomZ - metadata.importScanTranslation.z, 3) << std::endl;
        }
        os << std::endl;
    }

    if (metadata.hasVerticalCorners)
    {
        os << "XY coordinates (m), image bottom left" << std::endl;
        os << Utils::roundFloat(metadata.imageBottomLeft.x, 3) << " " << Utils::roundFloat(metadata.imageBottomLeft.y, 3) << std::endl;
        os << "XY coordinates (m), image top right" << std::endl;
        os << Utils::roundFloat(metadata.imageTopRight.x, 3) << " " << Utils::roundFloat(metadata.imageTopRight.y, 3) << std::endl;
        if (std::abs(metadata.importScanTranslation.x) > std::numeric_limits<double>::epsilon()
            || std::abs(metadata.importScanTranslation.y) > std::numeric_limits<double>::epsilon())
        {
            os << "XY original coordinates (m), image bottom left" << std::endl;
            os << Utils::roundFloat(metadata.imageBottomLeft.x - metadata.importScanTranslation.x, 3) << " "
               << Utils::roundFloat(metadata.imageBottomLeft.y - metadata.importScanTranslation.y, 3) << std::endl;
            os << "XY original coordinates (m), image top right" << std::endl;
            os << Utils::roundFloat(metadata.imageTopRight.x - metadata.importScanTranslation.x, 3) << " "
               << Utils::roundFloat(metadata.imageTopRight.y - metadata.importScanTranslation.y, 3) << std::endl;
        }
        os << std::endl;
    }

    os.close();
    return true;
}

bool ImageWriter::saveImage(const std::filesystem::path& file_path)
{
    assert(image_buffer_);    
    QImage qImage;
    if (format_ == ImageFormat::PNG16)
        qImage = QImage(reinterpret_cast<uchar*>(image_buffer_), width_, height_, QImage::Format::Format_RGBA64);
    else
        qImage = QImage(reinterpret_cast<uchar*>(image_buffer_), width_, height_, QImage::Format::Format_ARGB32);
    if (include_alpha_ == false)
    {
        if (format_ == ImageFormat::PNG16 || format_ == ImageFormat::TIFF)
            qImage = qImage.convertToFormat(QImage::Format::Format_RGBX64);
        else
            qImage = qImage.convertToFormat(QImage::Format::Format_RGB32);
    }
    std::filesystem::path ext_path = file_path;
    ext_path.replace_extension("." + getImageExtension(format_));
    bool ret(qImage.save(QString::fromStdWString(ext_path.generic_wstring())));

    return ret;
}

bool ImageWriter::allocateBuffer()
{
    const size_t required_size = static_cast<size_t>(width_) * height_ * byte_per_pixel_;
    if (required_size == 0)
        return false;
    if (image_buffer_ != nullptr)
    {
        // Reuse the existing buffer if it is already large enough
        if (buffer_size_ >= required_size)
            return true;

        delete[] image_buffer_;
        image_buffer_ = nullptr;
        buffer_size_ = 0;
    }

    buffer_size_ = required_size;
    try
    {
        image_buffer_ = new char[buffer_size_];
    }
    catch (std::exception& e)
    {
        IOLOG << "Failed to allocate memory for the image buffer. Size needed =  " << buffer_size_ << "\n Exception details :" << e.what() << Logger::endl;
        buffer_size_ = 0;
        return false;
    }
    return true;
}
