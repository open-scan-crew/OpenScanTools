#include "gui/widgets/CustomWidgets/GenericPropertiesFeet.h"
#include "controller/controls/ControlObject3DEdition.h"
#include "controller/controls/ControlDataEdition.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/widgets/FocusWatcher.h"
#include <QDoubleValidator>
#include <QIntValidator>
#include "models/graph/ClusterNode.h"
#include <QtWidgets/qmenu.h>
#include <QDir>
#include "gui/Texts.hpp"

#include "utils/Logger.h"

GenericPropertiesFeet::GenericPropertiesFeet(QWidget* parent, float pixelRatio)
	: QWidget(parent)
	, m_stored()
	, m_hyperLinkDial(nullptr)
	, m_dataDispatcher(nullptr)
{
	m_ui.setupUi(this);

	m_ui.LinksTableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem("links"));
	m_ui.LinksTableWidget->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);

	m_ui.ItemLineInfield->hide();
	m_ui.ItemLineLabel->hide();
	adjustSize();
}

GenericPropertiesFeet::~GenericPropertiesFeet()
{
	if (m_hyperLinkDial != nullptr)
		delete m_hyperLinkDial;
}

void GenericPropertiesFeet::setDataDispatcher(IDataDispatcher & dataDispatcher)
{
	m_dataDispatcher = &dataDispatcher;

	m_hyperLinkDial = new HyperlinkAddDialog(dataDispatcher, this->parentWidget());

	connect(m_ui.AddLinkPushButton, &QPushButton::clicked, this, [this]() { m_hyperLinkDial->show(); });
	connect(m_hyperLinkDial, &HyperlinkAddDialog::onCreatedLink, this, &GenericPropertiesFeet::addHyperlink);
}

void GenericPropertiesFeet::setObject(const SafePtr<AObjectNode>& pData)
{
	m_stored = pData;

	{
		ReadPtr<AObjectNode> object = pData.cget();
		if (!object)
			return;

		prepareUi(object->getType());
	}

	setDataInformations(pData);
}

void GenericPropertiesFeet::hideEvent(QHideEvent* event)
{
	m_ui.DateCreatedInfield->blockSignals(true);
	m_ui.ModifiedDateInfield->blockSignals(true);
	m_ui.InternalInfield->blockSignals(true);
	m_ui.HierarchyClusterInfield->blockSignals(true);
	m_ui.HierarchyPathInfield->blockSignals(true);
}

void GenericPropertiesFeet::setDataInformations(const SafePtr<AObjectNode>& object)
{

	m_ui.DateCreatedInfield->blockSignals(true);
	m_ui.ModifiedDateInfield->blockSignals(true);
	m_ui.InternalInfield->blockSignals(true);
	m_ui.HierarchyClusterInfield->blockSignals(true);
	m_ui.HierarchyPathInfield->blockSignals(true);

	std::unordered_map<hLinkId, s_hyperlink> links;

	{
		ReadPtr<AObjectNode> rObject = object.cget();
		if (rObject)
		{
			m_ui.DateCreatedInfield->setText(QString::fromStdWString(rObject->getStringTimeCreated()));
			m_ui.ModifiedDateInfield->setText(QString::fromStdWString(rObject->getStringTimeModified()));
			m_ui.InternalInfield->setText(QString::fromStdString(rObject->getId()));

			links = rObject->getHyperlinks();
		}
	}

	int i = 0;

	m_ui.LinksTableWidget->clear();
	m_ui.LinksTableWidget->setColumnCount(1);
	m_ui.LinksTableWidget->setRowCount((int)links.size());
	m_ui.LinksTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	for (auto it = links.begin(); it != links.end(); it++)
	{
		QLabel* hlinkLabel = new QLabel(this);
		hlinkLabel->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
		hLinkId tmplink = it->first;
		QObject::connect(hlinkLabel, &QLabel::customContextMenuRequested, [this, tmplink]() { this->handleContextHyperlink(tmplink); });

		hlinkLabel->setTextFormat(Qt::RichText);
		hlinkLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
		hlinkLabel->setOpenExternalLinks(true);

		hlinkLabel->setText("<a href=\"" + QString::fromStdWString(it->second.hyperlink) + "\">" + QString::fromStdWString(it->second.name) + "</a>");
		hlinkLabel->setToolTip(QString::fromStdWString(it->second.hyperlink));
		m_ui.LinksTableWidget->setCellWidget(i, 0, hlinkLabel);
		//hLinkLinker.insert(std::pair<int, hLinkId>(i, it->first));
		++i;
	}

	std::wstring hName;
	std::wstring hPath;

	std::wstring dName;
	std::wstring dPath;

	std::wstring pName;

	getClusterPath(hName, hPath, dName, dPath, pName, object);

	m_ui.HierarchyClusterInfield->setText(QString::fromStdWString(hName));
	m_ui.HierarchyPathInfield->setText(QString::fromStdWString(hPath));

	m_ui.ItemClusterInfield->setText(QString::fromStdWString(dName));
	m_ui.ItemPathInfield->setText(QString::fromStdWString(dPath));


	if (!pName.empty())
	{
		m_ui.ItemLineInfield->show();
		m_ui.ItemLineLabel->show();

		m_ui.ItemLineInfield->setText(QString::fromStdWString(pName));
	}
	else
	{
		m_ui.ItemLineInfield->hide();
		m_ui.ItemLineLabel->hide();
	}

	m_ui.DateCreatedInfield->blockSignals(false);
	m_ui.ModifiedDateInfield->blockSignals(false);
	m_ui.InternalInfield->blockSignals(false);
	m_ui.HierarchyClusterInfield->blockSignals(false);
	m_ui.HierarchyPathInfield->blockSignals(false);
}

