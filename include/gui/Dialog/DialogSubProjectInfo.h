#ifndef DIALOG_SUBPROJECT_INFO_H_
#define DIALOG_SUBPROJECT_INFO_H_

#include "gui/IDataDispatcher.h"
#include "ui_DialogSubProjectInfo.h"

#include <qdialog.h>

#include "models/project/ProjectInfos.h"

class DialogSubProjectInfo : public QDialog
{
	Q_OBJECT

public:

	DialogSubProjectInfo(IDataDispatcher& dataDispatcher, QWidget* parent);
	~DialogSubProjectInfo();

	void setProjectInfos(const ProjectInfos& info);
	ProjectInfos getProjectInfos();

private:
	void onOkButton();

private:
	Ui::DialogSubProjectInfo m_ui;
	ProjectInfos storedInfo;
};

#endif // _PROPERTIESPROJECT_PANEL_H_

