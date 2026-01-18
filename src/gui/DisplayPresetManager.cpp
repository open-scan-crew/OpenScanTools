#include "gui/DisplayPresetManager.h"

#include "controller/controls/ControlApplication.h"
#include "controller/controls/ControlSpecial.h"
#include "gui/Dialog/DialogDisplayPresets.h"
#include "gui/Dialog/MessageSplashScreen.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "gui/toolBars/ToolBarRenderSettings.h"
#include "gui/toolBars/ToolBarShowHideGroup.h"
#include "io/SerializerKeys.h"
#include "utils/JsonWriter.h"
#include "utils/System.h"
#include "models/graph/CameraNode.h"

#include "magic_enum/magic_enum.hpp"

#include <algorithm>
#include <fstream>

namespace
{
	const QString kInitialPresetName = QStringLiteral("Initial");
	const char kDisplayPresetFileName[] = "Display_presets.tlt";
	const char kDefaultPresetKey[] = "DefaultPreset";
	const char kPresetsKey[] = "Presets";
	const char kPresetNameKey[] = "Name";
	const char kPresetDisplayParametersKey[] = "DisplayParameters";
	const char kPresetShowHideKey[] = "ShowHideOptions";

	nlohmann::json serializeDisplayParameters(const DisplayParameters& params)
	{
		nlohmann::json json;
		json[Key_Rendering_Mode] = magic_enum::enum_name(params.m_mode);
		json[Key_Background_Color] = { params.m_backgroundColor.Red(), params.m_backgroundColor.Green(), params.m_backgroundColor.Blue() };
		json[Key_Point_Size] = params.m_pointSize;
		json[Key_Texel_Threshold] = params.m_texelThreshold;
		json[Key_Delta_Filling] = params.m_deltaFilling;

		json[Key_Contrast] = params.m_contrast;
		json[Key_Brightness] = params.m_brightness;
		json[Key_Saturation] = params.m_saturation;
		json[Key_Luminance] = params.m_luminance;
		json[Key_Blending] = params.m_hue;
		json[Key_Flat_Color] = { params.m_flatColor.x, params.m_flatColor.y, params.m_flatColor.z };

		json[Key_DistRamp] = { params.m_distRampMin, params.m_distRampMax };
		json[Key_DistRampSteps] = params.m_distRampSteps;

		json[Key_Blend_Mode] = magic_enum::enum_name(params.m_blendMode);
		json[Key_NegativeEffect] = params.m_negativeEffect;
		json[Key_ReduceFlash] = params.m_reduceFlash;
		json[Key_FlashAdvanced] = params.m_flashAdvanced;
		json[Key_FlashControl] = params.m_flashControl;
		json[Key_Transparency] = params.m_transparency;

		json[Key_Post_Rendering_Normals] = { params.m_postRenderingNormals.show, params.m_postRenderingNormals.inverseTone, params.m_postRenderingNormals.blendColor, params.m_postRenderingNormals.normalStrength, params.m_postRenderingNormals.gloss };
		json[Key_Post_Rendering_Ambient_Occlusion] = { params.m_postRenderingAmbientOcclusion.enabled, params.m_postRenderingAmbientOcclusion.radius, params.m_postRenderingAmbientOcclusion.intensity };
		json[Key_Edge_Aware_Blur] = { params.m_edgeAwareBlur.enabled, params.m_edgeAwareBlur.radius, params.m_edgeAwareBlur.depthThreshold, params.m_edgeAwareBlur.blendStrength, params.m_edgeAwareBlur.resolutionScale };
		json[Key_Depth_Lining] = { params.m_depthLining.enabled, params.m_depthLining.strength, params.m_depthLining.threshold, params.m_depthLining.sensitivity, params.m_depthLining.strongMode };

		json[Key_Display_Guizmo] = params.m_displayGizmo;
		json[Key_Ramp_Scale_Options] = { params.m_rampScale.showScale, params.m_rampScale.graduationCount, params.m_rampScale.centerBoxScale };

		json[Key_Alpha_Object] = params.m_alphaObject;
		json[Key_Distance_Unit] = magic_enum::enum_name(params.m_unitUsage.distanceUnit);
		json[Key_Diameter_Unit] = magic_enum::enum_name(params.m_unitUsage.diameterUnit);
		json[Key_Volume_Unit] = magic_enum::enum_name(params.m_unitUsage.volumeUnit);
		json[Key_Displayed_Digits] = params.m_unitUsage.displayedDigits;
		json[Key_Measure_Show_Mask] = params.m_measureMask;
		json[Key_Marker_Show_Mask] = params.m_markerMask;
		json[Key_Marker_Rendering_Parameters] = { params.m_markerOptions.improveVisibility, params.m_markerOptions.maximumDisplayDistance, params.m_markerOptions.nearLimit, params.m_markerOptions.farLimit, params.m_markerOptions.nearSize, params.m_markerOptions.farSize };
		json[Key_Text_Display_Options] = { params.m_textOptions.m_filter, params.m_textOptions.m_textTheme, params.m_textOptions.m_textFontSize };
		json[Key_Display_All_Marker_Texts] = params.m_displayAllMarkersTexts;
		json[Key_Display_All_Measures] = params.m_displayAllMeasures;

		json[Key_Ortho_Grid_Active] = params.m_orthoGridActive;
		json[Key_Ortho_Grid_Color] = { params.m_orthoGridColor.r, params.m_orthoGridColor.g, params.m_orthoGridColor.b, params.m_orthoGridColor.a };
		json[Key_Ortho_Grid_Step] = params.m_orthoGridStep;
		json[Key_Ortho_Grid_Linewidth] = params.m_orthoGridLineWidth;

		return json;
	}

