#ifndef GENERIC_PROPERTIES_HEAD_H_
#define GENERIC_PROPERTIES_HEAD_H_

#include "ui_widget_generic_properties_head.h"
#include "gui/IDataDispatcher.h"
#include "models/ElementType.h"
#include "utils/safe_ptr.h"

class Controller;
class ControllerContext;
class AGraphNode;

class GenericPropertiesHead : public QWidget
{

public:
	GenericPropertiesHead(QWidget* parent = Q_NULLPTR, float pixelRatio = 1.f);
	~GenericPropertiesHead();
	void setControllerInfo(const Controller& controller);
	void setObject(SafePtr<AGraphNode> object);
	void hideEvent(QHideEvent* event) override;

private:
	void update();
	void updatePhaseDiscipline(ReadPtr<AGraphNode>& rObject);
	void updateType(ElementType objectType);

	void blockWidgets(bool block);

private slots:
	void onUserIndexEdit();
	void onColorChange(const Color32& color);
	void onDisciplineChange();
	void onPhaseChange();
	void onNameEdit();
	void onDescEdit();
	void onIdentifierEdit();

private:
	Ui::WidgetGenericPropertiesHead m_ui;
	IDataDispatcher* m_dataDispatcher;
	const ControllerContext* m_controllerContext;
	SafePtr<AGraphNode> m_object;
};

#endif // !GENERIC_PROPERTIES_HEAD_H_
