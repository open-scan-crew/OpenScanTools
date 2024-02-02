#ifndef TOOLBAR_FILTERGROUP_H
#define TOOLBAR_FILTERGROUP_H

#include <QtWidgets/QWidget>
#include <QtCore/QDateTime>
#include "ui_toolbar_filtergroup.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"
#include "gui/dialog/CalendarDialog.h"
#include "gui/Dialog/DialogSearchedObjects.h"
#include "gui/dialog/MarkerIconSelectionDialog.h"
#include "models/application/List.h"

#include "controller/FilterSystem.h"

class Controller;
class AGraphNode;
class ToolBarFilterGroup;

typedef void (ToolBarFilterGroup::* FilterGroupMethod)(IGuiData*);


class ToolBarFilterGroup : public QWidget, public IPanel
{
	Q_OBJECT

public:
	explicit ToolBarFilterGroup(IDataDispatcher &dataDispatcher, Controller& controller, QWidget *parent = 0, float guiScale = 1.f);

	// From IPanel
	void informData(IGuiData *keyValue);
	void toggleTypeFilter(std::unordered_set<ElementType> elements, QCheckBox* checkbox);


public slots:
	void filterOnKeyWord();
	void toggleScanFilter();
	void toggleTagFilter();
	void toggleClippingBoxFilter();
	void togglePipeFilter();
	void togglePointFilter();
	void toggleMeasureFilter();
	void toggleViewpointFilter();
	void launchFromCalendar();
	void getResultFromCalendar(QDate date);
	void launchToCalendar();
	void getResultToCalendar(QDate date);
	void cancelCalendar();
	void changeMarkerIcon(scs::MarkerIcon icon);
	void clickIconTag();
	void toogleIconFilter();
	void toogleDateFilter();
	void toogleKeywordFilter();
	void toogleDisciplineFilter();
	void tooglePhaseFilter();
	void toogleUserFilter();
	void onPhaseChange(int i);
	void onDisciplineChange(int i);
	void onUserChange(int i);

	//new
	void toggleAllFilter();
	void toggleSphereFilter();
	void toggleMeshFilter();
	void togglePcoObjectFilter();

	void updateFilterResult();

private:
    ~ToolBarFilterGroup();
	void selectTagColor(Color32 color);
	void unselectTagColor(Color32 color);
	void onProjectLoad(IGuiData* data);
	void onAuthorList(IGuiData* data);
	void onList(IGuiData* data);

private:
	QDateTime m_storedFromDate;
	QDateTime m_storedToDate;

	FilterSystem& m_filterSystem;
	SafePtr<AGraphNode> m_root;

	std::unordered_map<guiDType, FilterGroupMethod> m_methods;
	
	SafePtr<UserList> m_disciplineList;
	SafePtr<UserList> m_phaseList;

	//std::vector<std::string> m_disciplineSS;
	//std::vector<std::string> m_phaseSS;
	//std::vector<std::string> m_usersSS;

	MarkerIconSelectionDialog *m_iconSelectionDialog;
	CalendarDialog *m_activeCalendar; 
	DialogSearchedObjects* m_activeSearchedObjectsDialog;

	Ui::ToolBarFilterGroup m_ui;
    IDataDispatcher &m_dataDispatcher;
};

#endif // TOOLBAR_FILTERGROUP_H

