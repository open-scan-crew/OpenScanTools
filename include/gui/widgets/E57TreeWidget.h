#ifndef E57_TREE_WIDGET
#define E57_TREE_WIDGET

#include <QtWidgets/QTreeWidget>

namespace e57 {
    class Node;
}

class E57TreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    E57TreeWidget(QWidget* parent);
    ~E57TreeWidget();

    void inspect(const QString& filePath); // return bool if inspection is correct

    //static QTreeWidgetItem* getTreeWidgetItem(const QString& filePath);

private:
    static void constructTreeNode(QTreeWidgetItem* parent, e57::Node& node);

private:
    QString m_filePath;

};

#endif