	bool deserializeDisplayParameters(const nlohmann::json& json, DisplayParameters& data)
	{
		bool retVal = true;

		if (json.find(Key_Rendering_Mode) != json.end())
		{
			auto mode = magic_enum::enum_cast<UiRenderMode>(json.at(Key_Rendering_Mode).get<std::string>());
			data.m_mode = mode.has_value() ? mode.value() : UiRenderMode::Intensity;
		}
		else
		{
			retVal = false;
		}

		if (json.find(Key_Background_Color) != json.end())
		{
			nlohmann::json color = json.at(Key_Background_Color);
			data.m_backgroundColor = Color32{ color[0], color[1], color[2], 0 };
		}
		else
		{
			retVal = false;
		}

		if (json.find(Key_Point_Size) != json.end())
			data.m_pointSize = json.at(Key_Point_Size).get<float>();
		else
			retVal = false;

		if (json.find(Key_Texel_Threshold) != json.end())
			data.m_texelThreshold = json.at(Key_Texel_Threshold).get<int>();
		else
			data.m_texelThreshold = 4;

		if (json.find(Key_Delta_Filling) != json.end())
			data.m_deltaFilling = json.at(Key_Delta_Filling).get<float>();
		else
			retVal = false;

		if (json.find(Key_Alpha_Object) != json.end())
			data.m_alphaObject = json.at(Key_Alpha_Object).get<float>();
		else
			retVal = false;

		if (json.find(Key_Contrast) != json.end())
			data.m_contrast = json.at(Key_Contrast).get<float>();
		else
			retVal = false;

		if (json.find(Key_Brightness) != json.end())
			data.m_brightness = json.at(Key_Brightness).get<float>();
		else
			retVal = false;

		if (json.find(Key_Saturation) != json.end())
			data.m_saturation = json.at(Key_Saturation).get<float>();
		else
			retVal = false;

		if (json.find(Key_Luminance) != json.end())
			data.m_luminance = json.at(Key_Luminance).get<float>();
		else
			retVal = false;

		if (json.find(Key_Blending) != json.end())
			data.m_hue = json.at(Key_Blending).get<float>();
		else
			retVal = false;

		if (json.find(Key_Flat_Color) != json.end())
		{
			nlohmann::json color = json.at(Key_Flat_Color);
			data.m_flatColor = glm::vec3(color[0], color[1], color[2]);
		}
		else
		{
			retVal = false;
		}

		if (json.find(Key_DistRamp) != json.end())
		{
			nlohmann::json distRamp = json.at(Key_DistRamp);
			data.m_distRampMin = distRamp[0];
			data.m_distRampMax = distRamp[1];
		}
		else
		{
			retVal = false;
		}

		if (json.find(Key_DistRampSteps) != json.end())
			data.m_distRampSteps = json.at(Key_DistRampSteps).get<int>();

		if (json.find(Key_Blend_Mode) != json.end())
		{
			auto mode = magic_enum::enum_cast<BlendMode>(json.at(Key_Blend_Mode).get<std::string>());
			data.m_blendMode = mode.has_value() ? mode.value() : BlendMode::Opaque;
		}
		else
		{
			retVal = false;
		}

		if (json.find(Key_NegativeEffect) != json.end())
			data.m_negativeEffect = json.at(Key_NegativeEffect).get<bool>();
		else
			data.m_negativeEffect = false;

		if (json.find(Key_ReduceFlash) != json.end())
			data.m_reduceFlash = json.at(Key_ReduceFlash).get<bool>();

		if (json.find(Key_FlashAdvanced) != json.end())
			data.m_flashAdvanced = json.at(Key_FlashAdvanced).get<bool>();
		else
			data.m_flashAdvanced = false;

		if (json.find(Key_FlashControl) != json.end())
			data.m_flashControl = json.at(Key_FlashControl).get<float>();
		else
			data.m_flashControl = 50.f;

		if (json.find(Key_Transparency) != json.end())
			data.m_transparency = json.at(Key_Transparency).get<float>();
		else
			retVal = false;

		if (json.find(Key_Display_Guizmo) != json.end())
			data.m_displayGizmo = json.at(Key_Display_Guizmo).get<bool>();
		else
			retVal = false;

		if (json.find(Key_Ramp_Scale_Options) != json.end())
		{
			nlohmann::json options = json.at(Key_Ramp_Scale_Options);
			data.m_rampScale = { options[0].get<bool>(), options[2].get<bool>(), options[1].get<int>() };
		}

		if (json.find(Key_Distance_Unit) != json.end())
		{
			auto mode = magic_enum::enum_cast<UnitType>(json.at(Key_Distance_Unit).get<std::string>());
			if (mode.has_value())
				data.m_unitUsage.distanceUnit = mode.value();
		}

		if (json.find(Key_Diameter_Unit) != json.end())
		{
			auto mode = magic_enum::enum_cast<UnitType>(json.at(Key_Diameter_Unit).get<std::string>());
			if (mode.has_value())
				data.m_unitUsage.diameterUnit = mode.value();
		}

		if (json.find(Key_Volume_Unit) != json.end())
		{
			auto mode = magic_enum::enum_cast<UnitType>(json.at(Key_Volume_Unit).get<std::string>());
			if (mode.has_value())
				data.m_unitUsage.volumeUnit = mode.value();
		}

		if (json.find(Key_Displayed_Digits) != json.end())
			data.m_unitUsage.displayedDigits = json.at(Key_Displayed_Digits).get<int>();

		if (json.find(Key_Measure_Show_Mask) != json.end())
			data.m_measureMask = json.at(Key_Measure_Show_Mask).get<unsigned int>();
		else
			retVal = false;

		if (json.find(Key_Marker_Show_Mask) != json.end())
			data.m_markerMask = json.at(Key_Marker_Show_Mask).get<unsigned int>();
		else
			retVal = false;

		if (json.find(Key_Text_Display_Options) != json.end())
		{
			nlohmann::json options = json.at(Key_Text_Display_Options);
			data.m_textOptions = { options[0].get<unsigned int>(), options[1].get<int>(), options[2].get<float>() };
		}
		else
		{
			retVal = false;
		}

		if (json.find(Key_Display_All_Marker_Texts) != json.end())
			data.m_displayAllMarkersTexts = json.at(Key_Display_All_Marker_Texts).get<bool>();
		else
			retVal = false;

		if (json.find(Key_Display_All_Measures) != json.end())
			data.m_displayAllMeasures = json.at(Key_Display_All_Measures).get<bool>();
		else
			retVal = false;

		if (json.find(Key_Marker_Rendering_Parameters) != json.end())
		{
			nlohmann::json options = json.at(Key_Marker_Rendering_Parameters);
			data.m_markerOptions = { options[0], options[1], options[2], options[3], options[4], options[5] };
		}
		else
		{
			retVal = false;
		}

		if (json.find(Key_Post_Rendering_Normals) != json.end())
		{
			nlohmann::json options = json.at(Key_Post_Rendering_Normals);
			if (options.size() == 5)
				data.m_postRenderingNormals = { options[0], options[1], options[2], options[3], options[4] };
		}
		else
		{
			retVal = false;
		}

		if (json.find(Key_Post_Rendering_Ambient_Occlusion) != json.end())
		{
			nlohmann::json options = json.at(Key_Post_Rendering_Ambient_Occlusion);
			if (options.size() == 3)
				data.m_postRenderingAmbientOcclusion = { options[0], options[1], options[2] };
		}

		if (json.find(Key_Edge_Aware_Blur) != json.end())
		{
			nlohmann::json options = json.at(Key_Edge_Aware_Blur);
			if (options.size() == 5)
				data.m_edgeAwareBlur = { options[0], options[1], options[2], options[3], options[4] };
		}

		if (json.find(Key_Depth_Lining) != json.end())
		{
			nlohmann::json options = json.at(Key_Depth_Lining);
			if (options.size() == 5)
				data.m_depthLining = { options[0], options[1], options[2], options[3], options[4] };
		}

		if (json.find(Key_Ortho_Grid_Active) != json.end())
			data.m_orthoGridActive = json.at(Key_Ortho_Grid_Active).get<bool>();
		else
			retVal = false;

		if (json.find(Key_Ortho_Grid_Color) != json.end())
		{
			nlohmann::json options = json.at(Key_Ortho_Grid_Color);
			if (options.size() == 4)
				data.m_orthoGridColor = Color32(options[0], options[1], options[2], options[3]);
			else
				data.m_orthoGridColor = Color32(128, 128, 128);
		}
		else
		{
			retVal = false;
		}

		if (json.find(Key_Ortho_Grid_Step) != json.end())
			data.m_orthoGridStep = json.at(Key_Ortho_Grid_Step).get<float>();
		else
			retVal = false;

		if (json.find(Key_Ortho_Grid_Linewidth) != json.end())
			data.m_orthoGridLineWidth = json.at(Key_Ortho_Grid_Linewidth).get<uint32_t>();
		else
			retVal = false;

		return retVal;
	}

