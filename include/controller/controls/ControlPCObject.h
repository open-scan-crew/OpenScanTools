#ifndef CONTROL_PCOBJECT_H_
#define CONTROL_PCOBJECT_H_

#include "controller/controls/IControl.h"
#include <filesystem>
#include <list>

class PCObject;

namespace control::pcObject
{
	class CreatePCObjectFromBoxActivate : public AControl
	{
	public:
		CreatePCObjectFromBoxActivate();
		~CreatePCObjectFromBoxActivate();
		void doFunction(Controller& controller) override;
		ControlType getType() const override;
	};

	class CreatePCObjectFromFile : public AControl
	{
	public:
		CreatePCObjectFromFile(const std::list<std::filesystem::path>& pcFiles);
		~CreatePCObjectFromFile();
		void doFunction(Controller& controller) override;
		ControlType getType() const override;
	private:
		const std::list<std::filesystem::path> m_files;
	};

	class ActivateDuplicate : public AControl
	{
	public:
		ActivateDuplicate();
		~ActivateDuplicate();
		void doFunction(Controller& controller) override;
		bool canUndo() const override;
		void undoFunction(Controller& controller) override;
		ControlType getType() const override;
	};
}

#endif // !CONTROL_PCOBJECT_H_
