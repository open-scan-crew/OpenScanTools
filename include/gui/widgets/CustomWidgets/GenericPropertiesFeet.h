#ifndef GENERIC_PROPERTIES_FEET_H_
#define GENERIC_PROPERTIES_FEET_H_

#include "ui_widget_generic_properties_feet.h"
#include "gui/Dialog/HyperlinkAddDialog.h"
#include "gui/IDataDispatcher.h"
#include "models/Types.hpp"
#include "models/data/Data.h"
#include "utils/safe_ptr.h"

class AObjectNode;

class GenericPropertiesFeet: public QWidget
{
public:
	GenericPropertiesFeet(QWidget* parent = Q_NULLPTR, float pixelRatio = 1.f);
	~GenericPropertiesFeet();

	void setDataDispatcher(IDataDispatcher& dataDispatcher);
	void setObject(const SafePtr<AObjectNode>& pData);
	void hideEvent(QHideEvent* event) override;

private:
	void prepareUi(ElementType objectType);
	void setDataInformations(const SafePtr<AObjectNode>& object);
	void getClusterPath(std::wstring& hName, std::wstring& hPath, std::wstring& dName, std::wstring& dPath, std::wstring& pName, const SafePtr<AObjectNode>& object);

public slots:
	void addHyperlink(std::wstring hyperlink, std::wstring name);
	void handleContextHyperlink(hLinkId link);
	void deleteHyperlink(hLinkId id);

private:
	Ui::WidgetGenericPropertiesFeet m_ui;
	SafePtr<AObjectNode>			m_stored;
	HyperlinkAddDialog*				m_hyperLinkDial;
	IDataDispatcher*				m_dataDispatcher;
};

#endif // !GENERIC_PROPERTIES_FEET_H_
