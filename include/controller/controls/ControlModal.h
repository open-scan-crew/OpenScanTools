#ifndef CONTROL_MODAL_H
#define CONTROL_MODAL_H

#include "controller/controls/IControl.h"

#include <vector>
#include <filesystem>

// control::modal::

namespace control
{
	namespace modal
	{
		class ModalReturnValue : public AControl
		{
		public:
			ModalReturnValue(const uint32_t& value);
			~ModalReturnValue();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			ModalReturnValue() {};
		private:
				uint32_t m_returnedValue;
		};

		class ModalReturnFiles : public AControl
		{
		public:
			ModalReturnFiles(const std::vector<std::filesystem::path>& files);
			~ModalReturnFiles();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			ModalReturnFiles() {};
		private:
			std::vector<std::filesystem::path>  m_files;
		};
	}
}

#endif