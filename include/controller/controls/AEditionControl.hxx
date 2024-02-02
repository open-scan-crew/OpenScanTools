#ifndef A_EDITION_CONTROL_HXX
#define A_EDITION_CONTROL_HXX

#include "controller/controls/AEditionControl.h"
#include "controller/Controller.h"
#include "utils/Logger.h"

#include "gui/GuiData/GuiDataMessages.h"

#include "models/3d/Graph/AGraphNode.h"

// ATEditionControl

template<class ObjectClass, class AttrClass>
inline ATEditionControl<ObjectClass, AttrClass>::ATEditionControl(const std::unordered_map<SafePtr<ObjectClass>, AttrClass>& toEditDatas, const std::string& controlName, std::function<void(ObjectClass&, const AttrClass&)> setterAttr, std::function<AttrClass(const ObjectClass&)> getterAttr)
	: AEditionControl()
	, m_toEditDatas(toEditDatas)
	, m_controlName(controlName)
	, m_setterAttr(setterAttr)
	, m_getterAttr(getterAttr)
	, m_actualizeOptions(false)
{}

template<class ObjectClass, class AttrClass>
ATEditionControl<ObjectClass, AttrClass>::ATEditionControl(const std::unordered_set<SafePtr<ObjectClass>>& toEditDatas, const AttrClass& newValue, const std::string& controlName,
	std::function<void(ObjectClass&, const AttrClass&)> setterAttr,
	std::function<AttrClass (const ObjectClass&)> getterAttr)

	: AEditionControl()
	, m_controlName(controlName)
	, m_setterAttr(setterAttr)
	, m_getterAttr(getterAttr)
	, m_actualizeOptions(false)
{
	for (const SafePtr<ObjectClass>& data : toEditDatas)
		m_toEditDatas.insert({ data, newValue });
}

template<class ObjectClass, class AttrClass>
inline ATEditionControl<ObjectClass, AttrClass>::ATEditionControl(const std::string& controlName, std::function<void(ObjectClass&, const AttrClass&)> setterAttr, std::function<AttrClass(const ObjectClass&)> getterAttr)
	: AEditionControl()
	, m_controlName(controlName)
	, m_setterAttr(setterAttr)
	, m_getterAttr(getterAttr)
	, m_actualizeOptions(false)
{}

template<class ObjectClass, class AttrClass>
ATEditionControl<ObjectClass, AttrClass>::~ATEditionControl()
{
}

template<class ObjectClass, class AttrClass>
void ATEditionControl<ObjectClass, AttrClass>::doFunction(Controller& controller)
{
	doSimpleEdition(controller);
	CONTROLLOG << "control::" << m_controlName << " do" << Logger::endl;
}

template<class ObjectClass, class AttrClass>
bool ATEditionControl<ObjectClass, AttrClass>::canUndo() const
{
	return !m_oldValues.empty();
}

template<class ObjectClass, class AttrClass>
void ATEditionControl<ObjectClass, AttrClass>::undoFunction(Controller& controller)
{
	undoSimpleEdition(controller);
	CONTROLLOG << "control::" << m_controlName << " undo" << Logger::endl;
}

template<class ObjectClass, class AttrClass>
void ATEditionControl<ObjectClass, AttrClass>::redoFunction(Controller& controller)
{
	doSimpleEdition(controller);
	CONTROLLOG << "control::" << m_controlName << " redo" << Logger::endl;
}

