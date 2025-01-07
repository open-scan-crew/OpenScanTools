#include "gui/widgets/TreeNodeSystem/TreeView.h"
#include <QtWidgets/qheaderview.h>

TreeView::TreeView(QWidget *parent) : QTreeView(parent)
{
	setDragEnabled(true);
	setAcceptDrops(true);
	resizeColumnToContents(0);
	setSelectionMode(QAbstractItemView::SingleSelection);
	header()->setVisible(false);
	setEditTriggers(QAbstractItemView::EditTriggers());
}

void TreeView::dragEnterEvent(QDragEnterEvent * qevent)
{
	QModelIndex index = indexAt(qevent->pos());

	if (index.isValid() == false)
	{
		qevent->setDropAction(Qt::IgnoreAction);
		return;
	}

	emit myDragEvent(qevent);
}

void TreeView::dragMoveEvent(QDragMoveEvent *mevent)
{
	QModelIndex index = indexAt(mevent->pos());

	if (index.isValid() == false)
	{
		mevent->setDropAction(Qt::IgnoreAction);
		return;
	}

	emit myMoveEvent(mevent);
}

void TreeView::dropEvent(QDropEvent * dEvent)
{
	QModelIndex index = indexAt(dEvent->pos());

	if (index.isValid() == false)
	{
		dEvent->setDropAction(Qt::IgnoreAction);
		return;
	}

	emit myDropEvent(dEvent);
}