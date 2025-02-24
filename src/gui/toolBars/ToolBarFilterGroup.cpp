#include "gui/toolBars/ToolBarFilterGroup.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataAuthor.h"
#include "gui/Dialog/DialogSearchedObjects.h"
#include "services/MarkerDefinitions.hpp"
#include "gui/GuiData/GuiDataList.h"
#include "models/application/Author.h"
#include "models/application/Ids.hpp"

#include "controller/Controller.h"
#include "models/graph/GraphManager.h"
#include "utils/Logger.h"

#include <QtWidgets/qwidget.h>
#include <QtGui/qscreen.h>

ToolBarFilterGroup::ToolBarFilterGroup(IDataDispatcher& dataDispatcher, Controller& controller, QWidget* parent, float guiScale)
	: QWidget(parent)
	, m_dataDispatcher(dataDispatcher)
	, m_filterSystem(controller.getFilterSystem())
	, m_root(controller.getGraphManager().getRoot())
{
	m_ui.setupUi(this);
	setEnabled(false);

	m_storedFromDate = QDateTime::currentDateTime();
	m_storedToDate = QDateTime::currentDateTime();

	m_ui.colorPicker->setAutoUnselect(false);

	QScreen* screen = QGuiApplication::primaryScreen();
	float pixelRatio = screen->logicalDotsPerInch() / 96;

	m_iconSelectionDialog = new MarkerIconSelectionDialog(this, pixelRatio);

	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectLoaded);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::sendListsList);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::sendAuthorsList);

	m_methods.insert({ guiDType::projectLoaded, &ToolBarFilterGroup::onProjectLoad });
	m_methods.insert({ guiDType::sendAuthorsList, &ToolBarFilterGroup::onAuthorList });
	m_methods.insert({ guiDType::sendListsList, &ToolBarFilterGroup::onList });
	
	QObject::connect(m_ui.colorPicker, &ColorPicker::pickedColor, this, &ToolBarFilterGroup::selectTagColor);
	QObject::connect(m_ui.colorPicker, &ColorPicker::unPickedColor, this, &ToolBarFilterGroup::unselectTagColor);
	QObject::connect(m_ui.keyWordLineEdit, SIGNAL(editingFinished()), this, SLOT(filterOnKeyWord()));
	QObject::connect(m_ui.ScanCheckBox, SIGNAL(stateChanged(int)), this, SLOT(toggleScanFilter()));
	QObject::connect(m_ui.TagCheckBox, SIGNAL(stateChanged(int)), this, SLOT(toggleTagFilter()));
	QObject::connect(m_ui.ClippingBoxCheckBox, SIGNAL(stateChanged(int)), this, SLOT(toggleClippingBoxFilter()));
	QObject::connect(m_ui.PipeCheckBox, SIGNAL(stateChanged(int)), this, SLOT(togglePipeFilter()));
	QObject::connect(m_ui.MeasureCheckBox, SIGNAL(stateChanged(int)), this, SLOT(toggleMeasureFilter()));
	QObject::connect(m_ui.PointsCheckBox, SIGNAL(stateChanged(int)), this, SLOT(togglePointFilter()));
	QObject::connect(m_ui.ViewpointCheckBox, SIGNAL(stateChanged(int)), this, SLOT(toggleViewpointFilter()));
	QObject::connect(m_ui.fromDateBtn, SIGNAL(released()), this, SLOT(launchFromCalendar()));
	QObject::connect(m_ui.toDateBtn, SIGNAL(released()), this, SLOT(launchToCalendar()));
	QObject::connect(m_ui.IconFilter, SIGNAL(stateChanged(int)), this, SLOT(toogleIconFilter()));
	QObject::connect(m_ui.KeyWordCheckBox, SIGNAL(stateChanged(int)), this, SLOT(toogleKeywordFilter()));
	QObject::connect(m_ui.DateCheckBox, SIGNAL(stateChanged(int)), this, SLOT(toogleDateFilter()));
	QObject::connect(m_ui.DisciplineCheckBox, SIGNAL(stateChanged(int)), this, SLOT(toogleDisciplineFilter()));
	QObject::connect(m_ui.PhaseCheckBox, SIGNAL(stateChanged(int)), this, SLOT(tooglePhaseFilter()));
	QObject::connect(m_ui.UserCheckBox, SIGNAL(stateChanged(int)), this, SLOT(toogleUserFilter()));
	QObject::connect(m_ui.DisciplineCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onDisciplineChange(int)));
	QObject::connect(m_ui.UserCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onUserChange(int)));
	QObject::connect(m_ui.PhaseCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onPhaseChange(int)));
	QObject::connect(m_iconSelectionDialog, &MarkerIconSelectionDialog::markerSelected, this, &ToolBarFilterGroup::changeMarkerIcon);
	QObject::connect(m_ui.iconTagButton, &QToolButton::released, this, &ToolBarFilterGroup::clickIconTag);
	
	QObject::connect(m_ui.AllCheckBox, SIGNAL(stateChanged(int)), this, SLOT(toggleAllFilter()));
	QObject::connect(m_ui.SphereCheckBox, SIGNAL(stateChanged(int)), this, SLOT(toggleSphereFilter()));
	QObject::connect(m_ui.MeshCheckBox, SIGNAL(stateChanged(int)), this, SLOT(toggleMeshFilter()));
	QObject::connect(m_ui.PcoCheckBox, SIGNAL(stateChanged(int)), this, SLOT(togglePcoObjectFilter()));

	QObject::connect(m_ui.searchButton, &QToolButton::released, this, &ToolBarFilterGroup::updateFilterResult);

	
}

