#include "utils/Config.h"
#include "utils/Utils.h"

#include "utils/Logger.h"
#include "utils/JsonWriter.h"

#include "gui/Translator.h"
#include "utils/Color32.hpp"
#include "pointCloudEngine/RenderingTypes.h"
#include "gui/UnitUsage.h"
#include "models/project/ProjectTypes.h"
#include "models/3d/NavigationTypes.h"
#include "magic_enum/magic_enum.hpp"

#include <filesystem>
#include <fstream>
#include <string>

#include <direct.h> // WIN32

#define LOG_JSON_KEY "logs"
#define LICENSE_JSON_KEY "license"
#define LANG_JSON_KEY "lang"
#define CENTER_JSON_KEY "center"
#define KEEP_EXAMINE_JSON_KEY "keep_examine"
#define EXAMINE_DISPLAY_JSON_KEY "display_examine"
#define FRAMELESS_JSON_KEY "frameless"
#define PROJS_JSON_KEY "projs"
#define TEMP_JSON_KEY "temp"
#define FFMPEG_JSON_KEY "ffmpeg_path"
#define USERCOLOR_JSON_KEY "usercolor"
#define DECIMATE_OPTIONS_JSON_KEY "decimate_options"
#define DECIMATE_MODE_JSON_KEY "decimate_mode"
#define DECIMATE_CONST_VALUE_JSON_KEY "constant_value"
#define DECIMATE_ADAPTIVE_MIN_JSON_KEY "adaptive_minimum"
#define OCTREE_PRECISION_JSON_KEY "octree_precision"
#define RECENT_PROJECTS_JSON_KEY "recent_projects"
#define VALUE_DISPLAY_PARAMETERS_JSON_KEY "value_display_parameters"
#define DIGITS_JSON_KEY "digits"
#define UNIT_DIAMETER_JSON_KEY "unit_diameter"
#define UNIT_DISTANCE_JSON_KEY "unit_distance"
#define UNIT_VOLUME_JSON_KEY "unit_volume"
#define AUTOSAVE_JSON_KEY "autosave"
#define AUTOSAVE_TIMING_JSON_KEY "autosave_timing"
#define GIZMO_PARAMETERS_JSON_KEY "gizmo_parameters"
#define MANIPULATOR_SIZE "manipulator_size"
#define INDEXATION_METHOD "indexation_method"
#define GDEVICE_JSON_KEY "graphic_device"
#define RENDER_PERSPECTIVE_PLANS "render_perspective_plans"
#define RENDER_ORTHOGRAPHIC_Z_BOUND "render_orthographic_z_bound"
#define RENDER_NEAR_PLAN "near_plan"
#define RENDER_NEAR_FAR_RANGE "near_far_range"
#define NAVIGATION_PARAMETERS "navigation_parameters"
#define NAV_PARAM_EXAMINE_MIN_RADIUS "examine_min_radius"
#define NAV_PARAM_TRANSLATION_SPEED "translation_speed"
#define NAV_PARAM_ROTATION_EXAMINE_SPEED "rotation_examine_speed"
#define NAV_PARAM_MOUSE_INVERT "mouse_drag_camera_inverted"
#define NAV_PARAM_WHEEL_INVERT_JSON_KEY "wheel"
#define NAV_PARAM_IS_UNLOCK_SCAN_MANIPULATION_JSON_KEY "isUnlockScanManipulation"


static const std::vector<std::pair<LoggerMode, bool>> defaultLog = {
	{LoggerMode::TreeLog, true},
	{LoggerMode::IOLog, true},
	{LoggerMode::VKLog, true},
	{LoggerMode::ControlLog, true},
	{LoggerMode::DataLog, true},
	{LoggerMode::ControllerLog, true},
	{LoggerMode::GuiLog, true},
	{LoggerMode::LicenseLog, true},
	{LoggerMode::GTLog, true},
	{LoggerMode::GTExtraLog, false},
	{LoggerMode::FunctionLog, true},
	{LoggerMode::rayTracingLog, true},
	{LoggerMode::SceneGraphLog, true},
	{LoggerMode::TranslatorLog, true}
};
enum class LicenseParametersKey { Type, Server, Proxy, ActivationStep, LicenceKey_Old, LicenceKey};
static const std::unordered_map<std::string, LicenseParametersKey> licenseParametersDics = 
{
	{ "type", LicenseParametersKey::Type },
	{ "server", LicenseParametersKey::Server },
	{ "proxy", LicenseParametersKey::Proxy },
	{ "activationStep", LicenseParametersKey::ActivationStep },
	{ "licenceKey", LicenseParametersKey::LicenceKey_Old},
	{ "licenceKeyUtf8", LicenseParametersKey::LicenceKey}
};