void GenericPropertiesFeet::getClusterPath(std::wstring& hName, std::wstring& hPath, std::wstring& dName, std::wstring& dPath, std::wstring& pName, const SafePtr<AObjectNode>& object)
{
	TreeType dTreeType;
	{
		ReadPtr<AObjectNode> rObject = object.cget();
		if (!rObject)
			return;
		dTreeType = rObject->getDefaultTreeType();
	}

	auto rec = [&hName, &hPath, &dName, &dPath, &pName, &dTreeType](const SafePtr<AGraphNode>& node)
	{
		ElementType type;
		std::wstring name;
		{
			ReadPtr<AGraphNode> rNode = node.cget();
			if (!rNode)
				return false;
			type = rNode->getType();
			name = rNode->getName();
		}

		if (type != ElementType::Cluster)
			return true;

		TreeType treetype;
		{
			ReadPtr<ClusterNode> rCluster = static_pointer_cast<ClusterNode>(node).cget();
			if (!rCluster)
				return false;
			treetype = rCluster->getDefaultTreeType();
		}

		if (treetype == dTreeType)
		{
			if (dName.empty())
				dName = name;
			dPath = name + L'\\' + dPath;
			return true;
		}

		if (treetype == TreeType::Hierarchy)
		{
			if (hName.empty())
				hName = name;
			hPath = name + L'\\' + hPath;
			return true;
		}

		if (treetype == TreeType::Piping)
		{
			if (pName.empty())
				pName = name;
			return false;
		}

		return false;
	};

	for (SafePtr<AGraphNode> parent : AGraphNode::getOwningParents(object))
	{
		std::unordered_set<SafePtr<AGraphNode>> visitedNodes;
		AGraphNode::recOnAncestors(parent, [](const SafePtr<AGraphNode>& node) {return AGraphNode::getOwningParents(node); }, visitedNodes, rec);
	}
}

void GenericPropertiesFeet::handleContextHyperlink(hLinkId link)
{
	GUI_LOG << "context custom" << LOGENDL;
	if (link.isValid() == false)
		return;

	QMenu* menu = new QMenu(this);

	QAction* delHLink = new QAction(TEXT_TAG_REMOVELINK, this);
	menu->addAction(delHLink);
	QObject::connect(delHLink, &QAction::triggered, [this, link]() { deleteHyperlink(link); });
	menu->popup(QCursor::pos());
}

void GenericPropertiesFeet::deleteHyperlink(hLinkId id)
{
	m_dataDispatcher->sendControl(new control::dataEdition::removeHyperlink(m_stored, id));
}

void GenericPropertiesFeet::prepareUi(ElementType objectType)
{
	switch (objectType)
	{
		case ElementType::Scan:
		{
			m_ui.ModifiedDateInfield->setVisible(false);
			m_ui.ModifiedDateLabel->setVisible(false);

			m_ui.CreatedDateLabel->setVisible(false);
			m_ui.DateCreatedInfield->setVisible(false);

			m_ui.InternalIdLabel->setVisible(false);
			m_ui.InternalInfield->setVisible(false);
		}
		break;
		default:
		{
			m_ui.ModifiedDateInfield->setVisible(true);
			m_ui.ModifiedDateLabel->setVisible(true);

			m_ui.CreatedDateLabel->setVisible(true);
			m_ui.DateCreatedInfield->setVisible(true);

			m_ui.InternalIdLabel->setVisible(true);
			m_ui.InternalInfield->setVisible(true);
		}
	}
}


void GenericPropertiesFeet::addHyperlink(std::wstring hyperlink, std::wstring name)
{
	if(m_dataDispatcher)
		m_dataDispatcher->sendControl(new control::dataEdition::addHyperlink(m_stored, hyperlink, name));
}