ToolBarFilterGroup::~ToolBarFilterGroup()
{
	m_dataDispatcher.unregisterObserver(this);
}

void ToolBarFilterGroup::unselectTagColor(Color32 color)
{
	m_filterSystem.removeColorFilter(color);
}

void ToolBarFilterGroup::selectTagColor(Color32 color)
{
	m_filterSystem.addColorFilter(color);
}

void ToolBarFilterGroup::informData(IGuiData *data)
{
	if (m_methods.find(data->getType()) != m_methods.end())
	{
		FilterGroupMethod method = m_methods.at(data->getType());
		(this->*method)(data);
	}
}

void ToolBarFilterGroup::toggleTypeFilter(std::unordered_set<ElementType> elements, QCheckBox* checkbox)
{
	if (checkbox->isChecked())
		for (ElementType type : elements)
			m_filterSystem.addTypeSearched(type);
	else
		for (ElementType type : elements)
			m_filterSystem.removeTypeSearched(type);

	if (!checkbox->isChecked() && m_ui.AllCheckBox->isChecked()) {
		m_ui.AllCheckBox->blockSignals(true);
		m_ui.AllCheckBox->setChecked(false);
		m_ui.AllCheckBox->blockSignals(false);
	}
}

void ToolBarFilterGroup::onProjectLoad(IGuiData* data)
{
	GuiDataProjectLoaded* plData = static_cast<GuiDataProjectLoaded*>(data);
	setEnabled(plData->m_isProjectLoad);
	if (plData->m_isProjectLoad == true)
		m_filterSystem.setUserFilter(xg::Guid(m_ui.UserCombo->currentData().toString().toStdString()));
}

void ToolBarFilterGroup::onAuthorList(IGuiData* data)
{
	GuiDataSendAuthorsList *userList = static_cast<GuiDataSendAuthorsList*>(data);

	if (userList->m_isProjectScope == false)
		return;
	xg::Guid currentAuthor = xg::Guid(m_ui.UserCombo->currentData().toString().toStdString());


	m_ui.UserCombo->blockSignals(true);

	m_ui.UserCombo->clear();
	for (const SafePtr<Author>& auth : userList->m_authors)
	{
		ReadPtr<Author> rAuth = auth.cget();
		if (!rAuth)
			continue;
		m_ui.UserCombo->addItem(QString::fromStdWString(rAuth->getName()), QString::fromStdString(rAuth->getId()));
	}

	int ind = m_ui.UserCombo->findData(QString::fromStdString(currentAuthor));
	m_ui.UserCombo->setCurrentIndex(ind);

	if (ind > 0)
		m_filterSystem.setUserFilter(xg::Guid(m_ui.UserCombo->currentData().toString().toStdString()));

	m_ui.UserCombo->blockSignals(false);
}

