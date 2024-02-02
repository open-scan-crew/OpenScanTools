#include "gui/widgets/TreeNodeSystem/TreeModel.h"

#include <iostream>
#include <QtCore/QCoreApplication>
#include <QtCore/QStringList>
#include <QtCore/QMimeData>
#include <QtCore/QIODevice>
#include <QtCore/QDataStream>

TreeModel::TreeModel(int row, int columns, QObject *parent)
	: QStandardItemModel(row, columns, parent)
{
}

TreeModel::~TreeModel()
{
}

static const char s_treeNodeMimeType[] = "application/x-treenode";

//returns the mime type
QStringList TreeModel::mimeTypes() const
{
	//std::cout << "mimitype : " << s_treeNodeMimeType << std::endl;
	return (QStringList() << s_treeNodeMimeType);
}

//receives a list of model indexes list
QMimeData *TreeModel::mimeData(const QModelIndexList &indexes) const
{
	QMimeData *mimeData = new QMimeData;
	QByteArray data; //a kind of RAW format for datas

	//QDataStream is independant on the OS or proc architecture
	//serialization of C++'s basic data types, like char, short, int, char *, etc.
	//Serialization of more complex data is accomplished
	//by breaking up the data into primitive units.
	QDataStream stream(&data, QIODevice::WriteOnly);
	QList<TreeNode *> nodes;

	//
	foreach (const QModelIndex &index, indexes)
	{
		TreeNode *node = nodeForIndex(index);
		if (nodes.contains(node) == false)
			nodes << node;
	}
	stream << QCoreApplication::applicationPid();
	stream << nodes.count();
	foreach (TreeNode *node, nodes)
		stream << reinterpret_cast<qlonglong>(node);
	mimeData->setData(s_treeNodeMimeType, data);
	return (mimeData);
}

bool TreeModel::dropMimeData(const QMimeData *mimeData, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
	return (false);
}

Qt::DropActions TreeModel::supportedDropActions() const
{
	return (Qt::MoveAction);
}

Qt::DropActions TreeModel::supportedDragActions() const
{
	return (Qt::MoveAction);
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
	if (index.isValid() == false)
		return (Qt::ItemIsDropEnabled);

	return (QStandardItemModel::flags(index) | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
}

//returns a pointer to the "index"
TreeNode *TreeModel::nodeForIndex(const QModelIndex &index) const
{
	//Si l'index n'est pas valide, on renvoit le pointeur racine
	if (index.isValid() == false)
		return (nullptr);
	else
		return (static_cast<TreeNode*>(index.internalPointer()));
}

void TreeModel::removeNode(TreeNode *node)
{
	//int row = node->getRow();
	int row = node->row();
	QModelIndex idx = createIndex(row, 0, node);
	beginRemoveRows(idx.parent(), row, row);
	node->parent()->removeRow(row);
	endRemoveRows();
}

std::unordered_map<SafePtr<AGraphNode>, std::vector<TreeNode*>>& TreeModel::getTreeNodes()
{
	return (m_dataToTreeNodes);
}

std::vector<TreeNode*> TreeModel::getTreeNodes(SafePtr<AGraphNode> data)
{
	if (m_dataToTreeNodes.find(data) == m_dataToTreeNodes.end())
		return std::vector<TreeNode*>();
	return m_dataToTreeNodes.at(data);
}