template<class ObjectClass, class AttrClass>
void ATEditionControl<ObjectClass, AttrClass>::doSimpleEdition(Controller& controller)
{
	m_oldValues.clear();
	std::unordered_set<SafePtr<AGraphNode>> actualizeNodes;

	bool cannotFinishEdit = false;

	for (const auto& toEditDataPair : m_toEditDatas)
	{
		WritePtr<ObjectClass> doWriteData = toEditDataPair.first.get();
		if (!doWriteData)
		{
			CONTROLLOG << "control::" << m_controlName << " do : data null" << Logger::endl;
			continue;
		}

		const AttrClass& newValue = toEditDataPair.second;
		if (m_getterAttr(*&doWriteData) == newValue)
		{
			CONTROLLOG << "control::" << m_controlName << " do : same value" << Logger::endl;
			continue;
		}

		if (m_editCondition(*&doWriteData, newValue))
		{
			actualizeNodes.insert(toEditDataPair.first);
			m_oldValues[toEditDataPair.first] = m_getterAttr(*&doWriteData);
			m_setterAttr(*&doWriteData, newValue);
			doTimeModified(*&doWriteData);
		}
		else
			cannotFinishEdit = true;
	}

	if (cannotFinishEdit && !m_cannotEditWarningMessage.isEmpty())
		controller.updateInfo(new GuiDataWarning(m_cannotEditWarningMessage));

	controller.actualizeNodes(m_actualizeOptions, actualizeNodes);
}


template<class ObjectClass, class AttrClass>
void ATEditionControl<ObjectClass, AttrClass>::undoSimpleEdition(Controller& controller)
{
	std::unordered_set<SafePtr<AGraphNode>> actualizeNodes;

	bool cannotFinishEdit = false;
	for (const auto& pairToEditData_Value : m_oldValues)
	{
		WritePtr<ObjectClass> undoWriteData = pairToEditData_Value.first.get();
		if (!undoWriteData)
		{
			CONTROLLOG << "control::" << m_controlName << " undo : data null" << Logger::endl;
			continue;
		}

		if (m_editCondition(*&undoWriteData, pairToEditData_Value.second))
		{
			actualizeNodes.insert(pairToEditData_Value.first);
			m_setterAttr(*&undoWriteData, pairToEditData_Value.second);
			undoTimeModified(*&undoWriteData);
		}
		else
			cannotFinishEdit = true;
	}

	if(cannotFinishEdit && !m_cannotEditWarningMessage.isEmpty())
		controller.updateInfo(new GuiDataWarning(m_cannotEditWarningMessage));

	controller.actualizeNodes(m_actualizeOptions, actualizeNodes);
}

/*
template<class ObjectClass, class AttrClass>
void ATEditionControl<ObjectClass, AttrClass>::redoSimpleEdition(Controller& controller)
{
	std::vector<SafePtr<ObjectClass>> toEditDatas;
	std::unordered_set<SafePtr<AGraphNode>> actualizeNodes;
	for (auto pairToEditData_value : m_oldValues)
		toEditDatas.push_back(pairToEditData_value.first);
	m_oldValues.clear();

	bool cannotFinishEdit = false;

	for (const auto& toEditDataPair : m_toEditDatas)
	{
		WritePtr<ObjectClass> redoWriteData = toEditData.get();
		if (!redoWriteData)
		{
			CONTROLLOG << "control::" << m_controlName << " redo : data null" << Logger::endl;
			continue;
		}

		if (m_editCondition(*&redoWriteData, m_newValue))
		{
			actualizeNodes.insert(toEditData);
			m_oldValues[toEditData] = m_getterAttr(*&redoWriteData);
			m_setterAttr(*&redoWriteData, m_newValue);
			doTimeModified(*&redoWriteData);
		}
		else
			cannotFinishEdit = true;
	}

	if (cannotFinishEdit && !m_cannotEditWarningMessage.isEmpty())
		controller.updateInfo(new GuiDataWarning(m_cannotEditWarningMessage));

	controller.actualizeNodes(m_actualizeOptions, actualizeNodes);
}
*/

template<class ObjectClass, class AttrClass>
void ATEditionControl<ObjectClass, AttrClass>::setEditCondition(std::function<bool(const ObjectClass&, const AttrClass&)> editCondition)
{
	m_editCondition = editCondition;
}

template<class ObjectClass, class AttrClass>
void ATEditionControl<ObjectClass, AttrClass>::setToEditData(const std::unordered_set<SafePtr<ObjectClass>>& toEditDatas, const AttrClass& newValue)
{
	m_toEditDatas.clear();
	for (const SafePtr<ObjectClass>& data : toEditDatas)
		m_toEditDatas.insert({ data, newValue });
}

#endif //! A_EDITION_CONTROL_HXX