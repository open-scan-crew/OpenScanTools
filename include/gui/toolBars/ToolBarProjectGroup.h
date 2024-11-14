#ifndef TOOLBAR_PROJECTGROUP_H
#define TOOLBAR_PROJECTGROUP_H

#include <QtWidgets/qwidget.h>
#include "ui_toolbar_projectgroup.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"
#include "models/project/ProjectInfos.h"

class ToolBarProjectGroup;

typedef void (ToolBarProjectGroup::* ProjectGroupMethod)(IGuiData*);

class ToolBarProjectGroup : public QWidget, public IPanel
{
	Q_OBJECT

public:
	explicit ToolBarProjectGroup(IDataDispatcher &dataDispatcher, QWidget *parent, float scale);
    ~ToolBarProjectGroup();

    void informData(IGuiData *keyValue) override;

public slots:
	void manageAuthors(bool displayCloseButton);
	void openRecentSave();

private:
	std::unordered_map<guiDType, ProjectGroupMethod> m_methods;
	Ui::ToolBarProjectGroup m_ui;
    IDataDispatcher &m_dataDispatcher;
    bool m_projectSaved;
    float m_guiScale;
	bool m_saveable;

private:
	void onProjectLoad(IGuiData *data);
private slots:
    void slotProperties();
};

#endif // TOOLBAR_PROJECTGROUP_H