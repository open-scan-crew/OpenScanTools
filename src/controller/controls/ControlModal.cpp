#include "controller/controls/ControlModal.h"
#include "controller/messages/ModalMessage.h"
#include "controller/messages/FilesMessage.h"
#include "controller/Controller.h"
#include "controller/functionSystem/FunctionManager.h"

namespace control
{
	namespace modal
	{
		ModalReturnValue::ModalReturnValue(const uint32_t& value)
			: m_returnedValue(value)
		{}

		ModalReturnValue::~ModalReturnValue()
		{}

		void ModalReturnValue::doFunction(Controller& controller)
		{
			ModalMessage message(m_returnedValue);
			controller.getFunctionManager().feedMessage(controller, &message);
		}

		bool ModalReturnValue::canUndo() const
		{
			return false;
		}
		void ModalReturnValue::undoFunction(Controller& controller)
		{}

		ControlType ModalReturnValue::getType() const
		{
			return ControlType::ModalReturnValue;
		}


		ModalReturnFiles::ModalReturnFiles(const std::vector<std::filesystem::path>& files)
			: m_files(files)
		{}

		ModalReturnFiles::~ModalReturnFiles()
		{}

		void ModalReturnFiles::doFunction(Controller& controller)
		{
			FilesMessage message(m_files);
			controller.getFunctionManager().feedMessage(controller, &message);
		}

		bool ModalReturnFiles::canUndo() const
		{
			return false;
		}

		void  ModalReturnFiles::undoFunction(Controller& controller)
		{}

		ControlType ModalReturnFiles::getType() const
		{
			return ControlType::ModalReturnFiles;
		}
		
	};
};