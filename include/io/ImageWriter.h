#ifndef IMAGE_MANAGER_H_
#define IMAGE_MANAGER_H_

#include "vulkan/ImageTransferEvent.h"
#include "io/ImageTypes.h"

#include <filesystem>

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
    ImageWriter();
    ~ImageWriter();

    static bool saveScreenshot(const std::filesystem::path& filepath, ImageFormat format, ImageTransferEvent transfer, uint32_t width, uint32_t height);

    bool startCapture(ImageFormat format, uint32_t width, uint32_t height, ImageHDMetadata metadata, bool showProgressBar);
    bool save(const std::filesystem::path& file_path, ImageFormat format);
    void transferImageTile(const WriteTask& task);


private:
    bool saveMetadata(const std::filesystem::path& path, TiledImage dstImage);

    static bool saveImage(const std::filesystem::path& filepath, TiledImage image);

    static char* allocateBuffer(uint32_t width, uint32_t height, uint32_t pixelSize);

private:
    char* image_buffer_ = nullptr;
    TiledImage dst_image_;
};

#endif //! IMAGE_MANAGER_H_