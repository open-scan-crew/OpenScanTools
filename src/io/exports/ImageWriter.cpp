#include "io/ImageWriter.h"
#include "utils/Logger.h"
#include "utils/Utils.h"
#include "vulkan/VulkanManager.h"

#include <fstream>

#include <QtGui/qimage.h>

ImageWriter::ImageWriter()
{}

ImageWriter::~ImageWriter()
{}

bool ImageWriter::saveScreenshot(const std::filesystem::path& filepath, ImageFormat format, ImageTransferEvent transfer, uint32_t width, uint32_t height)
{
    width_ = width;
    height_ = height;
    format_ = format;
    byte_per_pixel_ = format == ImageFormat::PNG16 ? 8u : 4u;

    if (!allocateBuffer())
        return false;

    VulkanManager::getInstance().doImageTransfer(transfer, width_, height_, image_buffer_, buffer_size_, 0u, 0u, 0u, format == ImageFormat::PNG16);
    saveImage(filepath);

    return true;
}

bool ImageWriter::startCapture(ImageFormat format, uint32_t width, uint32_t height)
{
    width_ = width;
    height_ = height;
    format_ = format;
    byte_per_pixel_ = format == ImageFormat::PNG16 ? 8u : 4u;
    return allocateBuffer();
}

void ImageWriter::transferImageTile(ImageTransferEvent transfer, uint32_t dstOffsetW, uint32_t dstOffsetH)
{
    VulkanManager::getInstance().doImageTransfer(transfer, width_, height_, image_buffer_, buffer_size_, dstOffsetW, dstOffsetH, 1u, format_ == ImageFormat::PNG16);
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
    std::filesystem::path ext_path = file_path;
    ext_path.replace_extension("." + getImageExtension(format_));
    bool ret(qImage.save(QString::fromStdWString(ext_path.generic_wstring())));

    return ret;
}

bool ImageWriter::allocateBuffer()
{
    if (image_buffer_ != nullptr)
        return false;
    buffer_size_ = (size_t)width_ * height_ * byte_per_pixel_;
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