	nlohmann::json serializeShowHideState(const DisplayPresetManager::ShowHideState& state)
	{
		return {
			{ "ShowTagMarkers", state.showTagMarkers },
			{ "ShowViewpointMarkers", state.showViewpointMarkers },
			{ "ShowClippings", state.showClippings },
			{ "ShowPoints", state.showPoints },
			{ "ShowPipes", state.showPipes },
			{ "ShowAll", state.showAll },
			{ "ShowObjectTexts", state.showObjectTexts },
			{ "ShowSelected", state.showSelected },
			{ "ShowUnselected", state.showUnselected },
			{ "ShowMeasures", state.showMeasures }
		};
	}

	DisplayPresetManager::ShowHideState deserializeShowHideState(const nlohmann::json& json, const DisplayPresetManager::ShowHideState& fallback)
	{
		DisplayPresetManager::ShowHideState state = fallback;
		if (!json.is_object())
			return state;

		state.showTagMarkers = json.value("ShowTagMarkers", state.showTagMarkers);
		state.showViewpointMarkers = json.value("ShowViewpointMarkers", state.showViewpointMarkers);
		state.showClippings = json.value("ShowClippings", state.showClippings);
		state.showPoints = json.value("ShowPoints", state.showPoints);
		state.showPipes = json.value("ShowPipes", state.showPipes);
		state.showAll = json.value("ShowAll", state.showAll);
		state.showObjectTexts = json.value("ShowObjectTexts", state.showObjectTexts);
		state.showSelected = json.value("ShowSelected", state.showSelected);
		state.showUnselected = json.value("ShowUnselected", state.showUnselected);
		state.showMeasures = json.value("ShowMeasures", state.showMeasures);
		return state;
	}
}