static std::string cleanString(const std::string& str)
{
	std::string corrected(str);
	while (corrected.find("\"") != std::string::npos)
		corrected.erase(corrected.find("\""), 1);
	return corrected;
}

static std::wstring cleanWString(const std::wstring& wstr)
{
	std::wstring corrected(wstr);
	while (corrected.find(L"\"") != std::wstring::npos)
		corrected.erase(corrected.find(L"\""), 1);
	return corrected;
}

static std::wstring cleanWString(const std::string& str)
{
	std::wstring corrected(Utils::from_utf8(str));
	while (corrected.find(L"\"") != std::wstring::npos)
		corrected.erase(corrected.find(L"\""), 1);
	return corrected;
}

namespace Config
{
	static nlohmann::json jsonConfig = nlohmann::json();
	static bool isLoaded = false;
	static bool needSave = false;

	static std::filesystem::path applicationDirPath = "";
	static std::filesystem::path filePath = "";

	void loadConfigFile(const std::filesystem::path& filepath)
	{
		filePath = filepath;
		std::ifstream fileStream(filepath.c_str());
		if (fileStream.good() == false)
		{
			isLoaded = false;
			return;
		}
		isLoaded = true;
		try 
		{
			fileStream >> jsonConfig;
		}
		catch(std::exception&)
		{
			isLoaded = false;
		}
	}

	bool saveConfigFile(const std::filesystem::path& filepath)
	{

		if (!utils::writeJsonFile(filepath, jsonConfig))
			return false;
		needSave = false;
		return true;
	}

	bool isFileLoaded()
	{
		return (isLoaded);
	}


    // NOTE(robin) - Unused function
	bool findKey(std::string key)
	{
		if (isLoaded == true && jsonConfig.find(key) != jsonConfig.end())
			return (true);
		return (false);
	}

    std::vector<std::pair<LoggerMode, bool>> getLogMods()
    {
        std::vector<std::pair<LoggerMode, bool>> result;

        if (isLoaded == false)
            return (defaultLog);

        try
        {
            for (auto item : jsonConfig.at(LOG_JSON_KEY).items())
            {
			    auto mode = magic_enum::enum_cast<LoggerMode>(item.key());
			    if (mode.has_value() == true)
				    result.push_back({ mode.value(), item.value() });
            }
        }
        catch (std::exception&)
        {
			return (defaultLog);
        }

        return (result);
    }
	
	LanguageType getLanguage()
	{
		if (jsonConfig.find(LANG_JSON_KEY) == jsonConfig.end())
			return LanguageType::English;
		auto type = magic_enum::enum_cast<LanguageType>(cleanString(jsonConfig.at(LANG_JSON_KEY).dump()));
		if (!type.has_value())
			return LanguageType::English;
		return *type;
	}
	
	bool setLanguage(LanguageType type)
	{
		jsonConfig[LANG_JSON_KEY] = Translator::getLanguageQStr(type).toStdWString();
		return saveConfigFile(filePath);
	}

	bool getCenteringConfiguration()
	{
		if (jsonConfig.find(CENTER_JSON_KEY) == jsonConfig.end())
			return false;
		return jsonConfig.at(CENTER_JSON_KEY);
	}

	bool setCenteringConfiguration(bool value)
	{
		jsonConfig[CENTER_JSON_KEY] = value;
		return saveConfigFile(filePath);
	}

	bool getKeepingExamineConfiguration()
	{
		if (jsonConfig.find(KEEP_EXAMINE_JSON_KEY) == jsonConfig.end())
			return false;
		return jsonConfig.at(KEEP_EXAMINE_JSON_KEY);
	}

	bool setKeepingExamineConfiguration(bool value)
	{
		jsonConfig[KEEP_EXAMINE_JSON_KEY] = value;
		return saveConfigFile(filePath);
	}

