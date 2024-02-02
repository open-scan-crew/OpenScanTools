#include "models/3d/Graph/ScanObjectNode.h"
#include "utils/Logger.h"
#include "vulkan/VulkanManager.h"
#include "models/3d/ManipulationTypes.h"
#include "controller/Controller.h"

#define SGLog Logger::log(LoggerMode::SceneGraphLog)

ScanObjectNode::ScanObjectNode()
	: APointCloudNode()
{
	m_clippable = false;
}

ScanObjectNode::ScanObjectNode(const ScanObjectNode & node)
	: APointCloudNode(node)
{
	m_clippable = false;
}

ScanObjectNode::~ScanObjectNode()
{}

ElementType ScanObjectNode::getType() const
{
	return ElementType::PCO;
}

TreeType ScanObjectNode::getDefaultTreeType() const
{
	return TreeType::Pco;
}

std::unordered_set<Selection> ScanObjectNode::getAcceptableSelections(const ManipulationMode& mode) const
{
	switch (mode)
	{
	case ManipulationMode::Translation:
	case ManipulationMode::Rotation:
	case ManipulationMode::Extrusion:
		return { Selection::X, Selection::Y, Selection::Z };
	case ManipulationMode::Scale:
	default:
		return {};
	}
}

std::unordered_set<ManipulationMode> ScanObjectNode::getAcceptableManipulationModes() const
{
	return { ManipulationMode::Translation, ManipulationMode::Rotation };
}