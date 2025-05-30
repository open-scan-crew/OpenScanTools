#ifndef CONTROL_PROJECT_H_
#define CONTROL_PROJECT_H_

#include "controller/controls/IControl.h"
#include "models/project/ProjectInfos.h"
#include "gui/LanguageType.h"
#include "io/ConvertProperties.h"
#include "io/imports/ImportTypes.h"

#include <filesystem>

#include "models/graph/CameraNode.h"

namespace control::project
{
    class Create : public AControl
    {
    public:
        Create(const ProjectInfos & infos, const std::filesystem::path& folderPath,
            const std::filesystem::path& templatePath);
        Create(const ProjectInfos& infos, const std::filesystem::path& folderPath, LanguageType languageTemplate);
        ~Create();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        ProjectInfos m_projectInfo;
        const std::filesystem::path m_folderPath;
        const std::filesystem::path m_templatePath;
        const LanguageType m_languageTemplate;
    };

    class DropLoad : public AControl
    {
    public:
        DropLoad(const std::filesystem::path& loadPath);
        ~DropLoad();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        std::filesystem::path m_loadPath;
    };

    class Load : public AControl
    {
    public:
        Load(std::filesystem::path loadPath);
        ~Load();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        std::filesystem::path m_loadPath;
    };

    class StartSave : public AControl
    {
    public:
        StartSave();
        ~StartSave();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    };

    class Save : public AControl
    {
    public:
        Save(const SafePtr<CameraNode>& camera);
        ~Save();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        SafePtr<CameraNode> m_camera;
    };

    class Close : public AControl
    {
    public:
        Close();
        ~Close();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    };

    class SaveCreate : public AControl
    {
    public:
        SaveCreate();
        SaveCreate(std::filesystem::path init_path, std::wstring init_name, std::wstring init_company);
        ~SaveCreate();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        std::filesystem::path init_path_;
        std::wstring init_name_;
        std::wstring init_company_;
    };

    class SaveClose : public AControl
    {
    public:
        SaveClose();
        ~SaveClose();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    };

    class SaveCloseLoad : public AControl
    {
    public:
        SaveCloseLoad(std::filesystem::path path = "");
        ~SaveCloseLoad();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        std::filesystem::path m_projectToLoad;
    };

    class SaveQuit : public AControl
    {
    public:
        SaveQuit();
        ~SaveQuit();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    };

    class Edit : public AControl
    {
    public:
        Edit(const ProjectInfos & infos);
        ~Edit();

        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        ProjectInfos newInfos;
        ProjectInfos oldInfos;
    };

    class FunctionImportScan : public AControl
    {
    public:
        FunctionImportScan();
        ~FunctionImportScan();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    };

    class ImportScan: public AControl
    {
    public:
        ImportScan(const Import::ScanInfo& importInfo);
        ~ImportScan();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;

    private:
        Import::ScanInfo m_importInfo;
    };

    class ConvertScan: public AControl
    {
    public:
        ConvertScan(const ConvertProperties& prop);
        ~ConvertScan();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        ConvertScan() {};
    private:
        ConvertProperties m_prop;
    };

    class ShowProperties : public AControl
    {
    public:
        ShowProperties();
        ~ShowProperties();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    };
}

#endif // !CONTROLPROJECT_H_