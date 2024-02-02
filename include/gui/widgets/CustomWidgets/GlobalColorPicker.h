#ifndef GLOBAL_COLORPICKER_H_
#define GLOBAL_COLORPICKER_H_

#include "gui/widgets/CustomWidgets/ColorPicker.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"

class IGuiData;

class GlobalColorPicker : public ColorPicker, public IPanel
{
	public:
		GlobalColorPicker(IDataDispatcher* dataDispatcher, QWidget* parent = Q_NULLPTR, float pixelRatio = 1.f);
		GlobalColorPicker(QWidget* parent = Q_NULLPTR, float pixelRatio = 1.f);
		~GlobalColorPicker();
		void setDataDispatcher(IDataDispatcher* dataDispatcher);
		void informData(IGuiData* keyValue);

	private slots:
		void selectColor(QPushButton* qbutton) override;

	private:
		IDataDispatcher* m_dataDispatcher;
};

#endif // !GLOBAL_COLORPICKER_H_
