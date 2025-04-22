#include "models/graph/ScanNode.h"
#include "models/3d/ManipulationTypes.h"

ScanNode::ScanNode(const ScanNode& node)
	: APointCloudNode(node)
{
}

ScanNode::ScanNode()
{
	Data::marker_icon_ = scs::MarkerIcon::Scan_Base;
}

ScanNode::~ScanNode()
{
}

Color32 ScanNode::getMarkerColor() const
{
	return m_markerColor;
}

std::wstring ScanNode::getComposedName() const
{
	return m_name;
}

void ScanNode::setMarkerColor(const Color32& color)
{
    m_markerColor = color;
}

ElementType ScanNode::getType() const
{
	return ElementType::Scan;
}

TreeType ScanNode::getDefaultTreeType() const
{
	return TreeType::Scan;
}

std::unordered_set<Selection> ScanNode::getAcceptableSelections(const ManipulationMode& mode) const
{
	switch (mode)
	{
	case ManipulationMode::Translation:
	case ManipulationMode::Rotation:
		return { Selection::X, Selection::Y, Selection::Z };
	}
	return {};
}

std::unordered_set<ManipulationMode> ScanNode::getAcceptableManipulationModes() const
{
	return { ManipulationMode::Translation, ManipulationMode::Rotation }; 
}