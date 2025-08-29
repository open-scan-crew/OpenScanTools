#ifndef MULTI_PROPERTY_H_
#define MULTI_PROPERTY_H_

#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"
#include "gui/Dialog/HyperlinkAddDialog.h"
#include "models/data/Data.h"
#include "utils/safe_ptr.h"

#include "ui_MultiProperty.h"
#include <QtWidgets/qwidget.h>

#include <unordered_set>
#include <vector>

class Controller;
class ControllerContext;
class AGraphNode;

class MultiProperty : public QWidget, public IPanel
{
	Q_OBJECT

public:
	explicit MultiProperty(Controller& controller, QWidget* parent = nullptr, float guiScale = 1.f);
	~MultiProperty();

private:
	void informData(IGuiData* keyValue);
	void onMultiData(IGuiData* data);
	void onActivateChanges();

	void updatePhaseDiscipline();
	void updateToolButton(QToolButton* button);
	void clearFields();

	void updateLinkTable();

	void toggleToolButton(QToolButton* button);

public slots:
	void addHyperlink(std::wstring hyperlink, std::wstring name);
	void handleContextHyperlink(int linkIndex);
	void deleteHyperlink(int linkIndex);

private:
	Ui::multiProperty m_ui;
	std::unordered_set<SafePtr<AGraphNode>> m_objects;
	IDataDispatcher& m_dataDispatcher;
	const ControllerContext& m_context;

	Color32 m_selectedColor;
	std::vector<s_hyperlink> m_hyperLinks;

	HyperlinkAddDialog* m_hyperLinkDial;

	std::unordered_map<QToolButton*, std::vector<QWidget*>> m_toolbuttonWidgets;
};

#endif //PropertyPoint