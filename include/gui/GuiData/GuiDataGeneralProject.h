#ifndef GUI_DATA_GENERAL_PROJECT_H
#define GUI_DATA_GENERAL_PROJECT_H

#include "gui/GuiData/IGuiData.h"

#include "models/Types.hpp"
#include "models/project/ProjectInfos.h"
#include "models/OpenScanToolsModelEssentials.h"
#include "controller/functionSystem/AContext.h"

#include "utils/Color32.hpp"

#include <unordered_set>

class AGraphNode;
class AClippingNode;
class CameraNode;

class GraphManager;
class ControllerContext;

class QString;

class GuiDataNewProject : public IGuiData
{
public:
	GuiDataNewProject();
    ~GuiDataNewProject();
    guiDType getType() override;
public:
    std::filesystem::path default_folder_;
    std::wstring default_name_;
    std::wstring default_company_;
    std::vector<std::filesystem::path> templates_;
};

class GuiDataAbort : public IGuiData
{
public:
	GuiDataAbort();
	~GuiDataAbort();
	guiDType getType() override;
};

class GuiDataQuit : public IGuiData
{
public:
	GuiDataQuit();
	~GuiDataQuit();
	guiDType getType() override;
};

class GuiDataProjectLoaded : public IGuiData
{
public:
	GuiDataProjectLoaded(bool projectLoad, const std::wstring& name);
	~GuiDataProjectLoaded();
	guiDType getType() override;
public:
	bool m_isProjectLoad;
    std::wstring m_projectName;
};

class GuiDataProjectTemplateList : public IGuiData
{
public:
	GuiDataProjectTemplateList(const std::vector<std::filesystem::path>& templates);
	~GuiDataProjectTemplateList();
	guiDType getType() override;
public:
	const std::vector<std::filesystem::path> m_templates;
};

class GuiDataHidePropertyPanels : public IGuiData
{
public:
    GuiDataHidePropertyPanels();
    GuiDataHidePropertyPanels(bool project, bool scan, bool tag, bool cluster);
    ~GuiDataHidePropertyPanels();
    guiDType getType() override;
public:
    bool hideProjectProperties;
    bool hideScanProperties;
    bool hideTagProperties;
    bool hideClusterProperties;
};


class GuiDataCameraPos : public IGuiData
{
public:
	GuiDataCameraPos();
	GuiDataCameraPos(const Pos3D& npos);
	~GuiDataCameraPos();
	guiDType getType() override;
public:
	Pos3D pos;
};

class GuiDataScanCurrent : public IGuiData
{
public:
	GuiDataScanCurrent(const Pos3D& npos);
	GuiDataScanCurrent();
	guiDType getType() override;
public:
	bool exists;
	Pos3D pos;
};

class GuiDataUndoRedoAble : public IGuiData
{
public:
	GuiDataUndoRedoAble(bool undoAble, bool redoAble);
	guiDType getType() override;
public:
	bool _undoAble;
	bool _redoAble;
};

class GuiDataObjectSelected : public IGuiData
{
public:
	GuiDataObjectSelected(SafePtr<AGraphNode> object);
	GuiDataObjectSelected();
	~GuiDataObjectSelected();
	guiDType getType() override;
public:
	SafePtr<AGraphNode> m_object;
	ElementType m_type = ElementType::None;
};

class GuiDataMultiObjectProperties : public IGuiData
{
public:
	GuiDataMultiObjectProperties(std::unordered_set<SafePtr<AGraphNode>> object);
	~GuiDataMultiObjectProperties();
	guiDType getType() override;
public:
	std::unordered_set<SafePtr<AGraphNode>> m_objects;
};

class GuiDataTolerances : public IGuiData
{
public:
	GuiDataTolerances(const double& bending, const double& tilt);
	~GuiDataTolerances();
	guiDType getType() override;
public:
	const double m_bending;
	const double m_tilt;
};

class GuiDataDefaultClipParams : public IGuiData
{
public:
	GuiDataDefaultClipParams(float min, float max, ClippingMode mode);
	~GuiDataDefaultClipParams();
	guiDType getType() override;

public:
	const ClippingMode m_mode;
	const float m_minClipDistance;
	const float m_maxClipDistance;
};