void ToolBarFilterGroup::onList(IGuiData * data)
{
	const std::unordered_set<SafePtr<UserList>>& lists = static_cast<GuiDataSendListsList*>(data)->_lists; 
	for (SafePtr<UserList> list : lists)
	{
		ReadPtr<UserList> rList = list.cget();
		if (!rList)
			continue;
		xg::Guid id = rList->getId();

		if (id == listId(LIST_DISCIPLINE_ID))
			m_disciplineList = list;

		if (id == listId(LIST_PHASE_ID))
			m_phaseList = list;
	}

	m_ui.DisciplineCombo->blockSignals(true);
	m_ui.PhaseCombo->blockSignals(true);
	std::wstring selected;
	{
		ReadPtr<UserList> rDisc = m_disciplineList.cget();
		selected = m_ui.DisciplineCombo->currentText().toStdWString();
		m_ui.DisciplineCombo->clear();
		if (rDisc)
		{
			int storedId = rDisc->clist().empty() ? -1 : 0;
			int i = 0;
			for (std::wstring data : rDisc->clist())
			{
				m_ui.DisciplineCombo->addItem(QString::fromStdWString(data));
				if (selected == data)
					storedId = i;
				++i;
			}
			if (storedId >= 0)
			{
				m_ui.DisciplineCombo->setCurrentIndex(storedId);
				m_filterSystem.setDisciplineFilter(m_ui.DisciplineCombo->currentText().toStdWString());
			}
		}
	}

	{
		ReadPtr<UserList> rPhase = m_phaseList.cget();
		selected = m_ui.PhaseCombo->currentText().toStdWString();
		m_ui.PhaseCombo->clear();
		if (rPhase)
		{
			int i = 0;

			int storedId = rPhase->clist().empty() ? -1 : 0;

			for (std::wstring data : rPhase->clist())
			{
				m_ui.PhaseCombo->addItem(QString::fromStdWString(data));
				if (selected == data)
					storedId = i;
				++i;
			}
			if (storedId >= 0)
			{
				m_ui.PhaseCombo->setCurrentIndex(storedId);
				m_filterSystem.setDisciplineFilter(m_ui.PhaseCombo->currentText().toStdWString());
			}
		}
	}
	m_ui.DisciplineCombo->blockSignals(false);
	m_ui.PhaseCombo->blockSignals(false);
}

void ToolBarFilterGroup::filterOnKeyWord()
{
	GUI_LOG << "filter on keyWord " << m_ui.keyWordLineEdit->text().toStdString() << LOGENDL;
	m_filterSystem.applyNewKeyWord(m_ui.keyWordLineEdit->text().toStdWString());
}

void ToolBarFilterGroup::toggleScanFilter()
{
	GUI_LOG << "toogle scan" << ((m_ui.ScanCheckBox->checkState() == Qt::CheckState::Checked) ? "true" : "false") << LOGENDL;
	toggleTypeFilter({ ElementType::Scan }, m_ui.ScanCheckBox);
}