	UnitUsage getUnitUsageConfiguration()
	{
		UnitUsage parameters(unit_usage::by_default);
		if (jsonConfig.find(VALUE_DISPLAY_PARAMETERS_JSON_KEY) != jsonConfig.end())
		{
			nlohmann::json parametersNode = jsonConfig[VALUE_DISPLAY_PARAMETERS_JSON_KEY];
			if (parametersNode.find(UNIT_DISTANCE_JSON_KEY) != parametersNode.end()) {
				std::optional<UnitType> optional = magic_enum::enum_cast<UnitType>(cleanString(parametersNode[UNIT_DISTANCE_JSON_KEY].dump()));
				if (optional.has_value())
					parameters.distanceUnit = *optional;
			}
			if (parametersNode.find(UNIT_DIAMETER_JSON_KEY) != parametersNode.end()) {
				std::optional<UnitType> optional = magic_enum::enum_cast<UnitType>(cleanString(parametersNode[UNIT_DIAMETER_JSON_KEY].dump()));
				if(optional.has_value())
					parameters.diameterUnit = *optional;
			}
			if (parametersNode.find(UNIT_VOLUME_JSON_KEY) != parametersNode.end()) {
				std::optional<UnitType> optional = magic_enum::enum_cast<UnitType>(cleanString(parametersNode[UNIT_VOLUME_JSON_KEY].dump()));
				if (optional.has_value())
					parameters.volumeUnit = *optional;
			}
			if (parametersNode.find(DIGITS_JSON_KEY) != parametersNode.end())
				parameters.displayedDigits = parametersNode[DIGITS_JSON_KEY];
		}
		return (parameters);
	}

	bool setUnitUsageConfiguration(const UnitUsage& parameters)
	{
		try {
			nlohmann::json parametersNode;
			parametersNode[DIGITS_JSON_KEY] = parameters.displayedDigits;
			parametersNode[UNIT_DIAMETER_JSON_KEY] = magic_enum::enum_name<UnitType>(parameters.diameterUnit);
			parametersNode[UNIT_DISTANCE_JSON_KEY] = magic_enum::enum_name<UnitType>(parameters.distanceUnit);
			parametersNode[UNIT_VOLUME_JSON_KEY] = magic_enum::enum_name<UnitType>(parameters.volumeUnit);
			jsonConfig[VALUE_DISPLAY_PARAMETERS_JSON_KEY] = parametersNode;
			return saveConfigFile(filePath);
		}
		catch (...)
		{
			return false;
		}
	}

	Color32 getUserColor()
	{
		if (jsonConfig.find(USERCOLOR_JSON_KEY) == jsonConfig.end())
			return Color32();
		nlohmann::json jcolors = jsonConfig.at(USERCOLOR_JSON_KEY);
		return Color32({ jcolors[0], jcolors[1], jcolors[2], jcolors[3] });
	}

	bool setUserColor(const Color32& type)
	{
		jsonConfig[USERCOLOR_JSON_KEY] = { type.r, type.g, type.b, type.a }; ;
		return saveConfigFile(filePath);
	}

    std::filesystem::path getApplicationDirPath()
    {
        return applicationDirPath;
    }

    void setApplicationDirPath(const std::filesystem::path& _path)
    {
        applicationDirPath = _path;
    }

	std::filesystem::path getProjectsPath()
	{
		if (jsonConfig.find(PROJS_JSON_KEY) == jsonConfig.end())
			return "";
		return cleanWString(jsonConfig.at(PROJS_JSON_KEY).dump());
	}

	bool setProjectsPath(const std::filesystem::path& type)
	{
		jsonConfig[PROJS_JSON_KEY] = Utils::to_utf8(type.wstring());
		return saveConfigFile(filePath);
	}

	std::filesystem::path getTemporaryPath()
	{
		if (jsonConfig.find(TEMP_JSON_KEY) == jsonConfig.end())
			return "";
		return cleanWString(jsonConfig.at(TEMP_JSON_KEY).dump());
	}

	bool setTemporaryPath(const std::filesystem::path& type)
	{
		jsonConfig[TEMP_JSON_KEY] = Utils::to_utf8(type.wstring());
		return saveConfigFile(filePath);
	}

	std::filesystem::path getFFmpegPath()
	{
		if (jsonConfig.find(FFMPEG_JSON_KEY) == jsonConfig.end())
			return "";
		return cleanWString(jsonConfig.at(FFMPEG_JSON_KEY).dump());
	}