class GuiDataDefaultRampParams : public IGuiData
{
public:
	GuiDataDefaultRampParams(float min, float max, int steps);
	~GuiDataDefaultRampParams();
	guiDType getType() override;

public:
	const float m_minRampDistance;
	const float m_maxRampDistance;
	const int m_steps;
};

class GuiDataProjectProperties : public IGuiData
{
public:
	GuiDataProjectProperties(const ControllerContext& context, const GraphManager& graphManager, bool openProperties = true);
	guiDType getType() override;
public:
    ProjectInfos m_projectInfo;
	std::unordered_set<SafePtr<AGraphNode>> m_scans;

	uint64_t m_projectPointsCount;
	uint64_t m_projectScansCount;
	uint64_t m_projectActiveScansCount;

	bool m_openProperties;
};

class GuiDataActivatedFunctions : public IGuiData
{
public:
	GuiDataActivatedFunctions(const ContextType& type);
	guiDType getType() override;
public:
	ContextType type;
};

class GuiDataDeletePointsDialogDisplay : public IGuiData
{
public:
    GuiDataDeletePointsDialogDisplay(const std::vector<SafePtr<AClippingNode>>& clippings);
    ~GuiDataDeletePointsDialogDisplay();
    guiDType getType() override;

public:
    std::vector<SafePtr<AClippingNode>> m_clippings;
};

class GuiDataGlobalColorPickerValue : public IGuiData
{
public:
	GuiDataGlobalColorPickerValue(const Color32& color, const bool& picked = true);
	guiDType getType() override;
public:
	const bool m_isPicked;
	const Color32 m_color;
};

class GuiDataNameSelection : public IGuiData
{
public:
	GuiDataNameSelection(const std::wstring& name);
	guiDType getType() override;
public:
	const  std::wstring m_name;
};

class GuiDataIdentifierSelection : public IGuiData
{
public:
	GuiDataIdentifierSelection(const std::wstring& identifier);
	guiDType getType() override;
public:
	const  std::wstring m_identifier;
};

class GuiDataFocusViewport : public IGuiData
{
public:
	GuiDataFocusViewport(SafePtr<CameraNode> camera, int width, int height, bool forceFocus);
	guiDType getType() override;

	SafePtr<CameraNode> m_camera;
	int m_width;
	int m_height;
	bool m_forceFocus;
};

class GuiDataExamineOptions : public IGuiData
{
public :
	GuiDataExamineOptions(bool targetCentering, bool keepOnPan);
	~GuiDataExamineOptions();
	guiDType getType() override;

public :
	const bool m_targetCentering;
	const bool m_keepOnPan;
};

class GuiDataDeleteFileDependantObjectDialog : public IGuiData
{
public:
	GuiDataDeleteFileDependantObjectDialog(const std::unordered_map<SafePtr<AGraphNode>, std::pair<QString, QString>>& importantData, const std::unordered_set<SafePtr<AGraphNode>> & otherData);
	~GuiDataDeleteFileDependantObjectDialog();
	guiDType getType() override;
public:
	std::unordered_map<SafePtr<AGraphNode>, std::pair<QString, QString>> m_importantData;
	std::unordered_set<SafePtr<AGraphNode>> m_otherData;

};

class GuiDataProjectTemplateManagerDisplay : public IGuiData
{
public:
	GuiDataProjectTemplateManagerDisplay();
	~GuiDataProjectTemplateManagerDisplay();
	guiDType getType() override;
};

class GuiDataDisableFullScreen : public IGuiData
{
public:
	GuiDataDisableFullScreen();
	~GuiDataDisableFullScreen();
	guiDType getType() override;
};

class GuiDataSelectItems : public IGuiData
{
public:
	GuiDataSelectItems(const std::unordered_set<SafePtr<AGraphNode>>& objects);
	~GuiDataSelectItems();
	guiDType getType() override;
public:
	std::unordered_set<SafePtr<AGraphNode>> m_objects;
};

#endif