DisplayPresetManager::DisplayPresetManager(IDataDispatcher& dataDispatcher, ToolBarShowHideGroup* showHideGroup, QWidget* parent)
	: QObject(parent)
	, m_dataDispatcher(dataDispatcher)
	, m_showHideGroup(showHideGroup)
	, m_defaultPresetName(kInitialPresetName)
{
	m_messageScreen = new MessageSplashScreen(parent);
	m_dialog = new DialogDisplayPresets(parent);

	connect(m_dialog->okButton(), &QPushButton::clicked, this, &DisplayPresetManager::handleDialogOk);
	connect(m_dialog->updateButton(), &QPushButton::clicked, this, &DisplayPresetManager::handleDialogUpdate);
	connect(m_dialog->deleteButton(), &QPushButton::clicked, this, &DisplayPresetManager::handleDialogDelete);
	connect(m_dialog->defaultButton(), &QPushButton::clicked, this, &DisplayPresetManager::handleDialogDefault);
	connect(m_dialog->initialPresetButton(), &QPushButton::clicked, this, &DisplayPresetManager::handleDialogInitialDefault);
	connect(m_dialog->cancelButton(), &QPushButton::clicked, this, &DisplayPresetManager::handleDialogCancel);

	registerGuiDataFunction(guiDType::projectLoaded, &DisplayPresetManager::onProjectLoad);
	registerGuiDataFunction(guiDType::renderActiveCamera, &DisplayPresetManager::onActiveCamera);
	registerGuiDataFunction(guiDType::focusViewport, &DisplayPresetManager::onFocusViewport);

	loadPresets();
	refreshRenderSettingsUi(m_defaultPresetName);
}

