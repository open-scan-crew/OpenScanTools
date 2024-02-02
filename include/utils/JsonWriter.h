#include <filesystem>
#include "nlohmannJson/json.hpp"

namespace utils
{
    bool writeJsonFile(const std::filesystem::path& exportPath, const nlohmann::json& json);
}
