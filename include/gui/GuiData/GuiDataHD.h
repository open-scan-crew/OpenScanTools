#ifndef GUI_DATA_HD_H
#define GUI_DATA_HD_H

#include "gui/GuiData/IGuiData.h"
#include "io/ImageTypes.h"
#include "utils/safe_ptr.h"

#include <glm/glm.hpp>

#include <cstdint>
#include <filesystem>

class CameraNode;

class GuiDataPrepareHDImage : public IGuiData
{
public:
    GuiDataPrepareHDImage(bool showFrame, bool showGrid, double frameRatio, SafePtr<CameraNode> viewport);
    ~GuiDataPrepareHDImage() {};
    virtual guiDType getType() override;

public:
    bool m_showFrame;
    bool m_showGrid;
    double m_frameRatio;
    SafePtr<CameraNode> m_viewport;
};


class GuiDataGenerateHDImage : public IGuiData
{
public:
    GuiDataGenerateHDImage(glm::ivec2 imageSize, int multisampling, ImageFormat format, SafePtr<CameraNode> camera, const std::filesystem::path& filepath, const ImageHDMetadata& metadata, bool showProgressBar, uint32_t hdimagetilesize, bool fullResolutionTraversal);
    ~GuiDataGenerateHDImage() {};
    virtual guiDType getType() override;

public:
    glm::ivec2 m_imageSize;
    int m_multisampling;
    ImageFormat m_imageFormat;
    SafePtr<CameraNode> m_camera;
    // TODO - Use a viewpoint ID
    std::filesystem::path m_filepath;
    ImageHDMetadata m_metadata;
    bool m_showProgressBar;
    uint32_t m_hdimagetilesize;
    bool m_fullResolutionTraversal;
};


class GuiDataCallImage : public IGuiData
{
public:
    GuiDataCallImage(bool callhdimage, std::filesystem::path filepath);
    ~GuiDataCallImage() {};
    virtual guiDType getType() override;
public:
    bool m_callHDImage = true;
    std::filesystem::path m_filepath;
};


#endif
