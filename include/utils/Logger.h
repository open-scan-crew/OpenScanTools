#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ostream>
#include <mutex>
#include <glm/glm.hpp>

#include "utils/Utils.h"

#define LOGENDL Logger::endl

enum LoggerMode
{
    LogConfig,
    TreeLog,
    IOLog,
    VKLog,
    SQLog,
    ControlLog,
    DataLog,
    ControllerLog,
    HistoricLog,
    GuiLog,
    LicenseLog,
    GTLog,
    GTExtraLog,
    FunctionLog,
    rayTracingLog,
    SceneGraphLog,
    TranslatorLog,
    LOGGER_MODE_MAX_ENUM
};

class SubLogger;

class EndLog
{
    friend std::ostream& operator<<(std::ostream& os, const EndLog& endl)
    {
        return os;
    }
};

class Logger
{
public:
    static std::filesystem::path getOpenScanToolsPath();

    static void init(std::filesystem::path path);
    static void setStatusToMode(const LoggerMode& mode, bool state);
    static void log(const LoggerMode& mode, std::string message);
    static void log(const LoggerMode& mode, std::stringstream& message);

    // Must absolutelly be called before the others
    static void logInFile();
    static void stopLog();
    static SubLogger& log();
    static SubLogger& log(const LoggerMode& mode);

public:
    static const EndLog endl;

private:
    friend class SubLogger;
    static void flushLog(LoggerMode mode, const std::stringbuf& log);

private:
    static std::mutex loggerMutex;
    static bool modeEnabled[LoggerMode::LOGGER_MODE_MAX_ENUM];
    static std::ofstream out;

    static thread_local SubLogger trueLogger;
    static SubLogger deadEndLogger;
};


class SubLogger
{
public:
    SubLogger(bool active);

private:
    const bool isActive;

    friend class Logger;
    LoggerMode	modeName;

    std::stringbuf buffer;
    std::ostream outStream;

public:

    template<typename T>
    SubLogger& operator<<(T &&value)
    {
        if (isActive == true)
            outStream << value;
        return (*this);
    }

    SubLogger& operator<<(std::wstring value)
    {
        if (isActive == true)
            outStream << Utils::to_utf8(value);
        return (*this);
    }

    SubLogger& operator<<(const glm::dvec3& value)
    {
        if (isActive)
            outStream << "x: " << value.x << ",y: " << value.y << ",z: " << value.z;

        return (*this);
    }

    SubLogger& operator<<(glm::dmat4 value)
    {
        if (isActive)
            outStream << "[ " << value[0].x << "," << value[0].y << "," << value[0].z << "," << value[0].w << ",\n " 
            << value[1].x << "," << value[1].y << "," << value[1].z << "," << value[1].w << ",\n "
            << value[2].x << "," << value[2].y << "," << value[2].z << "," << value[2].w << ",\n " 
            << value[3].x << "," << value[3].y << "," << value[3].z << "," << value[3].w << ",\n ";

        return (*this);
    }

    SubLogger& operator<<(const EndLog&)
    {
        if (isActive)
        {
            outStream << "\n";
            Logger::flushLog(modeName, buffer);
            // Reset local streambuffer
            buffer.str("");
        }
        return (*this);
    }

};

#endif
