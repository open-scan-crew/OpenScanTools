#include "gui/Dialog/DialogSearchedObjects.h"
#include "gui/style/IconObject.h"
#include "models/graph/TagNode.h"
#include "controller/controls/ControlSpecial.h"
#include "gui/GuiData/GuiData3dObjects.h"

DialogSearchedObjects::DialogSearchedObjects(IDataDispatcher& dataDispatcher, QWidget *parent)
	: ADialog(dataDispatcher, parent)
{
	m_ui.setupUi(this);
	this->hide();
	setModal(false);
	setSizeGripEnabled(true);

	m_ui.searchedObjectListView->setDragDropMode(QAbstractItemView::DragDropMode::NoDragDrop);
	m_ui.searchedObjectListView->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
	m_ui.searchedObjectListView->setDragEnabled(false);
	m_ui.searchedObjectListView->setAcceptDrops(false);
	m_ui.searchedObjectListView->setDropIndicatorShown(false);

	m_ui.hideSelectedButton->hide();
	m_ui.hideUnselectedButton->hide();

	QObject::connect(m_ui.okButton, &QPushButton::released, this, &DialogSearchedObjects::hide);
	QObject::connect(m_ui.searchedObjectListView, &QListView::doubleClicked, this, &DialogSearchedObjects::moveToObject);
	QObject::connect(m_ui.hideSelectedButton, &QPushButton::released, this, &DialogSearchedObjects::hideSelectedObjects);
	QObject::connect(m_ui.hideUnselectedButton, &QPushButton::released, this, &DialogSearchedObjects::hideUnselectedObjects);
}

DialogSearchedObjects::~DialogSearchedObjects()
{
	if (m_model)
		delete m_model;
}

void DialogSearchedObjects::informData(IGuiData *data)
{
}

void DialogSearchedObjects::setSearchedObjects(const std::vector<SafePtr<AGraphNode>>& searchedObjs)
{
	m_model = new QStandardItemModel(0, 0);
	int i = 0;

	for (const SafePtr<AGraphNode>& obj : searchedObjs)
	{
		std::wstring name;
		ElementType type;
		Color32 color;
		scs::MarkerIcon icon = scs::MarkerIcon::Max_Enum;
		{
			ReadPtr<AGraphNode> rObj = obj.cget();
			if (!rObj)
				continue;
			type = rObj->getType();
			name = rObj->getComposedName();
			color = rObj->getColor();
			icon = rObj->getMarkerIcon();
		}

		SearchItem* item = new SearchItem(obj);
		item->setIcon(scs::IconManager::getInstance().getIcon(icon, color));
		item->setText(QString::fromStdWString(name));
		m_model->setItem(i, 0, item);

		i++;
	}

	m_ui.searchedObjectListView->setModel(m_model);
	QObject::connect(m_ui.searchedObjectListView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &DialogSearchedObjects::selectObjects);

	show();
}

void DialogSearchedObjects::selectObjects()
{
	std::unordered_set<SafePtr<AGraphNode>> toSelectNodes;
	for (const QModelIndex& ind : m_ui.searchedObjectListView->selectionModel()->selectedIndexes())
	{
		QStandardItem* item = m_model->itemFromIndex(ind);
		if (!item)
			continue;
		toSelectNodes.insert(static_cast<SearchItem*>(item)->m_node);
	}
	m_dataDispatcher.sendControl(new control::special::MultiSelect(toSelectNodes, true));
}

void DialogSearchedObjects::hideSelectedObjects()
{
	m_dataDispatcher.sendControl(new control::special::ShowHideCurrentObjects(false));
}

void DialogSearchedObjects::hideUnselectedObjects()
{
	m_dataDispatcher.sendControl(new control::special::ShowHideUncurrentObjects(false));
}

void DialogSearchedObjects::moveToObject(const QModelIndex& index)
{
	QStandardItem* item = m_model->itemFromIndex(index);
	if (!item)
		return;

	m_dataDispatcher.updateInformation(new GuiDataMoveToData(static_cast<SearchItem*>(item)->m_node));
}
