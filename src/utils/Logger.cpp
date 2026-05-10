#include "utils/Logger.h"
#include "utils/Config.h"
#include "utils/System.h"
#include "utils/FilesAndFoldersDefinitions.h"
#include "utils/OpenScanToolsVersion.h"
#include <magic_enum/magic_enum.hpp>

#include <chrono>
#include <thread>
#include <windows.h>
#include <mutex>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <system_error>

#ifdef WIN32
#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif
#else
#include <sys/time.h>
#endif

#ifdef _DEBUG_
#define TL_BUILD "Debug  "
#else
#define TL_BUILD "Release"
#endif

#define HEADER(build, vers, date) "\
*******************************************************************************\n\
** OpenScanTools - OST                                                       **\n\
** "##build" v"##vers"  "##date"                                                **\n\
** (C)OpenScanTools 2024                                                     **\n\
*******************************************************************************\n"


std::mutex Logger::loggerMutex;
std::ofstream Logger::out;
bool Logger::modeEnabled[LoggerMode::LOGGER_MODE_MAX_ENUM];

thread_local SubLogger Logger::trueLogger(true);
SubLogger Logger::deadEndLogger(false);

const EndLog Logger::endl;

SubLogger::SubLogger(bool active)
    : isActive(active)
    , modeName(LoggerMode::LogConfig)
    , buffer()
    , outStream(&buffer)
{
}

std::filesystem::path Logger::getOpenScanToolsPath()
{
    std::filesystem::path path = Utils::System::getDocumentPath();
    path = Utils::System::getAndCreateDirectory(path, Folder_OpenScanTools);

    return (path);
}

void Logger::init()
{
    // Initialize every mode as disabled before loading configuration.
    for (uint32_t iterator(0); iterator < LoggerMode::LOGGER_MODE_MAX_ENUM; iterator++)
        Logger::setStatusToMode((LoggerMode)iterator, false);
    Logger::setStatusToMode(LoggerMode::LogConfig, true);
    Logger::logInFile();

    std::filesystem::path path = Utils::System::getOSTProgramDataPath();
    path += "\\init.json";
    Config::loadConfigFile(path);

    if (!Config::isFileLoaded())
    {
        Logger::log(LoggerMode::LogConfig, "Failed to load init.json");
        return;
    }
    std::vector<std::pair<LoggerMode, bool>> logMods = Config::getLogMods();

    for (auto mods : logMods)
    {
        Logger::setStatusToMode(mods.first, mods.second);
    }

    Logger::log() << "\n" << HEADER(TL_BUILD, OPENSCANTOOLS_VERSION, BUILD_DATE) << LOGENDL;
}

void Logger::log(const LoggerMode& mode, std::string message)
{
    log(mode) << message << Logger::endl;
}

void Logger::log(const LoggerMode& mode, std::stringstream& message)
{
    log(mode, message.str());
}

void Logger::logInFile()
{
    std::lock_guard<std::mutex> lock(loggerMutex);
    std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::string str = std::ctime(&time);
    for (int i = 0; i < str.size(); i++)
    {
        if (str[i] == ' ' || str[i] == '\r' || str[i] == '\n' || str[i] == ':')
            str[i] = '_';
    }

    std::filesystem::path OSTPath = getOpenScanToolsPath();
    std::string path;
    OSTPath = Utils::System::getAndCreateDirectory(OSTPath, "log");
    path = OSTPath.string() + "\\log_" + str + ".log";

    out.open(path);

    // Keep log directory size under control:
    // - only managed .log files are considered,
    // - current session log is always protected.
    cleanupOldLogs(OSTPath, std::filesystem::path(path));
}

void Logger::stopLog()
{
    std::lock_guard<std::mutex> lock(loggerMutex);
    if (out.is_open() == true)
        out.close();
}

void Logger::setStatusToMode(const LoggerMode& mode, bool state)
{
    {
        std::lock_guard<std::mutex> lock(loggerMutex);
        modeEnabled[(size_t)mode] = state;
    }
    std::string modestr(magic_enum::enum_name(mode));
    std::string message = state ? modestr + " Enabled" : modestr + " Disabled";
    log(LoggerMode::LogConfig) << message << Logger::endl;
}