	bool setFFmpegPath(const std::filesystem::path& type)
	{
		jsonConfig[FFMPEG_JSON_KEY] = Utils::to_utf8(type.wstring());
		return saveConfigFile(filePath);
	}

    std::filesystem::path getResourcesPath()
    {
        return applicationDirPath / "resources";
    }

    DecimationOptions getDecimationOptions()
    {
        DecimationOptions options;
        if (jsonConfig.find(DECIMATE_OPTIONS_JSON_KEY) != jsonConfig.end())
        {
            nlohmann::json optionsNode = jsonConfig[DECIMATE_OPTIONS_JSON_KEY];
            if (optionsNode.find(DECIMATE_MODE_JSON_KEY) != optionsNode.end())
                options.mode = optionsNode[DECIMATE_MODE_JSON_KEY];
            if (optionsNode.find(DECIMATE_CONST_VALUE_JSON_KEY) != optionsNode.end())
                options.constantValue = optionsNode[DECIMATE_CONST_VALUE_JSON_KEY];
            if (optionsNode.find(DECIMATE_ADAPTIVE_MIN_JSON_KEY) != optionsNode.end())
                options.dynamicMin = optionsNode[DECIMATE_ADAPTIVE_MIN_JSON_KEY];
        }
        return (options);
    }

	OctreePrecision getOctreePrecision()
	{
		if (jsonConfig.find(OCTREE_PRECISION_JSON_KEY) == jsonConfig.end())
			return OctreePrecision::Normal;

		int precision = jsonConfig.at(OCTREE_PRECISION_JSON_KEY);
		switch (precision)
		{
		case static_cast<int>(OctreePrecision::Analysis):
			return OctreePrecision::Analysis;
		case static_cast<int>(OctreePrecision::Performances):
			return OctreePrecision::Performances;
		case static_cast<int>(OctreePrecision::Normal):
		default:
			return OctreePrecision::Normal;
		}
	}

    bool setDecimationOptions(const DecimationOptions& options)
    {
        try {
            nlohmann::json optionsNode;
            optionsNode[DECIMATE_MODE_JSON_KEY] = options.mode;
            optionsNode[DECIMATE_CONST_VALUE_JSON_KEY] = options.constantValue;
            optionsNode[DECIMATE_ADAPTIVE_MIN_JSON_KEY] = options.dynamicMin;
            jsonConfig[DECIMATE_OPTIONS_JSON_KEY] = optionsNode;
            return saveConfigFile(filePath);
        }
        catch (...)
        {
            return false;
        }
    }

	bool setOctreePrecision(const OctreePrecision precision)
	{
		try {
			jsonConfig[OCTREE_PRECISION_JSON_KEY] = static_cast<int>(precision);
			return saveConfigFile(filePath);
		}
		catch (...)
		{
			return false;
		}
	}

    bool getMaximizedFrameless()
    {
        if (jsonConfig.find(FRAMELESS_JSON_KEY) == jsonConfig.end())
            return false;
        return jsonConfig.at(FRAMELESS_JSON_KEY);
    }

    bool setMaximizedFrameless(bool value)
    {
        jsonConfig[FRAMELESS_JSON_KEY] = value;
        return saveConfigFile(filePath);
    }

	bool getExamineDisplayMode()
	{
		if (jsonConfig.find(EXAMINE_DISPLAY_JSON_KEY) == jsonConfig.end())
			return false;
		return jsonConfig.at(EXAMINE_DISPLAY_JSON_KEY);
	}

	bool setExamineDisplayMode(bool value)
	{
		jsonConfig[EXAMINE_DISPLAY_JSON_KEY] = value;
		return saveConfigFile(filePath);
	}

	std::vector<std::pair<std::filesystem::path, time_t>> getRecentProjects()
	{
		std::vector<std::pair<std::filesystem::path, time_t>> retour;

		if (jsonConfig.find(RECENT_PROJECTS_JSON_KEY) == jsonConfig.end())
			return retour;

		try
		{
			auto projects = jsonConfig.at(RECENT_PROJECTS_JSON_KEY).get<std::vector<std::pair<std::string, time_t>>>();

			for (auto pair : projects)
				retour.push_back({ Utils::from_utf8(pair.first), pair.second });
		}
		catch(...)
		{
			Logger::log(LoggerMode::LogConfig) << "Failed to load recent project." << Logger::endl;
			//assert(false);
		}

		return retour;
	}

