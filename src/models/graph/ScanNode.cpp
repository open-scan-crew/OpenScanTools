#include "models/graph/ScanNode.h"
#include "models/3d/ManipulationTypes.h"
#include "services/MarkerDefinitions.hpp"

#define SGLog Logger::log(LoggerMode::SceneGraphLog)

ScanNode::ScanNode(const ScanNode& node)
	: APointCloudNode(node)
{
	updateMarker();
}

ScanNode::ScanNode()
{
	updateMarker();
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
    if (color == m_markerColor)
        return;
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

scs::MarkerIcon ScanNode::getIconType() const
{
    return scs::MarkerIcon::Scan_Base;
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

MarkerDrawData ScanNode::getMarkerDrawData(const glm::dmat4& gTransfo) const
{
	// Compose the style
	uint32_t status = 0;
	if (m_selected)
		status |= 0x01;
	if (m_isHovered)
		status |= 0x02;
	if (scs::markerStyleDefs.at(scs::MarkerIcon::Scan_Base).showTrueColor)
		status |= 0x04;

	return {
		{ (float)gTransfo[3][0], (float)gTransfo[3][1], (float)gTransfo[3][2] },
		{ m_color.r, m_color.g, m_color.b, m_color.a },
		m_graphicId,
		(uint32_t)scs::MarkerIcon::Scan_Base,
		m_primitiveDef.firstVertex,
		m_primitiveDef.vertexCount,
		status
	};
}

void ScanNode::updateMarker()
{
	scs::MarkerStyleDefinition marker_style = scs::markerStyleDefs.at(scs::MarkerIcon::Scan_Base);
	m_showMarkerTrueColor = marker_style.showTrueColor;
	m_primitiveDef = scs::g_shapePrimitives.at(marker_style.shape);
}