DisplayPresetManager::~DisplayPresetManager()
{
	m_dataDispatcher.unregisterObserver(this);
}

void DisplayPresetManager::registerRenderSettings(ToolBarRenderSettings* settings)
{
	if (!settings)
		return;

	m_renderSettings.push_back(settings);
	connect(settings, &ToolBarRenderSettings::displayPresetSelectionChanged, this, &DisplayPresetManager::handlePresetSelectionChanged);
	connect(settings, &ToolBarRenderSettings::displayPresetNewRequested, this, &DisplayPresetManager::handlePresetNewRequested);
	connect(settings, &ToolBarRenderSettings::displayPresetEditRequested, this, &DisplayPresetManager::handlePresetEditRequested);
	refreshRenderSettingsUi(m_defaultPresetName);
}

void DisplayPresetManager::informData(IGuiData* data)
{
	if (m_methods.find(data->getType()) != m_methods.end())
	{
		GuiDataFunction method = m_methods.at(data->getType());
		(this->*method)(data);
	}
}

void DisplayPresetManager::handlePresetSelectionChanged(const QString& name)
{
	if (m_updatingSelection)
		return;

	m_updatingSelection = true;
	for (ToolBarRenderSettings* settings : m_renderSettings)
		settings->setDisplayPresetSelection(name);
	m_updatingSelection = false;

	applyPresetByName(name);
}

void DisplayPresetManager::handlePresetNewRequested()
{
	openDialog(QString(), false);
}

void DisplayPresetManager::handlePresetEditRequested(const QString& name)
{
	if (name == kInitialPresetName)
		return;
	openDialog(name, true);
}

void DisplayPresetManager::handleDialogOk()
{
	const QString name = m_dialog->presetName().trimmed();
	if (!validatePresetName(name, m_editingPresetName))
		return;

	if (m_dialog->isEditMode())
	{
		DisplayPreset* preset = findPreset(m_editingPresetName);
		if (!preset)
			return;

		if (name != m_editingPresetName)
		{
			preset->name = name;
			if (m_defaultPresetName == m_editingPresetName)
				setDefaultPresetName(name);
		}
	}
	else
	{
		m_presets.push_back(buildPresetFromCurrent(name));
	}

	savePresets();
	refreshRenderSettingsUi(name);
	applyPresetByName(name);
	m_dialog->accept();
}

void DisplayPresetManager::handleDialogUpdate()
{
	if (!m_dialog->isEditMode())
		return;

	DisplayPreset* preset = findPreset(m_editingPresetName);
	if (!preset)
		return;

	preset->displayParameters = buildPresetFromCurrent(preset->name).displayParameters;
	preset->showHideState = getCurrentShowHideState();

	savePresets();
	applyPresetByName(preset->name);
	m_dialog->accept();
}

void DisplayPresetManager::handleDialogDelete()
{
	if (!m_dialog->isEditMode())
		return;

	const QString name = m_editingPresetName;
	auto it = std::remove_if(m_presets.begin(), m_presets.end(), [&](const DisplayPreset& preset) { return preset.name == name; });
	if (it != m_presets.end())
		m_presets.erase(it, m_presets.end());

	if (m_defaultPresetName == name)
		setDefaultPresetName(kInitialPresetName);

	savePresets();
	refreshRenderSettingsUi(kInitialPresetName);
	applyPresetByName(kInitialPresetName);
	m_dialog->accept();
}

void DisplayPresetManager::handleDialogDefault()
{
	const QString name = m_dialog->presetName().trimmed();
	if (!validatePresetName(name, m_editingPresetName))
		return;

	if (name == kInitialPresetName)
	{
		setDefaultPresetName(kInitialPresetName);
		refreshRenderSettingsUi(kInitialPresetName);
		applyPresetByName(kInitialPresetName);
		m_dialog->accept();
		return;
	}

	DisplayPreset* preset = findPreset(name);
	if (!preset)
	{
		m_presets.push_back(buildPresetFromCurrent(name));
		preset = &m_presets.back();
	}
	setDefaultPresetName(name);
	savePresets();
	refreshRenderSettingsUi(name);
	applyPresetByName(name);
	m_dialog->accept();
}

