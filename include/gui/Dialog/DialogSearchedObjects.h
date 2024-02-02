#ifndef DIALOG_SEARCHED_OBJECTS_H_
#define DIALOG_SEARCHED_OBJECTS_H_

#include <QtGui/QStandardItemModel>
#include "gui/Dialog/ADialog.h"
#include "ui_DialogSearchedObjects.h"

#include "models/OpenScanToolsModelEssentials.h"

class AGraphNode;

class SearchItem : public QStandardItem
{
public:
	SearchItem(const SafePtr<AGraphNode>& node)
		: QStandardItem()
		, m_node(node)
	{}

public:
	SafePtr<AGraphNode> m_node;
};

class DialogSearchedObjects : public ADialog
{
	Q_OBJECT

public:
	explicit DialogSearchedObjects(IDataDispatcher &dataDispacher, QWidget *parent = 0);
	~DialogSearchedObjects();

	// from IPanel
	void informData(IGuiData *keyValue) override;
	void setSearchedObjects(const std::vector<SafePtr<AGraphNode>>& searchedObjs);

public slots:
	void moveToObject(const QModelIndex& index);
	void selectObjects();
	void hideSelectedObjects();
	void hideUnselectedObjects();

private:
	Ui::DialogSearchedObjects m_ui;
	QStandardItemModel* m_model;
};

#endif // !DIALOG_RECENT_PROJECT_H_