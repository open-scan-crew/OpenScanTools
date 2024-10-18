#include "gui/GuiData/GuiData3dObjects.h"

GuiDataCallBackManipulatorMode::GuiDataCallBackManipulatorMode(const ManipulationMode& mode)
    : m_mode(mode)
{}

GuiDataManipulatorMode::GuiDataManipulatorMode(const ManipulationMode& mode)
    : m_mode(mode)
{}

GuiDataManipulatorLocGlob::GuiDataManipulatorLocGlob(const bool& invert, const bool& isLocal)
    : m_justInvert(invert)
    , m_isLocal(isLocal)
{}

GuiDataManipulatorSize::GuiDataManipulatorSize(const double& size)
    : m_size(size)
{}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//             Move To Data
//______________________________________

GuiDataMoveToData::GuiDataMoveToData(SafePtr<AGraphNode> object)
    : m_object(object)
{}

GuiDataMoveToData::GuiDataMoveToData(const GuiDataMoveToData& ref)
    : m_object(ref.m_object)
{}

GuiDataScreenshot::GuiDataScreenshot(const std::filesystem::path& filename, const ImageFormat& format)
	: m_filename(filename)
	, m_format(format)
{}
