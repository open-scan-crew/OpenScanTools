#ifndef GUI_DATA_3D_OBJECTS_H
#define GUI_DATA_3D_OBJECTS_H

#include "gui/GuiData/IGuiData.h"
#include "utils/safe_ptr.h"
#include "io/ImageTypes.h"

#include <filesystem>

enum class ManipulationMode;

class AGraphNode;
class CameraNode;
class IPanel;

class GuiDataCallBackManipulatorMode : public IGuiData
{
public:
    GuiDataCallBackManipulatorMode(const ManipulationMode& mode);
    ~GuiDataCallBackManipulatorMode() {};
    virtual guiDType getType() { return (guiDType::manipulatorModeCallBack); };

public:
    const ManipulationMode m_mode;
};

class GuiDataManipulatorMode : public IGuiData
{
public:
    GuiDataManipulatorMode(const ManipulationMode& mode);
    ~GuiDataManipulatorMode() {};
    virtual guiDType getType() { return (guiDType::manipulatorMode); };

public:
    const ManipulationMode m_mode;
};

class GuiDataManipulatorLocGlob : public IGuiData
{
public:
    GuiDataManipulatorLocGlob(const bool& invert, const bool& isLocal = true);
    ~GuiDataManipulatorLocGlob() {};
    virtual guiDType getType() { return (guiDType::manipulatorLocGlob); };

public:
    const bool m_justInvert;
    const bool m_isLocal;
};

class GuiDataManipulatorSize : public IGuiData
{
public:
    GuiDataManipulatorSize(const double& size);
    ~GuiDataManipulatorSize() {};
    virtual guiDType getType() { return (guiDType::manipulatorSize); };

public:
    const double m_size;
};

class GuiDataMoveToData : public IGuiData
{
public:
    GuiDataMoveToData(SafePtr<AGraphNode> object);
    GuiDataMoveToData(const GuiDataMoveToData& ref);
    ~GuiDataMoveToData() {};
    virtual guiDType getType() { return (guiDType::moveToData); };

public:
    SafePtr<AGraphNode> m_object;
};

class GuiDataScreenshot : public IGuiData
{
public:
	GuiDataScreenshot(const std::filesystem::path& filename, const ImageFormat& format, bool includeAlpha);
	~GuiDataScreenshot() {};
	virtual guiDType getType() { return (guiDType::screenshot); };

public:
	const std::filesystem::path m_filename;
	const ImageFormat m_format;
	const bool m_includeAlpha;
};

#endif