void DisplayPresetManager::handleDialogInitialDefault()
{
	setDefaultPresetName(kInitialPresetName);
	savePresets();
	refreshRenderSettingsUi(kInitialPresetName);
	applyPresetByName(kInitialPresetName);
}

void DisplayPresetManager::handleDialogCancel()
{
	m_dialog->reject();
}

void DisplayPresetManager::onProjectLoad(IGuiData* data)
{
	GuiDataProjectLoaded* plData = static_cast<GuiDataProjectLoaded*>(data);
	m_isProjectLoaded = plData->m_isProjectLoad;
	if (m_isProjectLoaded)
		m_applyDefaultPending = true;
}

void DisplayPresetManager::onFocusViewport(IGuiData* data)
{
	GuiDataFocusViewport* castData = static_cast<GuiDataFocusViewport*>(data);
	if (castData->m_forceFocus && castData->m_camera)
		m_focusCamera = castData->m_camera;
}

void DisplayPresetManager::onActiveCamera(IGuiData* data)
{
	auto infos = static_cast<GuiDataCameraInfo*>(data);
	if (infos->m_camera && m_focusCamera && m_focusCamera != infos->m_camera)
		return;

	if (infos->m_camera)
		m_focusCamera = infos->m_camera;

	if (m_applyDefaultPending && m_isProjectLoaded)
	{
		m_applyDefaultPending = false;
		applyPresetByName(m_defaultPresetName);
	}
}

void DisplayPresetManager::openDialog(const QString& presetName, bool editMode)
{
	m_editingPresetName = presetName;
	m_dialog->setPresetName(presetName);
	m_dialog->setEditMode(editMode);
	m_dialog->show();
}

void DisplayPresetManager::refreshRenderSettingsUi(const QString& selectedName)
{
	QStringList names;
	names << kInitialPresetName;
	for (const DisplayPreset& preset : m_presets)
		names << preset.name;

	for (ToolBarRenderSettings* settings : m_renderSettings)
		settings->setDisplayPresetNames(names, selectedName);
}

void DisplayPresetManager::applyPresetByName(const QString& name)
{
	if (!m_focusCamera)
		return;

	if (name == kInitialPresetName)
	{
		applyPreset(getInitialPreset());
		return;
	}

	const DisplayPreset* preset = findPreset(name);
	if (preset)
		applyPreset(*preset);
}

