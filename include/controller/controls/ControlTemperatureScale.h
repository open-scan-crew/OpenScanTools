#ifndef CONTROL_TEMPERATURE_SCALE_H
#define CONTROL_TEMPERATURE_SCALE_H

#include "controller/controls/IControl.h"

#include <filesystem>

namespace control::temperatureScale
{
    class ImportTemperatureScale : public AControl
    {
    public:
        explicit ImportTemperatureScale(const std::filesystem::path& path);
        ~ImportTemperatureScale() override;

        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;

    private:
        std::filesystem::path m_filePath;
    };
}

#endif // CONTROL_TEMPERATURE_SCALE_H