//new
void ToolBarFilterGroup::toggleAllFilter()
{
	toggleTypeFilter(s_allTypes, m_ui.AllCheckBox);
	bool check = m_ui.AllCheckBox->isChecked();

	m_ui.ScanCheckBox->blockSignals(true);
	m_ui.ClippingBoxCheckBox->blockSignals(true);
	m_ui.PipeCheckBox->blockSignals(true);
	m_ui.PointsCheckBox->blockSignals(true);
	m_ui.TagCheckBox->blockSignals(true);
	m_ui.MeasureCheckBox->blockSignals(true);
	m_ui.ViewpointCheckBox->blockSignals(true);
	m_ui.SphereCheckBox->blockSignals(true);
	m_ui.MeshCheckBox->blockSignals(true);
	m_ui.PcoCheckBox->blockSignals(true);

	m_ui.ScanCheckBox->setChecked(check);
	m_ui.ClippingBoxCheckBox->setChecked(check);
	m_ui.PipeCheckBox->setChecked(check);
	m_ui.PointsCheckBox->setChecked(check);
	m_ui.TagCheckBox->setChecked(check);
	m_ui.MeasureCheckBox->setChecked(check);
	m_ui.ViewpointCheckBox->setChecked(check);
	m_ui.SphereCheckBox->setChecked(check);
	m_ui.MeshCheckBox->setChecked(check);
	m_ui.PcoCheckBox->setChecked(check);

	m_ui.ScanCheckBox->blockSignals(false);
	m_ui.ClippingBoxCheckBox->blockSignals(false);
	m_ui.PipeCheckBox->blockSignals(false);
	m_ui.PointsCheckBox->blockSignals(false);
	m_ui.TagCheckBox->blockSignals(false);
	m_ui.MeasureCheckBox->blockSignals(false);
	m_ui.ViewpointCheckBox->blockSignals(false);
	m_ui.SphereCheckBox->blockSignals(false);
	m_ui.MeshCheckBox->blockSignals(false);
	m_ui.PcoCheckBox->blockSignals(false);

	GUI_LOG << "toogle all" << ((m_ui.AllCheckBox->checkState() == Qt::CheckState::Checked) ? "true" : "false") << LOGENDL;
}

void ToolBarFilterGroup::toggleTagFilter()
{
	GUI_LOG << "toogle tag " << ((m_ui.TagCheckBox->checkState() == Qt::CheckState::Checked) ? "true" : "false") << LOGENDL;
	toggleTypeFilter({ ElementType::Tag }, m_ui.TagCheckBox);
}

void ToolBarFilterGroup::toggleClippingBoxFilter()
{
	GUI_LOG << "toogle CBox" << ((m_ui.ClippingBoxCheckBox->checkState() == Qt::CheckState::Checked) ? "true" : "false") << LOGENDL;
	toggleTypeFilter({ ElementType::Grid, ElementType::Box }, m_ui.ClippingBoxCheckBox);
}

void ToolBarFilterGroup::togglePipeFilter()
{
	GUI_LOG << "toogle pipe " << ((m_ui.PipeCheckBox->checkState() == Qt::CheckState::Checked) ? "true" : "false") << LOGENDL;
	toggleTypeFilter({ ElementType::Cylinder, ElementType::Torus }, m_ui.PipeCheckBox);
}

//new
void ToolBarFilterGroup::toggleSphereFilter()
{
	GUI_LOG << "toogle sphere " << ((m_ui.SphereCheckBox->checkState() == Qt::CheckState::Checked) ? "true" : "false") << LOGENDL;
	toggleTypeFilter({ ElementType::Sphere }, m_ui.SphereCheckBox);
}

//new
void ToolBarFilterGroup::toggleMeshFilter()
{
	GUI_LOG << "toogle Mesh " << ((m_ui.MeshCheckBox->checkState() == Qt::CheckState::Checked) ? "true" : "false") << LOGENDL;
	toggleTypeFilter({ ElementType::MeshObject }, m_ui.MeshCheckBox);
}

//new
void ToolBarFilterGroup::togglePcoObjectFilter()
{
	GUI_LOG << "toogle Pco Object " << ((m_ui.PcoCheckBox->checkState() == Qt::CheckState::Checked) ? "true" : "false") << LOGENDL;
	toggleTypeFilter({ ElementType::PCO }, m_ui.PcoCheckBox);
}