void DisplayPresetManager::applyPreset(const DisplayPreset& preset)
{
	if (!m_focusCamera)
		return;

	const DisplayParameters& params = preset.displayParameters;

	m_dataDispatcher.sendControl(new control::application::RenderModeUpdate(params.m_mode, m_focusCamera));
	m_dataDispatcher.sendControl(new control::application::SetRenderPointSize(static_cast<int>(params.m_pointSize), m_focusCamera));
	m_dataDispatcher.updateInformation(new GuiDataRenderTexelThreshold(params.m_texelThreshold, m_focusCamera), this);
	m_dataDispatcher.updateInformation(new GuiDataRenderBrightness(static_cast<int>(params.m_brightness), m_focusCamera), this);
	m_dataDispatcher.updateInformation(new GuiDataRenderContrast(static_cast<int>(params.m_contrast), m_focusCamera), this);
	m_dataDispatcher.updateInformation(new GuiDataRenderLuminance(static_cast<int>(params.m_luminance), m_focusCamera), this);
	m_dataDispatcher.updateInformation(new GuiDataRenderSaturation(static_cast<int>(params.m_saturation), m_focusCamera), this);
	m_dataDispatcher.updateInformation(new GuiDataRenderBlending(static_cast<int>(params.m_hue), m_focusCamera), this);
	m_dataDispatcher.updateInformation(new GuiDataRenderFlatColor(params.m_flatColor, m_focusCamera), this);
	m_dataDispatcher.updateInformation(new GuiDataRenderDistanceRampValues(params.m_distRampMin, params.m_distRampMax, params.m_distRampSteps, m_focusCamera), this);
	m_dataDispatcher.updateInformation(new GuiDataRenderTransparency(params.m_blendMode, params.m_transparency, m_focusCamera), this);
	m_dataDispatcher.updateInformation(new GuiDataRenderTransparencyOptions(params.m_negativeEffect, params.m_reduceFlash, params.m_flashAdvanced, params.m_flashControl, 0.f, m_focusCamera), this);
	m_dataDispatcher.updateInformation(new GuiDataPostRenderingNormals(params.m_postRenderingNormals, false, m_focusCamera), this);
	m_dataDispatcher.updateInformation(new GuiDataRenderAmbientOcclusion(params.m_postRenderingAmbientOcclusion, m_focusCamera), this);
	m_dataDispatcher.updateInformation(new GuiDataEdgeAwareBlur(params.m_edgeAwareBlur, m_focusCamera), this);
	m_dataDispatcher.updateInformation(new GuiDataDepthLining(params.m_depthLining, m_focusCamera), this);
	m_dataDispatcher.updateInformation(new GuiDataMarkerDisplayOptions(params.m_markerOptions, m_focusCamera), this);
	m_dataDispatcher.updateInformation(new GuiDataRenderTextFilter(params.m_textOptions.m_filter, m_focusCamera), this);
	m_dataDispatcher.updateInformation(new GuiDataRenderTextTheme(params.m_textOptions.m_textTheme, m_focusCamera), this);
	m_dataDispatcher.updateInformation(new GuiDataRenderTextFontSize(params.m_textOptions.m_textFontSize, m_focusCamera), this);
	m_dataDispatcher.updateInformation(new GuiDataRenderDisplayObjectTexts(params.m_displayAllMarkersTexts, m_focusCamera), this);
	m_dataDispatcher.updateInformation(new GuiDataAlphaObjectsRendering(params.m_alphaObject, m_focusCamera), this);
	m_dataDispatcher.sendControl(new control::application::SetOrthoGridParameters(params.m_orthoGridActive, params.m_orthoGridColor, params.m_orthoGridStep, params.m_orthoGridLineWidth));

	{
		WritePtr<CameraNode> wCam = m_focusCamera.get();
		if (wCam)
			wCam->setMarkerShowMask(params.m_markerMask);
	}

	if (m_showHideGroup)
	{
		ToolBarShowHideGroup::ShowHideState state = {};
		state.showTagMarkers = preset.showHideState.showTagMarkers;
		state.showViewpointMarkers = preset.showHideState.showViewpointMarkers;
		state.showClippings = preset.showHideState.showClippings;
		state.showPoints = preset.showHideState.showPoints;
		state.showPipes = preset.showHideState.showPipes;
		state.showAll = preset.showHideState.showAll;
		state.showObjectTexts = preset.showHideState.showObjectTexts;
		state.showSelected = preset.showHideState.showSelected;
		state.showUnselected = preset.showHideState.showUnselected;
		state.showMeasures = preset.showHideState.showMeasures;
		m_showHideGroup->applyShowHideState(state);
	}

	m_dataDispatcher.updateInformation(new GuiDataCameraInfo(m_focusCamera), this);
}

DisplayPresetManager::DisplayPreset DisplayPresetManager::buildPresetFromCurrent(const QString& name) const
{
	DisplayPreset preset;
	preset.name = name;
	preset.showHideState = getCurrentShowHideState();

	ReadPtr<CameraNode> rCam = m_focusCamera.cget();
	if (rCam)
		preset.displayParameters = rCam->getDisplayParameters();

	return preset;
}

DisplayPresetManager::ShowHideState DisplayPresetManager::getCurrentShowHideState() const
{
	if (m_showHideGroup)
	{
		ToolBarShowHideGroup::ShowHideState state = m_showHideGroup->currentShowHideState();
		ShowHideState result;
		result.showTagMarkers = state.showTagMarkers;
		result.showViewpointMarkers = state.showViewpointMarkers;
		result.showClippings = state.showClippings;
		result.showPoints = state.showPoints;
		result.showPipes = state.showPipes;
		result.showAll = state.showAll;
		result.showObjectTexts = state.showObjectTexts;
		result.showSelected = state.showSelected;
		result.showUnselected = state.showUnselected;
		result.showMeasures = state.showMeasures;
		return result;
	}
	return getDefaultShowHideState();
}

DisplayPresetManager::ShowHideState DisplayPresetManager::getDefaultShowHideState() const
{
	return ShowHideState{};
}

