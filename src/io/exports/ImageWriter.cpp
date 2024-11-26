#include "io/ImageWriter.h"

#include <fstream>
#include <QtGui/qimage.h>
#include "utils/Logger.h"
#include "utils/Utils.h"
#include "vulkan/VulkanManager.h"


ImageWriter::ImageWriter()
{}

ImageWriter::~ImageWriter()
{}

bool ImageWriter::saveScreenshot(const std::filesystem::path& filepath, ImageFormat format, ImageTransferEvent transfer, uint32_t width, uint32_t height)
{
    char* imageBuffer = allocateBuffer(width, height, 4);
    if (imageBuffer == nullptr)
        return false;

    WriteTask task = {
        transfer,
        0,
        0
    };

    TiledImage dstImage = {
        format,
        width,
        height,
        {},
        imageBuffer
    };

    VulkanManager::getInstance().doImageTransfer(task.transfer, dstImage.width, dstImage.height, dstImage.buffer, 0u, 0u, 0u);
    saveImage(filepath, dstImage);

    return true;
}

bool ImageWriter::startCapture(ImageFormat format, uint32_t width, uint32_t height, ImageHDMetadata metadata, bool showProgressBar)
{
    image_buffer_ = allocateBuffer(width, height, 4);
    if (image_buffer_ == nullptr)
        return false;

    dst_image_ = TiledImage{
        format,
        width,
        height,
        metadata,
        image_buffer_
    };

    return true;
}

bool ImageWriter::save(const std::filesystem::path& file_path, ImageFormat format)
{
    bool result = true;
    result &= saveImage(file_path, dst_image_);
    result &= saveMetadata(file_path, dst_image_);
    //m_dataDispatcher.updateInformation(new GuiDataProcessingSplashScreenLogUpdate(TEXT_SCREENSHOT_TEXT_FAILED));
    return result;
}

void ImageWriter::transferImageTile(const WriteTask& task)
{
    VulkanManager::getInstance().doImageTransfer(task.transfer, dst_image_.width, dst_image_.height, dst_image_.buffer, task.dstOffsetW, task.dstOffsetH, 1u);
}

bool ImageWriter::saveMetadata(const std::filesystem::path& file_path, TiledImage image)
{
    if (!image.metadata.ortho)
        return true;

    std::filesystem::path metadata_path = file_path;

    std::ofstream os = std::ofstream(metadata_path.replace_extension(".txt"));
    if (os.is_open() == false)
    {
        return false;
    }

    os << "Image resolution" << std::endl;
    os << image.width << " x " << image.height << std::endl;
    os << std::endl;

    os << "Image date" << std::endl;
    os << image.metadata.date << std::endl;
    os << std::endl;

    os << "Image ratio" << std::endl;
    os << image.metadata.imageRatioLabel << std::endl;
    os << std::endl;

    os << "Image dimension L x H (m)" << std::endl;
    os << Utils::roundFloat(image.metadata.orthoSize.x, 3) << " x " << Utils::roundFloat(image.metadata.orthoSize.y, 3) << std::endl;
    os << std::endl;

    os << "Image scale" << std::endl;
    os << "1/" << (long)round(image.metadata.scaleInv) << std::endl;
    os << std::endl;

    os << "Image DPI" << std::endl;
    os << (long)round(image.metadata.dpi) << std::endl;
    os << std::endl;

    os.close();
    return true;
}

bool ImageWriter::saveImage(const std::filesystem::path& file_path, TiledImage image)
{
    // TODO - Essayer une option pour QImage::Format::Format_RGBA64
    QImage qImage((uchar*)image.buffer, image.width, image.height, QImage::Format::Format_ARGB32);
    std::filesystem::path ext_path = file_path;
    ext_path.replace_extension("." + ImageFormatDictio.at(image.format));
    bool ret(qImage.save(QString::fromStdWString(ext_path.generic_wstring())));

    return ret;
}

char* ImageWriter::allocateBuffer(uint32_t width, uint32_t height, uint32_t pixelSize)
{
    char* buffer = nullptr;
    size_t bufferSize = (size_t)width * height * 4;
    try
    {
        buffer = new char[bufferSize];
    }
    catch (std::exception& e)
    {
        IOLOG << "Failed to allocate memory for the image buffer. Size needed =  " << bufferSize << "\n Exception details :" << e.what() << Logger::endl;
    }
    return buffer;
}