void ToolBarFilterGroup::togglePointFilter()
{
	GUI_LOG << "toogle point" << ((m_ui.PointsCheckBox->checkState() == Qt::CheckState::Checked) ? "true" : "false") << LOGENDL;
	toggleTypeFilter({ ElementType::PCO }, m_ui.PointsCheckBox);
}

void ToolBarFilterGroup::toggleMeasureFilter()
{
	GUI_LOG << "toogle measure " << ((m_ui.MeasureCheckBox->checkState() == Qt::CheckState::Checked) ? "true" : "false") << LOGENDL;
	toggleTypeFilter({ ElementType::BeamBendingMeasure, ElementType::ColumnTiltMeasure, ElementType::SimpleMeasure,
					   ElementType::PolylineMeasure, ElementType::PipeToPipeMeasure, ElementType::PointToPipeMeasure, 
						ElementType::PipeToPlaneMeasure, ElementType::PointToPlaneMeasure }, m_ui.MeasureCheckBox);
}

void ToolBarFilterGroup::toggleViewpointFilter()
{
	GUI_LOG << "toogle viewpoint" << ((m_ui.ViewpointCheckBox->checkState() == Qt::CheckState::Checked) ? "true" : "false") << LOGENDL;
	toggleTypeFilter({ ElementType::ViewPoint }, m_ui.ViewpointCheckBox);
}

void ToolBarFilterGroup::launchFromCalendar()
{
	m_activeCalendar = new CalendarDialog(static_cast<QWidget*>(this->parent()));
	m_activeCalendar->setCurrentDate(m_storedFromDate.date());
	QObject::connect(m_activeCalendar, &CalendarDialog::finished, this, &ToolBarFilterGroup::getResultFromCalendar);
	QObject::connect(m_activeCalendar, &CalendarDialog::cancel, this, &ToolBarFilterGroup::cancelCalendar);
	m_activeCalendar->show();
}

void ToolBarFilterGroup::getResultFromCalendar(QDate date)
{
	QTime time;
	time.setHMS(00, 00, 01);
	m_storedFromDate.setTime(time);
	m_storedFromDate.setDate(date);
	time_t t = m_storedFromDate.toTime_t();
	m_filterSystem.setMinTime(t);
	m_ui.fromDateBtn->setText(date.toString());
	cancelCalendar();
}

void ToolBarFilterGroup::launchToCalendar()
{
	m_activeCalendar = new CalendarDialog(static_cast<QWidget*>(this->parent()));
	m_activeCalendar->setCurrentDate(m_storedToDate.date());
	QObject::connect(m_activeCalendar, &CalendarDialog::finished, this, &ToolBarFilterGroup::getResultToCalendar);
	QObject::connect(m_activeCalendar, &CalendarDialog::cancel, this, &ToolBarFilterGroup::cancelCalendar);
	m_activeCalendar->show();
}

void ToolBarFilterGroup::getResultToCalendar(QDate date)
{
	GUI_LOG << "receive TO calendar " << date.toString().toStdString() << LOGENDL;
	QTime time;
	time.setHMS(23, 59, 59);
	m_storedToDate.setTime(time);
	m_storedToDate.setDate(date);
	time_t t = m_storedToDate.toTime_t();
	m_filterSystem.setMaxTime(t);
	m_ui.toDateBtn->setText(date.toString());
	cancelCalendar();
}

void ToolBarFilterGroup::cancelCalendar()
{
	m_activeCalendar->close();
	delete(m_activeCalendar);
	m_activeCalendar = nullptr;
}

void ToolBarFilterGroup::changeMarkerIcon(scs::MarkerIcon icon)
{
	m_filterSystem.clearIconFilter();
	m_filterSystem.addIconFilter(icon);
	m_ui.iconTagButton->setIcon(QIcon(scs::markerStyleDefs.at(icon).qresource));
}

void ToolBarFilterGroup::clickIconTag()
{
	m_iconSelectionDialog->show();
}

void ToolBarFilterGroup::toogleIconFilter()
{
	GUI_LOG << "toogle icon filter " << ((m_ui.IconFilter->checkState() == Qt::CheckState::Checked) ? "true" : "false") << LOGENDL;
	m_filterSystem.setIconFilterStatus(m_ui.IconFilter->isChecked());
}