SubLogger& Logger::log()
{
    return (trueLogger);
}

SubLogger& Logger::log(const LoggerMode& mode)
{
    std::lock_guard<std::mutex> lock(loggerMutex);
    if (modeEnabled[mode])
    {
        trueLogger.modeName = mode;
        trueLogger.buffer.str("");
        return (trueLogger);
    }
    else
    {
        return (deadEndLogger);
    }
}

void Logger::flushLog(LoggerMode mode, const std::stringbuf& log)
{
    std::lock_guard<std::mutex> lock(loggerMutex);
    char timeBuffer[20], usec[10];
    long tv_usec;
    std::time_t currentTime(std::time(nullptr));
    std::strftime(timeBuffer, sizeof(timeBuffer), "%H:%M:%S.", std::localtime(&currentTime));
#ifdef WIN32
    FILETIME ft;
    unsigned __int64 tmpres = 0;
    GetSystemTimeAsFileTime(&ft);
    tmpres |= ft.dwHighDateTime;
    tmpres <<= 32;
    tmpres |= ft.dwLowDateTime;
    //converting file time to unix epoch
    tmpres -= DELTA_EPOCH_IN_MICROSECS;
    tmpres /= 10;  //convert into microseconds
    tv_usec = (long)(tmpres % 1000000UL);
#else
    struct timeval;
    gettimeofday(&timeval, NULL);
    tv_usec = timeval.tv_usec;
#endif
    sprintf_s(usec, 10, "%06u", tv_usec);
    strncat_s(timeBuffer, usec, 3);
#ifdef _DEBUG
    std::cout << timeBuffer << " ; T[" << std::this_thread::get_id() << "] ; " << magic_enum::enum_name(mode) << " ; " << log.str();
#endif // _DEBUG
    out << timeBuffer << " ; T[" << std::this_thread::get_id() << "] ; " << magic_enum::enum_name(mode) << " ; " << log.str();
    out.flush();
}

void Logger::cleanupOldLogs(const std::filesystem::path& logDirectory, const std::filesystem::path& currentLogFile)
{
    if (maxLogFilesToKeep == 0 || !std::filesystem::exists(logDirectory))
        return;

    std::error_code ec;
    std::vector<std::pair<std::filesystem::path, std::filesystem::file_time_type>> managedLogs;

    for (const auto& entry : std::filesystem::directory_iterator(logDirectory, ec))
    {
        if (ec)
            return;

        if (!entry.is_regular_file(ec) || ec)
            continue;

        const std::filesystem::path filePath = entry.path();
        const std::string filename = filePath.filename().string();

        // Ignore files that are not OpenScanTools managed logs.
        if (filePath.extension() != ".log" || filename.rfind("log_", 0) != 0)
            continue;

        // Never delete current session log file.
        if (std::filesystem::equivalent(filePath, currentLogFile, ec))
        {
            ec.clear();
            continue;
        }

        const auto lastWrite = std::filesystem::last_write_time(filePath, ec);
        if (ec)
        {
            ec.clear();
            continue;
        }

        managedLogs.emplace_back(filePath, lastWrite);
    }

    if (managedLogs.empty())
        return;

    std::sort(managedLogs.begin(), managedLogs.end(), [](const auto& lhs, const auto& rhs)
    {
        return lhs.second > rhs.second;
    });

    const std::size_t maxPreviousLogsToKeep = (maxLogFilesToKeep > 0) ? (maxLogFilesToKeep - 1) : 0;
    if (managedLogs.size() <= maxPreviousLogsToKeep)
        return;

    for (std::size_t idx = maxPreviousLogsToKeep; idx < managedLogs.size(); ++idx)
    {
        std::filesystem::remove(managedLogs[idx].first, ec);
        ec.clear(); // best effort cleanup: never fail application startup.
    }
}
