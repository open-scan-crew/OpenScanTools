#ifndef CONFIG_H
#define CONFIG_H

#include "pointCloudEngine/RenderingTypes.h"
#include "gui/LanguageType.h"

#include <unordered_set>
#include <vector>
#include <filesystem>

enum LoggerMode;
class Color32;
enum class IndexationMethod;
struct NavigationParameters;
struct UnitUsage;

namespace Config
{
	static const std::unordered_set<uint8_t> Timing = {5, 10, 15, 30, 60};

	void loadConfigFile(const std::filesystem::path& filepath);
	bool saveConfigFile(const std::filesystem::path& filepath);

	bool findKey(std::string key);

    std::vector<std::pair<LoggerMode, bool>> getLogMods();
	bool isFileLoaded();

	LanguageType getLanguage();
	bool setLanguage(LanguageType type);

	bool getCenteringConfiguration();
	bool setCenteringConfiguration(bool value);

	bool getKeepingExamineConfiguration();
	bool setKeepingExamineConfiguration(bool value);

	UnitUsage getUnitUsageConfiguration();
	bool setUnitUsageConfiguration(const UnitUsage& type);
		
	Color32 getUserColor();
	bool setUserColor(const Color32& type);

    std::filesystem::path getApplicationDirPath();
    void setApplicationDirPath(const std::filesystem::path& path);

	std::filesystem::path getProjectsPath();
	bool setProjectsPath(const std::filesystem::path& type);

	std::filesystem::path getTemporaryPath();
	bool setTemporaryPath(const std::filesystem::path& type);

	std::filesystem::path getFFmpegPath();
	bool setFFmpegPath(const std::filesystem::path& type);

    std::filesystem::path getResourcesPath();

    DecimationOptions getDecimationOptions();
    bool setDecimationOptions(const DecimationOptions& options);
	OctreePrecision getOctreePrecision();
	bool setOctreePrecision(const OctreePrecision precision);

    bool getMaximizedFrameless();
    bool setMaximizedFrameless(bool value);

	bool getExamineDisplayMode();
	bool setExamineDisplayMode(bool value);

	std::vector<std::pair<std::filesystem::path, time_t>> getRecentProjects();
	bool setRecentProjects(std::vector<std::pair<std::filesystem::path, time_t>> projects);

	bool getIsAutoSaveActive();
	bool setIsAutoSaveActive(const bool& autosave);

	uint8_t getAutoSaveTiming();
	bool setAutoSaveTiming(const uint8_t& timing);

	float* getGizmoParameters();
	bool setGizmoParameters(const float gizmo[3]);

	float getManipulatorSize();
	bool setManipulatorSize(float manipulatorSize);

	std::string getGraphicDevice();
	bool setGraphicDevice(const std::string name);

	IndexationMethod getIndexationMethod();
	bool setIndexationMethod(const IndexationMethod& indexationMethod);

	PerspectiveZBounds getPerspectiveZBounds();
	bool setPerspectiveZBounds(const PerspectiveZBounds& persBounds);

	OrthographicZBounds getOrthographicZBounds();
	bool setOrthographicZBounds(const OrthographicZBounds& orthoBounds);

	NavigationParameters getNavigationParameters();
	bool setNavigationParameters(const NavigationParameters& navParams);

	bool isUnlockScanManipulation();
	bool setUnlockScanManipulation(bool unlock);
}

#endif