void ToolBarFilterGroup::toogleDateFilter()
{
	GUI_LOG << "toogle date filter " << ((m_ui.DateCheckBox->checkState() == Qt::CheckState::Checked) ? "true" : "false") << LOGENDL;
	m_filterSystem.setTimeFilterStatus(m_ui.DateCheckBox->isChecked());
}

void ToolBarFilterGroup::toogleKeywordFilter()
{
	GUI_LOG << "toogle keyword filter " << ((m_ui.KeyWordCheckBox->checkState() == Qt::CheckState::Checked) ? "true" : "false") << LOGENDL;
	m_filterSystem.setKeywordFilterStatus(m_ui.KeyWordCheckBox->isChecked());
}

void ToolBarFilterGroup::toogleDisciplineFilter()
{
	GUI_LOG << "toogle discipline filter " << ((m_ui.DisciplineCheckBox->checkState() == Qt::CheckState::Checked) ? "true" : "false") << LOGENDL;
	m_filterSystem.setDisciplineFilterStatus(m_ui.DisciplineCheckBox->isChecked());
}

void ToolBarFilterGroup::tooglePhaseFilter()
{
	GUI_LOG << "toogle phase filter " << ((m_ui.PhaseCheckBox->checkState() == Qt::CheckState::Checked) ? "true" : "false") << LOGENDL;
	m_filterSystem.setPhaseFilterStatus(m_ui.PhaseCheckBox->isChecked());
}

void ToolBarFilterGroup::toogleUserFilter()
{
	GUI_LOG << "toogle user filter " << ((m_ui.UserCheckBox->checkState() == Qt::CheckState::Checked) ? "true" : "false") << LOGENDL;
	m_filterSystem.setUserFilterStatus(m_ui.UserCheckBox->isChecked());
}

void ToolBarFilterGroup::onPhaseChange(int i)
{
	GUI_LOG << "change phase filter " << m_ui.PhaseCombo->currentText().toStdString() << LOGENDL;
	m_filterSystem.setPhaseFilter(m_ui.PhaseCombo->currentText().toStdWString());
}

void ToolBarFilterGroup::onDisciplineChange(int i)
{
	GUI_LOG << "change discipline filter " << m_ui.DisciplineCombo->currentText().toStdString() << LOGENDL;
	m_filterSystem.setDisciplineFilter(m_ui.DisciplineCombo->currentText().toStdWString());
}

void ToolBarFilterGroup::onUserChange(int i)
{
	GUI_LOG << "change user filter " << m_ui.UserCombo->currentText().toStdString() << LOGENDL;
	m_filterSystem.setUserFilter(xg::Guid(m_ui.UserCombo->currentData().toString().toStdString()));
}

void ToolBarFilterGroup::updateFilterResult()
{
	std::vector<SafePtr<AGraphNode>> nodes;
	std::unordered_set<SafePtr<AGraphNode>> geoChildren = AGraphNode::getGeometricChildren_rec(m_root);

	for (const SafePtr<AGraphNode>& graphNode : geoChildren)
	{
		{
			ReadPtr<AGraphNode> readGraph = graphNode.cget();
			if (!readGraph || readGraph->getGraphType() != AGraphNode::Type::Object)
				continue;
		}

		{
			SafePtr<AObjectNode> objectNode = static_pointer_cast<AObjectNode>(graphNode);
			if (m_filterSystem.filter(objectNode))
				nodes.push_back(graphNode);
		}
	}

	if (m_activeSearchedObjectsDialog)
	{
		m_activeSearchedObjectsDialog->close();
		delete m_activeSearchedObjectsDialog;
		m_activeSearchedObjectsDialog = nullptr;
	}

	m_activeSearchedObjectsDialog = new DialogSearchedObjects(m_dataDispatcher, this);
	m_activeSearchedObjectsDialog->setSearchedObjects(nodes);
}
