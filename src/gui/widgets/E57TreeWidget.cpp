#include "gui/widgets/E57TreeWidget.h"
#include "utils/time.h"

#include "openE57.h"

#include <iostream>

E57TreeWidget::E57TreeWidget(QWidget* parent) : QTreeWidget(parent)
{
    setColumnCount(3);
    setHeaderHidden(false);
    setHeaderLabels({ tr("Name"), tr("Type"), tr("Content") });

    setColumnWidth(0, 160);
    setColumnWidth(1, 80);
    //resizeColumnToContents(0);
    setIndentation(12);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
}


void E57TreeWidget::inspect(const QString& filePath)
{
    m_filePath = filePath;
    clear();

    try {
        // Read the file from disk
        e57::ImageFile imf(m_filePath.toStdString(), "r");
        e57::StructureNode root = imf.root();

        QTreeWidgetItem* treeRoot = new QTreeWidgetItem((QTreeWidget*)this, QStringList(QString("Root")));

        int64_t childCount = root.childCount();
        // Start the recursive construction of the tree
        for (int i = 0; i < childCount; i++) {
            e57::Node child = root.get(i);
            constructTreeNode(treeRoot, child);
        }
        imf.close();

        insertTopLevelItem(0, treeRoot);
        treeRoot->setExpanded(true);
    }
    catch (e57::E57Exception& ex) {
        ex.report(__FILE__, __LINE__, __FUNCTION__);
        return;
    }
    catch (std::exception& ex) {
        std::cerr << "Got an std::excetion, what=" << ex.what() << std::endl;
        return;
    }
    catch (...) {
        std::cerr << "Got an unknown exception" << std::endl;
        return;
    }
}

E57TreeWidget::~E57TreeWidget()
{
}

void E57TreeWidget::constructTreeNode(QTreeWidgetItem* parent, e57::Node& _node)
{
    QTreeWidgetItem* treeItem = new QTreeWidgetItem(parent);
    treeItem->setText(0, QString::fromStdString(_node.elementName()));

    switch (_node.type())
    {
    case e57::E57_STRUCTURE:
    {
        e57::StructureNode sNode(_node);
        int64_t childCount = sNode.childCount();

        treeItem->setText(1, "Structure");
        treeItem->setText(2, "");

        for (int64_t i = 0; i < childCount; i++)
        {
            e57::Node child = sNode.get(i);
            constructTreeNode(treeItem, child);
        }
        break;
    }
    case e57::E57_VECTOR:
    {
        e57::VectorNode vNode(_node);
        int64_t childCount = vNode.childCount();

        treeItem->setText(1, "Vector");
        treeItem->setText(2, QString::number(childCount) + " elements");

        for (int64_t i = 0; i < childCount; i++)
        {
            e57::Node child = vNode.get(i);
            constructTreeNode(treeItem, child);
        }
        break;
    }
    case e57::E57_COMPRESSED_VECTOR:
    {
        e57::CompressedVectorNode cvNode(_node);
        int ptsCount = cvNode.childCount();

        treeItem->setText(1, "Compresssed Vector");
        treeItem->setText(2, QString::number(ptsCount) + " records");

        e57::StructureNode proto(cvNode.prototype());
        int attribCount = proto.childCount();

        for (int i = 0; i < attribCount; i++)
        {
            e57::Node child = proto.get(i);
            constructTreeNode(treeItem, child);
        }

        break;
    }
    case e57::E57_INTEGER:
    {
        treeItem->setText(1, "Integer");
        treeItem->setText(2, QString::number(e57::IntegerNode(_node).value()));
        break;
    }
    case e57::E57_SCALED_INTEGER:
    {
        e57::ScaledIntegerNode scIntNode(_node);

        treeItem->setText(1, "Scaled Integer");
        treeItem->setText(2, QString::number(scIntNode.scaledValue()));

        QTreeWidgetItem* rawValue = new QTreeWidgetItem(treeItem, QStringList({ "Raw Value", "int64", QString::number(scIntNode.rawValue()) }));
        QTreeWidgetItem* scale = new QTreeWidgetItem(treeItem, QStringList({ "Scale", "Double", QString::number(scIntNode.scale()) }));
        QTreeWidgetItem* offset = new QTreeWidgetItem(treeItem, QStringList({ "Offset", "Double", QString::number(scIntNode.offset()) }));
        break;
    }
    case e57::E57_FLOAT:
    {
        e57::FloatNode fNode(_node);
        if (fNode.precision() == e57::E57_SINGLE)
            treeItem->setText(1, "Float");
        else
            treeItem->setText(1, "Double");

        treeItem->setText(2, QString::number(fNode.value()));

        if (fNode.elementName() == "dateTimeValue")
        {
            uint64_t utcSeconds = scs::dtime_gps_to_utc(fNode.value());
            std::time_t utcTime(utcSeconds);

            char strDate[128];
            std::strftime(strDate, sizeof(strDate), "%F %T %Z", std::localtime(&utcTime));

            treeItem->setText(2, QString::fromLatin1(strDate));
        }

        break;
    }
    case e57::E57_STRING:
    {
        treeItem->setText(1, "String");
        treeItem->setText(2, QString::fromStdString(e57::StringNode(_node).value()));
        break;
    }
    case e57::E57_BLOB:
    {
        treeItem->setText(1, "Blob");
        treeItem->setText(2, QString::number(e57::BlobNode(_node).byteCount()) + " bytes");
        break;
    }
    default:
        break;
    }
}