#ifndef CONTROL_PROJECT_H_
#define CONTROL_PROJECT_H_

#include "controller/controls/IControl.h"
#include "models/project/ProjectInfos.h"
#include "models/project/ProjectTypes.h"
#include "io/ConvertProperties.h"
#include "io/imports/ImportTypes.h"

#include <filesystem>

#include "models/graph/CameraNode.h"

class TransformationModule;

struct ProjectTemplate;

namespace control
{
	namespace project
	{
		class Create : public AControl
		{
		public:
            //Create();
			Create(const ProjectInfos & infos, const std::filesystem::path& folderPath,
				const std::filesystem::path& templatePath);
			Create(const ProjectInfos& infos, const std::filesystem::path& folderPath, ProjectTemplate projectTemplate);
			~Create();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
            ProjectInfos m_projectInfo;
            const std::filesystem::path m_folderPath;
			const std::filesystem::path m_templatePath;
			const ProjectTemplate m_projectTemplate;
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

		/*class Autosave : public AControl
		{
		public:
			Autosave(SafePtr<CameraNode> camera);
			~Autosave();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			SafePtr<CameraNode> m_camera;
		};*/

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
            ~SaveCreate();
            void doFunction(Controller& controller) override;
            bool canUndo() const override;
            void undoFunction(Controller& controller) override;
            ControlType getType() const override;
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

		//new
		class SaveCloseLoadCentral : public AControl
		{
		public:
			SaveCloseLoadCentral(std::filesystem::path path = "");
			~SaveCloseLoadCentral();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			std::filesystem::path m_projectToLoad;
			bool m_isCentral;
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

		class ApplyProjectTransformation : public AControl
		{
		public:
			ApplyProjectTransformation();
			~ApplyProjectTransformation();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
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
}

#endif // !CONTROLPROJECT_H_