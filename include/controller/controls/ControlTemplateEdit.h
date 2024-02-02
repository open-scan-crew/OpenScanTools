#ifndef CONTROL_TEMPLATEEDIT_H_
#define CONTROL_TEMPLATEEDIT_H_

#include <filesystem>

#include "controller/controls/AEditionControl.h"
#include "models/application/TagTemplate.h"

class Tag;

namespace control
{
	namespace tagTemplate
	{
		class SendTemplateList : public AControl
		{
		public:
			SendTemplateList();
			~SendTemplateList();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		};

		class SendTagTemplate : public AControl
		{
		public:
			SendTagTemplate(SafePtr<sma::TagTemplate> temp);
			~SendTagTemplate();
			void doFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			SafePtr<sma::TagTemplate> m_temp;
		};

		class CreateTagTemplate : public AControl
		{
		public:
			CreateTagTemplate(std::wstring tagTemplate);
			~CreateTagTemplate();
			void doFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			std::wstring _name;
		};

		class SaveTemplates : public AControl
		{
		public:
			SaveTemplates();
			~SaveTemplates();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		};



		class DeleteTagTemplate : public AControl
		{
		public:
			DeleteTagTemplate(SafePtr<sma::TagTemplate> tagTemp);
			~DeleteTagTemplate();
			void doFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			SafePtr<sma::TagTemplate> m_toDeleteTemplate;
		};

		class DuplicateTagTemplate : public AControl
		{
		public:
			DuplicateTagTemplate(SafePtr<sma::TagTemplate> tagTemp);
			~DuplicateTagTemplate();
			void doFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			SafePtr<sma::TagTemplate> m_toDuplicateTemplate;
		};

		class RenameTagTemplate : public AControl
		{
		public:
			RenameTagTemplate(SafePtr<sma::TagTemplate> tagTemp, std::wstring newName);
			~RenameTagTemplate();
			void doFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			SafePtr<sma::TagTemplate> m_toChangeTemplate;
			std::wstring m_newName;
		};

		class TemplateCreateField : public AEditionControl
		{
		public:
			TemplateCreateField(SafePtr<sma::TagTemplate> tagTemp, std::wstring name);
			~TemplateCreateField();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			SafePtr<sma::TagTemplate> m_toChangeTemplate;
			std::wstring m_name;
		};
	
		class TemplateDeleteField : public AEditionControl
		{
		public:
			TemplateDeleteField(SafePtr<sma::TagTemplate> tagTemp, sma::tFieldId fieldId);
			~TemplateDeleteField();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			SafePtr<sma::TagTemplate> m_toChangeTemplate;
			sma::tFieldId m_fieldId;
		};
	
		class TemplateRenameField : public AEditionControl
		{
		public:
			TemplateRenameField(SafePtr<sma::TagTemplate> tagTemp, sma::tFieldId fieldId, std::wstring newName);
			~TemplateRenameField();
			void doFunction(Controller& controler) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			SafePtr<sma::TagTemplate> m_toChangeTemplate;
			sma::tFieldId m_fieldId;
			std::wstring m_newName;
		};
	
		class TemplateChangeTypeField : public AEditionControl
		{
		public:
			TemplateChangeTypeField(SafePtr<sma::TagTemplate> tagTemp, sma::tFieldId fieldId, sma::tFieldType newType);
			~TemplateChangeTypeField();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			SafePtr<sma::TagTemplate> m_toChangeTemplate;
			sma::tFieldId m_fieldId;
			sma::tFieldType m_newType;
		};
	
		class TemplateChangeRefField : public AEditionControl
		{
		public:
			TemplateChangeRefField(SafePtr<sma::TagTemplate> tagTemp, sma::tFieldId fieldId, SafePtr<UserList> newRef);
			~TemplateChangeRefField();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			SafePtr<sma::TagTemplate> m_toChangeTemplate;
			sma::tFieldId m_fieldId;
			SafePtr<UserList> m_newRef;
		};
	
		class TemplateChangeDefaultValue : public AControl
		{
		public:
			TemplateChangeDefaultValue(SafePtr<sma::TagTemplate> tagTemp, sma::tFieldId fieldId, std::wstring newValue);
			~TemplateChangeDefaultValue();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			SafePtr<sma::TagTemplate> m_toChangeTemplate;
			sma::tFieldId m_fieldId;
			std::wstring m_defaultValue;
		};
	}
}

#endif // !CONTROL_TEMPLATEEDIT_H_
