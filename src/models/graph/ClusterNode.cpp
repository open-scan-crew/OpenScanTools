#include "models/graph/ClusterNode.h"
#include "models/3d/ManipulationTypes.h"
//#include "models/graph/PointNode.h"
//#include "models/graph/ScanNode.h"
//#include "models/graph/TagNode.h"
//#include "controller/controls/ControlObject3DEdition.h"
//#include "gui/DataDispatcher.h"
#include "gui/texts/DefaultNameTexts.hpp"

ClusterNode::ClusterNode(const ClusterNode& node)
	: AObjectNode(node)
	, ClusterData(node)
	, m_isMasterCluster(false)
{
}

ClusterNode::ClusterNode()
	: m_isMasterCluster(false)
{
	setName(TEXT_DEFAULT_NAME_CLUSTER.toStdWString());
}

ClusterNode::~ClusterNode()
{}

ElementType ClusterNode::getType() const
{
	return m_isMasterCluster ? ElementType::MasterCluster : ElementType::Cluster;
}

bool ClusterNode::isAcceptableOwningChild(const SafePtr<AGraphNode>& child) const
{
	ElementType type;
	TreeType childDefaultTreeType;
	{
		ReadPtr<AGraphNode> rChild = child.cget();
		if (!rChild)
			return false;
		if (rChild->getId() == getId())
			return false;
		type = rChild->getType();
		childDefaultTreeType = rChild->getDefaultTreeType();
	}

	switch (m_clusterTreeType)
	{
		case TreeType::Hierarchy:
		{
			if (type == ElementType::Cluster && childDefaultTreeType != TreeType::Hierarchy)
				return false;
			return true;
		}
		break;
		case TreeType::Piping:
		{
			if (type == ElementType::Cylinder
				|| type == ElementType::Torus)
				return true;
		}
		break;
		default:
		{
			if (childDefaultTreeType == m_clusterTreeType)
				return true;
		}
		break;
	}

	if (m_clusterTreeType == TreeType::Hierarchy)
		return true;

	return false;
}

TreeType ClusterNode::getDefaultTreeType() const
{
	return getClusterTreeType();
}

std::wstring ClusterNode::getComposedName() const
{
	return m_name;
}

std::unordered_set<Selection> ClusterNode::getAcceptableSelections(const ManipulationMode& mode) const
{
	switch (mode)
	{
	case ManipulationMode::Translation:
	case ManipulationMode::Rotation:
		return { Selection::X, Selection::Y, Selection::Z };
	}
	return {};
}

std::unordered_set<ManipulationMode> ClusterNode::getAcceptableManipulationModes() const
{
	return { ManipulationMode::Translation, ManipulationMode::Rotation };
}

bool ClusterNode::setGeometricChildren(const SafePtr<ClusterNode>& parentCluster, const std::unordered_set<SafePtr<AObjectNode>>& children)
{
	glm::dvec3 centers(0);
	uint32_t counter(0);
	std::unordered_set<SafePtr<AObjectNode>> newChildren;
	for (const SafePtr<AObjectNode>& child : children)
	{
		ReadPtr<AObjectNode> rChild = child.cget();
		if (!rChild)
			continue;
		centers += rChild->getTranslation(true);
		newChildren.insert(child);
		counter++;
	}
	if (counter == 0)
		return false;
	{
		WritePtr<ClusterNode> wCluster = parentCluster.get();
		centers /= counter;
		wCluster->setPosition(centers);
	}

	for (const SafePtr<AObjectNode>& child : newChildren)
		AGraphNode::addGeometricLink(parentCluster, child);

	return true;
}