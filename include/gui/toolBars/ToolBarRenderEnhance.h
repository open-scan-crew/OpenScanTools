#ifndef TOOLBAR_RENDER_ENHANCE_H
#define TOOLBAR_RENDER_ENHANCE_H

#include "ui_toolbar_renderenhance.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"

#include <unordered_map>

class ToolBarRenderEnhance : public QWidget, public IPanel
{
	Q_OBJECT

public:
	explicit ToolBarRenderEnhance(IDataDispatcher& dataDispatcher, QWidget* parent = 0, float guiScale = 1.f);
	~ToolBarRenderEnhance() override = default;

private:
	void informData(IGuiData* data) override;
	void onProjectLoad(IGuiData* data);

	typedef void (ToolBarRenderEnhance::* GuiDataFunction)(IGuiData*);
	inline void registerGuiDataFunction(guiDType type, GuiDataFunction fct)
	{
		m_dataDispatcher.registerObserverOnKey(this, type);
		m_methods.insert({ type, fct });
	};

	std::unordered_map<guiDType, GuiDataFunction> m_methods;
	Ui::toolbar_renderenhance m_ui;
	IDataDispatcher& m_dataDispatcher;
};

#endif // TOOLBAR_RENDER_ENHANCE_H