	bool setRecentProjects(std::vector<std::pair<std::filesystem::path, time_t>> projects)
	{
		std::vector<std::pair<std::string, time_t>> toSave;

		for (auto pair : projects)
			toSave.push_back({ Utils::to_utf8(pair.first.wstring()), pair.second });

		jsonConfig[RECENT_PROJECTS_JSON_KEY] = toSave;
		return saveConfigFile(filePath);
	}

	bool getIsAutoSaveActive()
	{
		if (jsonConfig.find(AUTOSAVE_JSON_KEY) == jsonConfig.end())
			return false;
		return jsonConfig.at(AUTOSAVE_JSON_KEY);
	}

	bool setIsAutoSaveActive(const bool& autosave)
	{
		jsonConfig[AUTOSAVE_JSON_KEY] = autosave;
		return saveConfigFile(filePath);
	}

	uint8_t getAutoSaveTiming()
	{
		if (jsonConfig.find(AUTOSAVE_TIMING_JSON_KEY) == jsonConfig.end())
			return 0;
		return jsonConfig.at(AUTOSAVE_TIMING_JSON_KEY);
	}

	bool setAutoSaveTiming(const uint8_t& timing)
	{
		jsonConfig[AUTOSAVE_TIMING_JSON_KEY] = timing;
		return saveConfigFile(filePath);
	}


	float* getGizmoParameters()
	{
		float* ret = new float[3];
		ret[0] = -0.85f; ret[1] = 0.9f; ret[2] = 0.1f;

		if (jsonConfig.find(GIZMO_PARAMETERS_JSON_KEY) == jsonConfig.end())
			return ret;
		const auto& vec = jsonConfig.at(GIZMO_PARAMETERS_JSON_KEY);
		ret[0] = vec[0]; ret[1] = vec[1]; ret[2] = vec[2];
		return ret;
	}

	bool setGizmoParameters(const float guizmo[3])
	{
		jsonConfig[GIZMO_PARAMETERS_JSON_KEY] = { guizmo[0],guizmo[1],guizmo[2] };
		return saveConfigFile(filePath);
	}

	float getManipulatorSize()
	{
		if (jsonConfig.find(MANIPULATOR_SIZE) == jsonConfig.end())
			return 50;
		return jsonConfig.at(MANIPULATOR_SIZE).get<float>();
	}

	bool setManipulatorSize(float manipulatorSize)
	{
		jsonConfig[MANIPULATOR_SIZE] = manipulatorSize;
		return saveConfigFile(filePath);
	}


	std::string getGraphicDevice()
	{
		if (jsonConfig.find(GDEVICE_JSON_KEY) == jsonConfig.end())
			return "";
		return jsonConfig.at(GDEVICE_JSON_KEY);
	}

	bool setGraphicDevice(const std::string name)
	{
		jsonConfig[GDEVICE_JSON_KEY] = name;
		return saveConfigFile(filePath);
	}

	IndexationMethod getIndexationMethod()
	{
		if (jsonConfig.find(INDEXATION_METHOD) == jsonConfig.end())
			return IndexationMethod::FillMissingIndex;
		std::optional<IndexationMethod> indexation = magic_enum::enum_cast<IndexationMethod>(cleanString(jsonConfig.at(INDEXATION_METHOD).dump()));
		if(!indexation.has_value())
			return IndexationMethod::FillMissingIndex;

		return *indexation;
	}

	bool setIndexationMethod(const IndexationMethod& indexationMethod)
	{
		jsonConfig[INDEXATION_METHOD] = magic_enum::enum_name<IndexationMethod>(indexationMethod);
		return saveConfigFile(filePath);
	}

