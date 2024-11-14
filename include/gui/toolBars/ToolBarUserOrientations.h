#ifndef TOOLBAR_USERORIENTATIONS_H
#define TOOLBAR_USERORIENTATIONS_H

#include <QtWidgets/qwidget.h>
#include <forward_list>
#include "ui_toolbar_userOrientation.h"

#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"
#include "models/application/UserOrientation.h"


class ToolBarUserOrientation;

typedef void (ToolBarUserOrientation::* UserOrientationMethod)(IGuiData*);

class ToolBarUserOrientation : public QWidget, public IPanel
{
	Q_OBJECT

public:
	explicit ToolBarUserOrientation(IDataDispatcher &dataDispatcher, QWidget *parent, float guiScale);
	~ToolBarUserOrientation();

	// From IPanel
	void informData(IGuiData *keyValue);

private:
	void onProjectLoad(IGuiData* data);
	void onEditUO(IGuiData* data);
	void onLoadUO(IGuiData* data);

	void changeOrientation(bool isUserOrientation);

private slots:
	void slotSetOrientation(int idComboBox);
    void slotEditOrientation(int idComboBox);
	void slotNewOrientation();

private:
	std::vector<userOrientationId> m_idList;

	Ui::ToolBarUserOrientation m_ui;
    IDataDispatcher &m_dataDispatcher;
	std::unordered_map<guiDType, UserOrientationMethod> m_methods;

	bool m_useUserOrientation;


};

#endif // TOOLBAR_VIEWINGMODEGROUP_H

