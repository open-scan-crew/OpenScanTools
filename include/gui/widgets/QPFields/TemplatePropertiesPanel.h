#ifndef TEMPLATE_PROPERTIES_PANEL_H_
#define TEMPLATE_PROPERTIES_PANEL_H_
#include <vector>

#include <QtWidgets/qwidget.h>
#include <QtWidgets/QLayout>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qcombobox.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/qscrollarea.h>
#include <QtWidgets/QTableWidget>

#include "gui/widgets/APropertyGeneral.h"
#include "gui/widgets/QPFields/AQPField.h"
#include "models/application/TagTemplate.h"
#include "gui/widgets/QPFields/QPFactory.h"
#include "gui/widgets/CustomWidgets/ColorPicker.h"
#include "gui/Dialog/MarkerIconSelectionDialog.h"

#include "ui_Property_Tag.h"

class TagNode;

class Controller;

class TemplatePropertiesPanel;

typedef void (TemplatePropertiesPanel::* TemplatePropertiesMethod)(IGuiData*);

class TemplatePropertiesPanel : public APropertyGeneral
{
	Q_OBJECT

public:
	TemplatePropertiesPanel(Controller& controller, QWidget *parent);
	~TemplatePropertiesPanel();

	void addField(sma::tFieldId id, AQPField*field, QVBoxLayout* layout);

	void informData(IGuiData *data) override;

	virtual bool actualizeProperty(SafePtr<AGraphNode> object );

private:
	bool updateTag();

signals:
    void nameChanged(QString name);

public slots:

	void getFieldModification(sma::tFieldId id, std::wstring newData);

	void onShapePress();
	void changeMarkerIcon(scs::MarkerIcon icon);

	void changePosition();
	void onXEdit();
	void onYEdit();
	void onZEdit();

private:
	Ui::PropertyTag m_ui;
	QPFactory m_fieldFactory;
	std::wstring m_templateName = L"";
	SafePtr<TagNode> m_tag;
	std::unordered_map<sma::tFieldId, AQPField*> m_fields;
	std::unordered_map<guiDType, TemplatePropertiesMethod> m_tagsMethods;

   	MarkerIconSelectionDialog m_iconSelector;
};

#endif // !TEMPLATE_PROPERTIES_PANEL_H_