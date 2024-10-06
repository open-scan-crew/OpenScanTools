#ifndef VIEWPOINTNODE_H_
#define VIEWPOINTNODE_H_

#include "models/graph/AObjectNode.h"
#include "models/data/ViewPoint/ViewPointData.h"
#include "models/3d/MarkerDrawData.h"
#include "models/project/Marker.h"

class ViewPointNode : public AObjectNode, public ViewPointData
{
public:
    ViewPointNode();
    ViewPointNode(const ViewPointNode& node);
    ~ViewPointNode();

    ElementType getType() const override;
    TreeType getDefaultTreeType() const override;

    std::unordered_set<Selection> getAcceptableSelections(const ManipulationMode& mode) const override;
    std::unordered_set<ManipulationMode> getAcceptableManipulationModes() const override;

    scs::MarkerIcon getIconType() const;
    // TODO - future marker interface
    MarkerDrawData getMarkerDrawData(const glm::dmat4& gTransfo) const;

    void updateMarker();

protected:
    // New Marker Data
    bool m_showMarkerTrueColor;
    scs::PrimitiveDef m_primitiveDef;
};

#endif // !VIEWPOINTNODE_H_