DisplayPresetManager::DisplayPreset DisplayPresetManager::getInitialPreset() const
{
	DisplayPreset preset;
	preset.name = kInitialPresetName;
	preset.displayParameters = DisplayParameters{};
	preset.showHideState = getDefaultShowHideState();
	return preset;
}

bool DisplayPresetManager::presetExists(const QString& name) const
{
	return findPreset(name) != nullptr;
}

DisplayPresetManager::DisplayPreset* DisplayPresetManager::findPreset(const QString& name)
{
	for (DisplayPreset& preset : m_presets)
	{
		if (preset.name == name)
			return &preset;
	}
	return nullptr;
}

const DisplayPresetManager::DisplayPreset* DisplayPresetManager::findPreset(const QString& name) const
{
	for (const DisplayPreset& preset : m_presets)
	{
		if (preset.name == name)
			return &preset;
	}
	return nullptr;
}

void DisplayPresetManager::loadPresets()
{
	const std::filesystem::path filePath = Utils::System::getOSTProgramDataTemplatePath() / kDisplayPresetFileName;
	if (!std::filesystem::exists(filePath))
		return;

	std::ifstream fileStream(filePath);
	if (!fileStream)
		return;

	nlohmann::json jsonPresets;
	fileStream >> jsonPresets;

	if (jsonPresets.contains(kDefaultPresetKey))
		m_defaultPresetName = QString::fromUtf8(jsonPresets.at(kDefaultPresetKey).get<std::string>().c_str());

	if (!jsonPresets.contains(kPresetsKey) || !jsonPresets.at(kPresetsKey).is_array())
		return;

	for (const nlohmann::json& entry : jsonPresets.at(kPresetsKey))
	{
		if (!entry.contains(kPresetNameKey))
			continue;

		DisplayPreset preset;
		preset.name = QString::fromUtf8(entry.at(kPresetNameKey).get<std::string>().c_str());
		if (preset.name.isEmpty() || preset.name == kInitialPresetName)
			continue;

		if (entry.contains(kPresetDisplayParametersKey))
		{
			DisplayParameters params;
			if (deserializeDisplayParameters(entry.at(kPresetDisplayParametersKey), params))
				preset.displayParameters = params;
		}

		const ShowHideState fallback = getDefaultShowHideState();
		if (entry.contains(kPresetShowHideKey))
			preset.showHideState = deserializeShowHideState(entry.at(kPresetShowHideKey), fallback);
		else
			preset.showHideState = fallback;

		m_presets.push_back(preset);
	}

	if (!presetExists(m_defaultPresetName))
		m_defaultPresetName = kInitialPresetName;
}

void DisplayPresetManager::savePresets() const
{
	const std::filesystem::path directory = Utils::System::getOSTProgramDataTemplatePath();
	Utils::System::createDirectoryIfNotExist(directory);

	nlohmann::json jsonPresets;
	jsonPresets[kDefaultPresetKey] = m_defaultPresetName.toUtf8().constData();

	nlohmann::json presetsArray = nlohmann::json::array();
	for (const DisplayPreset& preset : m_presets)
	{
		nlohmann::json entry;
		entry[kPresetNameKey] = preset.name.toUtf8().constData();
		entry[kPresetDisplayParametersKey] = serializeDisplayParameters(preset.displayParameters);
		entry[kPresetShowHideKey] = serializeShowHideState(preset.showHideState);
		presetsArray.push_back(entry);
	}
	jsonPresets[kPresetsKey] = presetsArray;

	utils::writeJsonFile(directory / kDisplayPresetFileName, jsonPresets);
}

void DisplayPresetManager::setDefaultPresetName(const QString& name)
{
	if (name.isEmpty())
	{
		m_defaultPresetName = kInitialPresetName;
		return;
	}
	m_defaultPresetName = name;
}

void DisplayPresetManager::showErrorMessage(const QString& message)
{
	if (m_messageScreen)
		m_messageScreen->setShowMessage(message);
}

bool DisplayPresetManager::validatePresetName(const QString& name, const QString& currentName) const
{
	if (name.isEmpty())
	{
		showErrorMessage(tr("Please enter a name for the preset."));
		return false;
	}

	if (name == kInitialPresetName)
	{
		showErrorMessage(tr("Initial cannot be used to name the preset. Please enter another name"));
		return false;
	}

	if (name != currentName && presetExists(name))
	{
		showErrorMessage(tr("This name is already used and cannot be used to name the preset. Please enter another name"));
		return false;
	}

	return true;
}
