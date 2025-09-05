#ifndef A_EDITION_CONTROL_H
#define A_EDITION_CONTROL_H

#include "IControl.h"
#include "utils/safe_ptr.h"
#include <unordered_set>
#include <unordered_map>
#include <functional>
#include <string>
#include <qstring.h>

class Data;

class AEditionControl : public AControl
{
public:

	AEditionControl();
	AEditionControl(const std::time_t& redoTime);
	~AEditionControl();

protected:
	void doTimeModified(Data& data);
	void undoTimeModified(Data& data);

protected:
	std::time_t m_undoTimeModified;
	std::time_t m_doTimeModified;
};

template<class ObjectClass, class AttrClass>
class ATEditionControl : public AEditionControl
{
public:

	ATEditionControl(const std::unordered_map<SafePtr<ObjectClass>, AttrClass>& toEditDatas, const std::string& controlName,
		std::function<void(ObjectClass&, const AttrClass&)> setterAttr,
		std::function<AttrClass(const ObjectClass&)> getterAttr);
	ATEditionControl(const std::unordered_set<SafePtr<ObjectClass>>& toEditDatas, const AttrClass& newValue, const std::string& controlName,
		std::function<void(ObjectClass&, const AttrClass&)> setterAttr, 
		std::function<AttrClass (const ObjectClass&)> getterAttr);
	ATEditionControl(const std::string& controlName,
		std::function<void(ObjectClass&, const AttrClass&)> setterAttr,
		std::function<AttrClass(const ObjectClass&)> getterAttr);
	~ATEditionControl();

	virtual void doFunction(Controller& controller) override;
	virtual bool canUndo() const override;
	virtual void undoFunction(Controller& controller) override;
	virtual void redoFunction(Controller& controller) override;

protected:
	void doSimpleEdition(Controller& controller);

	void undoSimpleEdition(Controller& controller);
	//void redoSimpleEdition(Controller& controller);

	void setEditCondition(std::function<bool(const ObjectClass&, const AttrClass&)> editCondition);
	void setToEditData(const std::unordered_set<SafePtr<ObjectClass>>& toEditDatas, const AttrClass& newValue);

protected:
	QString m_cannotEditWarningMessage = QString();

	std::unordered_map<SafePtr<ObjectClass>, AttrClass> m_toEditDatas;
	std::unordered_map<SafePtr<ObjectClass>, AttrClass> m_oldValues;

	std::string m_controlName;

	std::function<void(ObjectClass&, const AttrClass&)> m_setterAttr;
	std::function<AttrClass(const ObjectClass&)> m_getterAttr;

	std::function<bool(const ObjectClass&, const AttrClass&)> m_editCondition = [](const ObjectClass&, const AttrClass&) {return true; };
};

#endif //! A_EDITION_CONTROL_H