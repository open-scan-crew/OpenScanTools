#ifndef PROJECT_TREE_PANEL_H
#define PROJECT_TREE_PANEL_H

#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"
#include "gui/widgets/TreeNodeSystem/TreeModel.h"
#include "gui/widgets/TreeNodeSystem/TreeNode.h"

#include <QtWidgets/qtreeview.h>

#include <vector>

class ProjectTreePanel;

typedef void (ProjectTreePanel::* treeMethod)(IGuiData*);

/*! Gère l'UI de l'arborescence */
class ProjectTreePanel : public QTreeView, public IPanel
{
    Q_OBJECT

public:

    ProjectTreePanel(IDataDispatcher& dataDispatcher, GraphManager& graphManager, float guiScale);
    ~ProjectTreePanel();

    void informData(IGuiData* keyValue) override;

private:
    void blockAllSignals(bool block);

    /*! Mise à jour de l'UI de l'arborescence  */
    void actualizeNodes(IGuiData* data);
    void selectItems(IGuiData* data);
    void cleanTree(IGuiData* data);

    void applyWaitingRefresh();
    /*! Remove from treeId map
        Remove from xg::Guid  multimap
        Romove from DataNode parent
    */
    void removeTreeNode(TreeNode* node);
    /*! Rafraîchit l'affichage de l'objet dans l'arbre*/

    void addRootToUpdateSystem(TreeNode* node, TreeType treeType);

    void updateSelection(const std::unordered_set<SafePtr<AGraphNode>>& datas);

    void generateTreeModel();

    TreeNode* buildTreeModelBranch(const QString& name, TreeType type);
    TreeNode* constructTreeNode(const SafePtr<AGraphNode>& data, TreeNode* parent, TreeType treetype);
    TreeNode* constructMasterNode(const QString& name, TreeType treeType);
    TreeNode* constructPlaceholderNode(TreeType treeType);
    TreeNode* constructStructNode(const QString& name,
        TreeNode::fctGetChildren getChildFonction,
        TreeNode::fctCanDrop can_drop,
        TreeNode::fctOnDrop on_drop,
        TreeNode* parent = nullptr);

    TreeNode* addSubList(TreeNode* parentNode, QString name, ElementType type);
    // Special for Box/Grid filter
    TreeNode* addBoxesSubList(TreeNode* parentNode, QString name, bool only_box);
    TreeNode* constructColorNodes(const std::unordered_set<ElementType>& types, TreeNode* parentNode);
    TreeNode* constructStatusNodes(const std::unordered_set<ElementType>& types, TreeNode* parentNode);
    TreeNode* constructClipMethodNodes(const std::unordered_set<ElementType>& types, TreeNode* parentNode);

    void updateNode(TreeNode* node);

    void treeDragEvent(QDragEnterEvent* dEvent);
    void treeMoveEvent(QDragMoveEvent* mEvent);
    void treeDropEvent(QDropEvent* dEvent);

    /*! Vérifie si le noeud _origin_ peut être avoir comme parent _dest_ */
    bool checkDropAreaIsValid(TreeNode* dest, const SafePtr<AGraphNode>& dropData);

    void onExpand(const QModelIndex& ind);
    void onCollapse(const QModelIndex& ind);

    void createCluster();
    void exportScan();

    /*! Supprime les objets sélectionnés dans l'arbre */
    void deleteTreeElement();
    void multiChangeAttributes();
    void removeElemFromHierarchy(); 
    void disconnectPiping();
    void moveToItem();
    void enableClipping();
    void disableClipping();
    void disableAllClippings();
    void disableAllRamps();
    void externCliping();
    void internClipping();
    void pickItems();
    void dropManualItems();
    void dropElem(TreeNode* dest);
    void selectClusterChildrens();

public slots:
    void treeViewMoveProgress(const QModelIndex& doubleClickedInd);
    void treeSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void collapseChildren(const QModelIndex& collapsInd);

    void onTreeDataChanged(QStandardItem* item);
    /*! Génère le menu contextuel apparaîsant selon le point QPoint p cliqué */
    void showTreeMenu(QPoint p); // to nodes

private:
    /*! Redéfinition de la méthode Qt qui traite l'évenement de 'Drag' (correspond au début du Drag&Drop) */
    void dragEnterEvent(QDragEnterEvent* qevent) override;
    /*! Redéfinition de la méthode Qt qui traite l'évenement de 'DragMove' (se répète entre un évènement de 'Drag' et 'Drop') */
    void dragMoveEvent(QDragMoveEvent* mevent) override;
    /*! Redéfinition de la méthode Qt qui traite l'évenement de 'Drop' (correspond à la fin du Drag&Drop) */
    void dropEvent(QDropEvent* qevent) override;

private:
    Qt::CheckState recGetNodeState(TreeNode* node);

private:

    IDataDispatcher& m_dataDispatcher;
    GraphManager& m_graphManager;

    QStandardItem* m_highlightNode;
    QBrush m_lastHighLightBrush;

    TreeNode* m_lastTreeNodeSelected = nullptr;
    TreeNode* m_itemsNode = nullptr;

    std::list<SafePtr<AGraphNode>> m_dropList;
    std::unordered_set<SafePtr<AGraphNode>> m_pickedItems;
    QPoint m_lastPoint;

    /*! Arbre côté UI */
    TreeModel* m_model = nullptr;

    typedef void (ProjectTreePanel::*GuiDataFunction)(IGuiData*);
    inline void registerGuiDataFunction(guiDType type, GuiDataFunction fct)
    {
        m_dataDispatcher.registerObserverOnKey(this, type);
        m_methods.insert({ type, fct });
    };
    std::unordered_map<guiDType, treeMethod> m_methods;

    std::unordered_set<SafePtr<AGraphNode>> m_selectedNodes;

    std::unordered_map<TreeType, std::vector<TreeNode*>> m_rootNodes;
};

#endif
