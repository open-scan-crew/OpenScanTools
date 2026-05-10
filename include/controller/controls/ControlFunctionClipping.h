#ifndef CONTROL_FUNCTION_CLIPPING_H_
#define CONTROL_FUNCTION_CLIPPING_H_

#include "controller/controls/IControl.h"
#include "models/OpenScanToolsModelEssentials.h"
#include "models/data/Box/ClippingBoxSettings.h"
#include "utils/Color32.hpp"
#include "glm/glm.hpp"
//#include "utils/tree/TreeElement.h"

class Grid;
class Box;


namespace control::function::clipping
{
	class ActivateCreateLocal : public AControl
	{
	public:
		ActivateCreateLocal();
		~ActivateCreateLocal();
		void doFunction(Controller& controller) override;
		bool canUndo() const override;
		void undoFunction(Controller& controller) override;
		ControlType getType() const override;
	};

	class ActivateCreateAttached : public AControl
	{
	public:
		ActivateCreateAttached();
		~ActivateCreateAttached();
		void doFunction(Controller& controller) override;
		bool canUndo() const override;
		void undoFunction(Controller& controller) override;
		ControlType getType() const override;
	};

	class ActivateCreateAttached2Points : public AControl
	{
	public:
		ActivateCreateAttached2Points();
		~ActivateCreateAttached2Points();
		void doFunction(Controller& controller) override;
		bool canUndo() const override;
		void undoFunction(Controller& controller) override;
		ControlType getType() const override;
	};

	class CreateGlobal : public AControl
	{
	public:
		CreateGlobal();
		~CreateGlobal();
		void doFunction(Controller& controller) override;
		ControlType getType() const override;
	};

	class SetAlignementValue : public AControl
	{
	public:
		SetAlignementValue(const double& angleZ);
		~SetAlignementValue();
		void doFunction(Controller& controller) override;
		ControlType getType() const override;
	private:
		const double m_angleZ;
	};

	class SetDefaultSize : public AControl
	{
	public:
		SetDefaultSize() = delete;
		SetDefaultSize(glm::vec3 size);
		~SetDefaultSize();
		void doFunction(Controller& controller) override;
		ControlType getType() const override;
	private:
		glm::vec3 m_size;
	};

	class SetDefaultOffset : public AControl
	{
	public:
		SetDefaultOffset(ClippingBoxOffset offset);
		~SetDefaultOffset();
		void doFunction(Controller& controller) override;
		ControlType getType() const override;
	private:
		ClippingBoxOffset m_offset;
	};

	class ActivateDuplicate : public AControl
	{
	public:
		ActivateDuplicate();
		~ActivateDuplicate();
		void doFunction(Controller& controller) override;
		ControlType getType() const override;
	};

	class ActivatePolygonalSelector : public AControl
	{
	public:
		ActivatePolygonalSelector();
		~ActivatePolygonalSelector();
		void doFunction(Controller& controller) override;
		ControlType getType() const override;
	};

}

#endif // !CONTROL_FUNCTION_TAG_H_
