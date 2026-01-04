#include "gui/guiData/GuiDataHD.h"


GuiDataPrepareHDImage::GuiDataPrepareHDImage(bool showFrame, double frameRatio, SafePtr<CameraNode> viewport)
    : m_showFrame(showFrame)
    , m_frameRatio(frameRatio)
    , m_viewport(viewport)
{ }

guiDType GuiDataPrepareHDImage::getType()
{
    return guiDType::hdPrepare;
}

GuiDataGenerateHDImage::GuiDataGenerateHDImage(glm::ivec2 imageSize, int multisampling, ImageFormat format, SafePtr<CameraNode> camera, const std::filesystem::path& filepath, const ImageHDMetadata& metadata, bool showProgressBar, uint32_t hdimagetilesize, bool fullResolutionTraversal)
    : m_imageSize(imageSize)
    , m_multisampling(multisampling)
    , m_imageFormat(format)
    , m_camera(camera)
    , m_filepath(filepath)
    , m_metadata(metadata)
    , m_showProgressBar(showProgressBar)
    , m_hdimagetilesize(hdimagetilesize)
    , m_fullResolutionTraversal(fullResolutionTraversal)
{ }

guiDType GuiDataGenerateHDImage::getType()
{
    return guiDType::hdGenerate;
}

GuiDataCallImage::GuiDataCallImage(bool callhdimage, std::filesystem::path filepath)
    : m_callHDImage(callhdimage)
    , m_filepath(filepath)
{}

guiDType GuiDataCallImage::getType()
{
    return guiDType::hdCall;
}