	PerspectiveZBounds getPerspectiveZBounds()
	{
		// Paramètres de base { 0.125, 8192.0 }
		PerspectiveZBounds zBounds = { -3, 16 };
		if (jsonConfig.find(RENDER_PERSPECTIVE_PLANS) == jsonConfig.end())
			return zBounds;
		nlohmann::json optionsNode = jsonConfig[RENDER_PERSPECTIVE_PLANS];
		if (optionsNode.find(RENDER_NEAR_PLAN) != optionsNode.end())
			zBounds.near_plan_log2 = optionsNode[RENDER_NEAR_PLAN].get<int>();
		if (optionsNode.find(RENDER_NEAR_FAR_RANGE) != optionsNode.end())
			zBounds.near_far_ratio_log2 = optionsNode[RENDER_NEAR_FAR_RANGE].get<int>();

		return zBounds;
	}

	bool setPerspectiveZBounds(const PerspectiveZBounds& zBounds)
	{
		try {
			nlohmann::json optionsNode;
			optionsNode[RENDER_NEAR_PLAN] = zBounds.near_plan_log2;
			optionsNode[RENDER_NEAR_FAR_RANGE] = zBounds.near_far_ratio_log2;
			jsonConfig[RENDER_PERSPECTIVE_PLANS] = optionsNode;
			return saveConfigFile(filePath);
		}
		catch (...)
		{
			return false;
		}
	}

	OrthographicZBounds getOrthographicZBounds()
	{
		if (jsonConfig.find(RENDER_ORTHOGRAPHIC_Z_BOUND) == jsonConfig.end())
			return 12;
		else
			return jsonConfig.at(RENDER_ORTHOGRAPHIC_Z_BOUND).get<int>();
	}

	bool setOrthographicZBounds(const OrthographicZBounds& orthoBounds)
	{
		try {
			jsonConfig[RENDER_ORTHOGRAPHIC_Z_BOUND] = orthoBounds;
			return saveConfigFile(filePath);
		}
		catch (...)
		{
			return false;
		}
	}

	NavigationParameters getNavigationParameters()
	{
		NavigationParameters navParams;
		navParams.cameraRotationExamineFactor = 100.;
		navParams.cameraTranslationSpeedFactor = 100.;
		navParams.examineMinimumRadius = 0.25;
		if (jsonConfig.find(NAVIGATION_PARAMETERS) == jsonConfig.end())
			return navParams;
		nlohmann::json optionsNode = jsonConfig[NAVIGATION_PARAMETERS];
		if (optionsNode.find(NAV_PARAM_EXAMINE_MIN_RADIUS) != optionsNode.end())
			navParams.examineMinimumRadius = optionsNode[NAV_PARAM_EXAMINE_MIN_RADIUS].get<double>();
		if (optionsNode.find(NAV_PARAM_TRANSLATION_SPEED) != optionsNode.end())
			navParams.cameraTranslationSpeedFactor = optionsNode[NAV_PARAM_TRANSLATION_SPEED].get<double>();
		if (optionsNode.find(NAV_PARAM_ROTATION_EXAMINE_SPEED) != optionsNode.end())
			navParams.cameraRotationExamineFactor = optionsNode[NAV_PARAM_ROTATION_EXAMINE_SPEED].get<double>();

		//retro-compatibility
		if (jsonConfig.find(NAV_PARAM_WHEEL_INVERT_JSON_KEY) != jsonConfig.end())
		{
			navParams.wheelInverted = jsonConfig.at(NAV_PARAM_WHEEL_INVERT_JSON_KEY).get<bool>();
			jsonConfig.erase(NAV_PARAM_WHEEL_INVERT_JSON_KEY);
		}
		if (optionsNode.find(NAV_PARAM_WHEEL_INVERT_JSON_KEY) != optionsNode.end())
			navParams.wheelInverted = optionsNode[NAV_PARAM_WHEEL_INVERT_JSON_KEY].get<bool>();

		if (optionsNode.find(NAV_PARAM_MOUSE_INVERT) != optionsNode.end())
			navParams.mouseDragInverted = optionsNode[NAV_PARAM_MOUSE_INVERT].get<bool>();


		
		return navParams;
	}

	bool setNavigationParameters(const NavigationParameters& navParams)
	{
		try {
			nlohmann::json optionsNode;
			optionsNode[NAV_PARAM_EXAMINE_MIN_RADIUS] = navParams.examineMinimumRadius;
			optionsNode[NAV_PARAM_TRANSLATION_SPEED] = navParams.cameraTranslationSpeedFactor;
			optionsNode[NAV_PARAM_ROTATION_EXAMINE_SPEED] = navParams.cameraRotationExamineFactor;
			optionsNode[NAV_PARAM_WHEEL_INVERT_JSON_KEY] = navParams.wheelInverted;
			optionsNode[NAV_PARAM_MOUSE_INVERT] = navParams.mouseDragInverted;
			jsonConfig[NAVIGATION_PARAMETERS] = optionsNode;
			return saveConfigFile(filePath);
		}
		catch (...)
		{
			return false;
		}
	}

