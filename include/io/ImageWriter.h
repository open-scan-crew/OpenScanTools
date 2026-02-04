#ifndef IMAGE_MANAGER_H_
#define IMAGE_MANAGER_H_

#include "vulkan/ImageTransferEvent.h"
#include "io/ImageTypes.h"
#include "gui/UnitUsage.h"
#include "utils/Color32.hpp"
#include "models/3d/TemperatureScaleData.h"

#include <filesystem>
#include <vector>

struct OrthoGridOverlay
{
    bool active = false;
    float step = 0.f;
    uint32_t lineWidth = 1;
    Color32 color = Color32(128, 128, 128);
    UnitType distanceUnit = UnitType::M;
};

struct RampScaleOverlay
{
    bool active = false;
    bool isTemperature = false;
    int graduation = 0;
    int steps = 0;
    double vmin = 0.0;
    double vmax = 0.0;
    bool temperatureAscending = true;
    UnitUsage unitUsage = unit_usage::by_default;
    std::vector<TemperatureScaleEntry> temperatureEntries;
};

class ImageWriter
{
public:
    ImageWriter();
    ~ImageWriter();

    bool saveScreenshot(const std::filesystem::path& filepath, ImageFormat format, ImageTransferEvent transfer, uint32_t width, uint32_t height, bool includeAlpha);

    bool startCapture(ImageFormat format, uint32_t width, uint32_t height, bool includeAlpha);
    void transferImageTile(ImageTransferEvent transfer, uint32_t dstOffsetW, uint32_t dstOffsetH, uint32_t border);
    void writeTile(const void* tileBuffer, uint32_t tileW, uint32_t tileH, uint32_t dstOffsetW, uint32_t dstOffsetH);
    void applyOrthoGridOverlay(const ImageHDMetadata& metadata, const OrthoGridOverlay& overlay);
    void applyRampScaleOverlay(const RampScaleOverlay& overlay);
    bool save(const std::filesystem::path& file_path, ImageHDMetadata metadata);

private:
    bool saveMetadata(const std::filesystem::path& path, ImageHDMetadata metadata);

    bool saveImage(const std::filesystem::path& filepath);

    bool allocateBuffer();

private:
    char* image_buffer_ = nullptr;
    size_t buffer_size_ = 0;

    uint32_t width_ = 0;
    uint32_t height_ = 0;
    ImageFormat format_ = ImageFormat::MAX_ENUM;
    uint32_t byte_per_pixel_ = 4;
    bool include_alpha_ = true;

    ImageHDMetadata metadata_;
};

#endif //! IMAGE_MANAGER_H_
