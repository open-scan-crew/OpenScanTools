#ifndef _PROPERTIESPROJECT_PANEL_H_
#define _PROPERTIESPROJECT_PANEL_H_

#include "ui_Property_Project.h"

#include "gui/widgets/APropertyGeneral.h"

#include "models/application/Author.h"
#include "models/project/ProjectInfos.h"

class PropertiesProjectPanel : public APropertyGeneral
{
	Q_OBJECT

public:

	PropertiesProjectPanel(IDataDispatcher& dataDispatcher, QWidget* parent);
	~PropertiesProjectPanel();

	void informData(IGuiData *keyValue) override;
	bool actualizeProperty(SafePtr<AGraphNode> object) override;

	void onProjectProperties(IGuiData *data);
	void hideEvent(QHideEvent* event);

public slots:
	void slotEditCompany();
	void slotEditLocation();
	void slotEditDescription();

private:
	void sendAEditProjectControl();
	void launchFileBrowserCustomScanFolder();

private:
	Ui::PropertyProject m_ui;

	ProjectInfos storedProjectInfo;

	bool canEditDesc;
	QString m_openPath;
};

#endif // _PROPERTIESPROJECT_PANEL_H_

