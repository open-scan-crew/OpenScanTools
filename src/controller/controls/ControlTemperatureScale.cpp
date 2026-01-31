#include "controller/controls/ControlTemperatureScale.h"

#include "controller/Controller.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "models/project/ProjectInfos.h"
#include "utils/TemperatureScaleUtils.h"
#include "utils/System.h"

namespace
{
    const QString kTemperatureScaleInvalidMessage = QStringLiteral(
        "The file structure is incorrect. It must be in the form of columns R G B T.\n"
        "RGB are integers (0-255) and the temperature T is in \xC2\xB0C, with decimals accepted (dot as a separator).\n"
        "Separator between columns: tab, space, or ; .\n"
        "File encoding must be either UTF-8 or ANSI");
    const QString kTemperatureScaleCopyErrorMessage = QStringLiteral("Unable to copy the temperature scale file to Templates.");

    bool isInTemplatesFolder(const std::filesystem::path& filePath, const std::filesystem::path& templatesPath)
    {
        std::error_code error;
        return std::filesystem::equivalent(filePath.parent_path(), templatesPath, error);
    }
}

namespace control::temperatureScale
{
    ImportTemperatureScale::ImportTemperatureScale(const std::filesystem::path& path)
        : m_filePath(path)
    {
    }

    ImportTemperatureScale::~ImportTemperatureScale() = default;

    void ImportTemperatureScale::doFunction(Controller& controller)
    {
        if (m_filePath.empty())
            return;

        ControllerContext& context = controller.getContext();
        if (!context.isProjectLoaded())
            return;

        std::string errorMessage;
        TemperatureScaleData parsed = TemperatureScaleUtils::loadTemperatureScaleFile(m_filePath, errorMessage);
        if (!parsed.isValid)
        {
            controller.updateInfo(new GuiDataWarning(kTemperatureScaleInvalidMessage));
            return;
        }

        const std::filesystem::path templatesPath = context.cgetProjectInternalInfo().getTemplatesFolderPath();
        Utils::System::createDirectoryIfNotExist(templatesPath);

        std::filesystem::path finalPath = m_filePath;
        if (!isInTemplatesFolder(m_filePath, templatesPath))
        {
            std::filesystem::path targetPath = templatesPath / m_filePath.filename();
            std::error_code error;
            std::filesystem::copy_file(m_filePath, targetPath, std::filesystem::copy_options::overwrite_existing, error);
            if (error)
            {
                controller.updateInfo(new GuiDataWarning(kTemperatureScaleCopyErrorMessage));
                return;
            }
            finalPath = targetPath;
        }

        parsed.filePath = finalPath;
        controller.getGraphManager().setTemperatureScaleData(parsed);
        context.getProjectInfo().m_temperatureScaleFilePath = finalPath;
        context.setIsCurrentProjectSaved(false);

        controller.updateInfo(new GuiDataTemperatureScaleInfo(finalPath, true, true));
    }

    bool ImportTemperatureScale::canUndo() const
    {
        return false;
    }

    void ImportTemperatureScale::undoFunction(Controller& controller)
    {
    }

    ControlType ImportTemperatureScale::getType() const
    {
        return ControlType::importTemperatureScale;
    }
}
