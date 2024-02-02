#include "io/ImageWriter.h"

#include <fstream>
#include <QImage>
#include "utils/Logger.h"
#include "utils/Utils.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/ScreenshotTexts.hpp"
#include "vulkan/VulkanManager.h"

#include "controller/controls/ControlFunction.h"
#include "controller/messages/GeneralMessage.h"


#define IOLOG Logger::log(LoggerMode::IOLog)


ImageWriter::ImageWriter(IDataDispatcher& dataDispatcher)
    : m_dataDispatcher(dataDispatcher)
{}

ImageWriter::~ImageWriter()
{
    if (m_hdThread.joinable())
        m_hdThread.join();
    std::lock_guard wait_busy_mutex(m_taskMutex);
}

bool ImageWriter::saveScreenshot(const std::filesystem::path& filepath, ImageFormat format, ImageTransferEvent transfer, uint32_t width, uint32_t height, IDataDispatcher& dataDispatcher)
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

    std::thread th = std::thread([filepath, task, dstImage, &dataDispatcher] {
        VulkanManager::getInstance().doImageTransfer(task.transfer, dstImage.width, dstImage.height, dstImage.buffer, 0u, 0u, 0u);
        saveImage(filepath, dstImage, false, dataDispatcher);
        delete[] dstImage.buffer;
    });
    th.detach();
    return true;
}

bool ImageWriter::startCapture(const std::filesystem::path& filepath, ImageFormat format, uint32_t width, uint32_t height, ImageHDMetadata metadata, bool showProgressBar)
{
    char* imageBuffer = allocateBuffer(width, height, 4);
    if (imageBuffer == nullptr)
        return false;

    {
        std::lock_guard lock(m_taskMutex);
        m_waitingTasks.clear();
    }

    TiledImage dstImage = {
        format,
        width,
        height,
        metadata,
        imageBuffer
    };

    m_stopCaptureThread.store(false);
    m_hdThread = std::thread([this, filepath, dstImage, showProgressBar] {
        runTasks(dstImage);
        saveImage(filepath, dstImage, showProgressBar, m_dataDispatcher);
        saveMetadata(filepath, dstImage);
        delete[] dstImage.buffer;
    });
    return true;
}

void ImageWriter::addTransfer(const WriteTask& task)
{
    std::lock_guard lock(m_taskMutex);
    m_waitingTasks.push_back(task);
}

void ImageWriter::endCapture()
{
    m_stopCaptureThread.store(true);
}

void ImageWriter::runTasks(TiledImage dstImage)
{
    while (m_stopCaptureThread.load() == false)
    {
        std::lock_guard lock(m_taskMutex);
        if (!m_waitingTasks.empty())
        {
            WriteTask task = m_waitingTasks.front();
            m_waitingTasks.pop_front();

            VulkanManager::getInstance().doImageTransfer(task.transfer, dstImage.width, dstImage.height, dstImage.buffer, task.dstOffsetW, task.dstOffsetH, 1u);
        }
    }
}

void ImageWriter::saveMetadata(std::filesystem::path path, TiledImage image)
{

    if (!image.metadata.ortho)
        return;

    std::ofstream os = std::ofstream(path.replace_extension(".txt"));
    if (os.is_open() == false)
    {
        m_dataDispatcher.updateInformation(new GuiDataProcessingSplashScreenLogUpdate(TEXT_SCREENSHOT_TEXT_FAILED));
        return;
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
    return;
}

void ImageWriter::saveImage(std::filesystem::path filepath, TiledImage image, bool showProgressBar, IDataDispatcher& dataDispatcher)
{
    // TODO - Essayer une option pour QImage::Format::Format_RGBA64
    QImage qImage((uchar*)image.buffer, image.width, image.height, QImage::Format::Format_ARGB32);
    filepath.replace_extension("." + ImageFormatDictio.at(image.format));
    bool ret(qImage.save(QString::fromStdWString(filepath.generic_wstring())));

    if (!ret)
    {
        if(showProgressBar)
            dataDispatcher.updateInformation(new GuiDataProcessingSplashScreenEnd(TEXT_SCREENSHOT_FAILED));
        IOLOG << "Failed to write " << filepath << LOGENDL;
    }
    else
    {
        if (showProgressBar)
        {
            dataDispatcher.updateInformation(new GuiDataProcessingSplashScreenLogUpdate(TEXT_SCREENSHOT_DONE.arg(filepath.generic_wstring())));
            dataDispatcher.updateInformation(new GuiDataProcessingSplashScreenEnd(TEXT_SCREENSHOT_PROCESSING_DONE));
        }
        dataDispatcher.updateInformation(new GuiDataTmpMessage(TEXT_SCREENSHOT_DONE.arg(filepath.generic_wstring())));
        dataDispatcher.sendControl(new control::function::ForwardMessage(new GeneralMessage(GeneralInfo::IMAGEEND)));
    }
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