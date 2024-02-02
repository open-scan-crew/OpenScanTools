#ifndef IMAGE_MANAGER_H_
#define IMAGE_MANAGER_H_

#include "vulkan/ImageTransferEvent.h"

#include "gui/IDataDispatcher.h"
#include "io/ImageTypes.h"

#include <filesystem>
#include <thread>
#include <deque>

struct WriteTask
{
    ImageTransferEvent      transfer;
    const uint32_t          dstOffsetW;
    const uint32_t          dstOffsetH;
};

struct TiledImage
{
    ImageFormat        format;
    uint32_t           width;
    uint32_t           height;
    ImageHDMetadata    metadata;
    char*              buffer;
};

class ImageWriter
{
public:
    ImageWriter(IDataDispatcher& dataDispatcher);
    ~ImageWriter();

    static bool saveScreenshot(const std::filesystem::path& filepath, ImageFormat format, ImageTransferEvent transfer, uint32_t width, uint32_t height, IDataDispatcher& dataDispatcher);

    bool startCapture(const std::filesystem::path& path, ImageFormat format, uint32_t width, uint32_t height, ImageHDMetadata metadata, bool showProgressBar);
    void addTransfer(const WriteTask& task);
    void endCapture();

private:
    void runTasks(TiledImage dstImage);
    void saveMetadata(std::filesystem::path path, TiledImage dstImage);

    static void saveImage(std::filesystem::path filepath, TiledImage image, bool showProgressBar, IDataDispatcher& dataDispatcher);

    static char* allocateBuffer(uint32_t width, uint32_t height, uint32_t pixelSize);

private:
    IDataDispatcher& m_dataDispatcher;

    //std::thread m_thread;

    // tiled image
    std::thread m_hdThread;
    std::atomic_bool m_stopCaptureThread;
    std::mutex m_taskMutex;
    std::deque<WriteTask> m_waitingTasks;
};

#endif //! IMAGE_MANAGER_H_