	bool isUnlockScanManipulation()
	{
		bool isUnlock = false;
		if (jsonConfig.find(NAV_PARAM_IS_UNLOCK_SCAN_MANIPULATION_JSON_KEY) == jsonConfig.end())
			return isUnlock;
		isUnlock = jsonConfig.at(NAV_PARAM_IS_UNLOCK_SCAN_MANIPULATION_JSON_KEY).get<bool>();
		return isUnlock;
	}

	bool setUnlockScanManipulation(bool unlock)
	{
		try {
			jsonConfig[NAV_PARAM_IS_UNLOCK_SCAN_MANIPULATION_JSON_KEY] = unlock;
			return saveConfigFile(filePath);
		}
		catch (...)
		{
			return false;
		}
	}
	/*
#ifdef LICENSE_MANAGER_BUILD
	uint16_t getLicenseParameters(LicenseParameters& parameters)
	{
		if (isLoaded == false)
			return (0);

		uint16_t activationStep(0);
		if (jsonConfig.find(LICENSE_JSON_KEY) == jsonConfig.end())
			return activationStep;
		for (auto item : jsonConfig.at(LICENSE_JSON_KEY).items())
		{
			if (licenseParametersDics.find(item.key()) != licenseParametersDics.end())
			{
				switch (licenseParametersDics.at(item.key()))
				{
					case LicenseParametersKey::Type:
					{
						auto type = magic_enum::enum_cast<LicenceType>(cleanString(item.value().dump()));
						if (!type.has_value())
						{
							parameters.type = LicenceType::Empty;
							return false;
						}
						parameters.type = *type;
					}
					break;
					case LicenseParametersKey::Server:
						parameters.server = cleanWString(item.value().dump());
						break;
					case LicenseParametersKey::Proxy:
						parameters.proxySettings = cleanWString(item.value().dump());
						break;
					case LicenseParametersKey::LicenceKey:
						parameters.onlineFloatingLicence = Utils::from_utf8(item.value().get<std::string>());
						break;
					case LicenseParametersKey::LicenceKey_Old:
						parameters.onlineFloatingLicence = item.value().get<std::wstring>();
						break;
					case LicenseParametersKey::ActivationStep:
						activationStep = (uint16_t)std::stoi(item.value().dump());
						break;
				}
			}
		}
		return (activationStep);
	}

	bool setLicenseParameters(const LicenseParameters& parameters, const uint16_t& activationStep, const bool& saveFile)
	{
		nlohmann::json licenseConfig;
		std::unordered_map<LicenseParametersKey, std::string> licenseParametersInvDics;
		for (const std::pair<std::string, LicenseParametersKey>& pair : licenseParametersDics)
			licenseParametersInvDics[pair.second] = pair.first;
		licenseConfig[licenseParametersInvDics[LicenseParametersKey::Type]] = magic_enum::enum_name<LicenceType>(parameters.type);
		if(parameters.type == LicenceType::OnlineFloating)
			licenseConfig[licenseParametersInvDics[LicenseParametersKey::LicenceKey]] = Utils::to_utf8(parameters.onlineFloatingLicence);
		//licenseConfig[licenseParametersInvDics[LicenseParametersKey::Proxy]] = parameters.proxySettings;
		switch (parameters.type)
		{
			case LicenceType::Empty:
			case LicenceType::Online:
			case LicenceType::TrialOnline:
				break;
#ifdef FLOATING_LICENCE
			case LicenceType::Floating:
				licenseConfig[licenseParametersInvDics[LicenseParametersKey::Server]] = parameters.server.toString().toStdString();
#endif

		}
		if(activationStep)
			licenseConfig[licenseParametersInvDics[LicenseParametersKey::ActivationStep]] = activationStep;

		jsonConfig[LICENSE_JSON_KEY] = licenseConfig;
		needSave = true;
		if (saveFile)
			return saveConfigFile(filePath);
		return true;
	}
#endif
	*/
}
