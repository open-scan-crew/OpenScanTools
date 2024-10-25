#ifndef CONTROL_PROJECT_TEMPLATE_H_
#define CONTROL_PROJECT_TEMPLATE_H_

#include "controller/controls/IControl.h"

#include <string>

namespace control
{
	namespace projectTemplate
	{
		class CreateTemplate : public AControl
		{
		public:
			CreateTemplate(const std::wstring& templateName
				, bool updateTemplate
				/*, const std::filesystem::path& templateOriginPath = ""*/);
			~CreateTemplate();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			const std::wstring m_newTemplateName;
			bool m_updateTemplate;
		};

		class SaveTemplate : public AControl
		{
		public:
			SaveTemplate(const std::wstring &templateName);
			~SaveTemplate();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			const std::wstring m_templateName;
		};
		
		class RenameTemplate : public AControl
		{
		public:
			RenameTemplate(const std::wstring& oldTemplateName, const std::wstring& newTemplateName);
			~RenameTemplate();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			const std::wstring m_oldTemplateName;
			const std::wstring m_newTemplateName;
		};

		class DeleteTemplate : public AControl
		{
		public:
			DeleteTemplate(const std::wstring& templateName);
			~DeleteTemplate();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			const std::wstring m_templateName;
		};

		class SendList : public AControl
		{
		public:
			SendList();
			~SendList();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		};

		class CloseTemplateEdition : public AControl
		{
		public:
			CloseTemplateEdition();
			~CloseTemplateEdition();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		};
	}
}

#endif // !CONTROL_PROJECT_TEMPLATE_H_