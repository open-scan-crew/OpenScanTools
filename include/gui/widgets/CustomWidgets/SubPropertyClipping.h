#ifndef SUB_PROPERTY_CLIPPING_H_
#define SUB_PROPERTY_CLIPPING_H_

#include "ui_SubProperty_Clipping.h"
#include "gui/IDataDispatcher.h"
#include "models/ElementType.h"
#include "utils/safe_ptr.h"

class Controller;
class AClippingNode;

class SubPropertyClipping : public QWidget
{
	Q_OBJECT

public:
	explicit SubPropertyClipping(QWidget *parent = Q_NULLPTR, float guiScale = 1.f);
	~SubPropertyClipping();
	void hideEvent(QHideEvent* event) override;

	void setObject(const SafePtr<AClippingNode>& object);
	void setDataDispatcher(IDataDispatcher* dataDispatcher);

private:
	void update();
	void prepareUi(ElementType type);

	void blockSignals(bool value);


public slots:
	void onShowInteriorClick();
	void onShowExteriorClick();
	void onShowPhaseClick();
	void onActiveClipping();
	void onMinClipDistEdit();
	void onMaxClipDistEdit();

	void onRampActive(bool active);
	void onRampMin();
	void onRampMax();
	void onRampSteps(int value);
	void onRampClamp(int value);

private:
	Ui::SubPropertyClipping m_ui;
	SafePtr<AClippingNode> m_storedClip;
	IDataDispatcher* m_dataDispatcher = nullptr;
};

#endif //SUB_PROPERTY_CLIPPING_H_
