#ifndef TREEMODEL_H
#define TREEMODEL_H

#include <QtCore/QAbstractItemModel>
#include <QtCore/QModelIndex>
#include <QtCore/QVariant>
#include <QtGui/QStandardItemModel>
#include "gui/widgets/TreeNodeSystem/TreeNode.h"
//#include "gui/widgets/TreeNodeSystem/DataTreeNode.h"

class AGraphNode;
/*! QUESTION(quentin) Arbre côté UI ? Il n'y a qu'un TreeModel qui gère les DataNode pour l'affichage UI
	tandis que il y a une map de TreeSytem (associé à un TreeType) qui gère les TreeElement  */
class TreeModel : public QStandardItemModel
{
	Q_OBJECT

public:
	explicit TreeModel(int row, int columns, QObject *parent = 0);
	~TreeModel();

	Qt::ItemFlags flags(const QModelIndex &index) const override;
	Qt::DropActions supportedDropActions() const override;
	Qt::DropActions supportedDragActions() const override;
	QStringList mimeTypes() const override;
	QMimeData *mimeData(const QModelIndexList &indexes) const Q_DECL_OVERRIDE;
	bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex & parent);

	// Getter
	std::unordered_map<SafePtr<AGraphNode>, std::vector<TreeNode*>>& getTreeNodes();
	std::vector<TreeNode*> getTreeNodes(SafePtr<AGraphNode> data);

private:
	/*! Retourne un pointeur TreeNode situé à l'index associé _index_*/
	TreeNode *nodeForIndex(const QModelIndex &index) const;
	void removeNode(TreeNode *node);

private:
    /*! Pour avoir une indexation des TreeNode par xg::Guid  */
    // FIXME - Change 'vector' to 'unordered_set' or 'multimap'
    std::unordered_map<SafePtr<AGraphNode>, std::vector<TreeNode*>> m_dataToTreeNodes;
};

#endif