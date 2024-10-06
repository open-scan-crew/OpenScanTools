#ifndef POINT_NODE_H_
#define POINT_NODE_H_

#include "models/graph/AClippingNode.h"
#include "models/3d/MarkerDrawData.h"
#include "models/project/Marker.h"

class PointNode : public AClippingNode
{
public:
    PointNode(const PointNode& node);
    PointNode();
    ~PointNode();

    ElementType getType() const override;
    TreeType getDefaultTreeType() const override;

    virtual void pushClippingGeometries(ClippingAssembly& clipAssembly, const TransformationModule& transfo) const override;
    virtual void pushRampGeometries(std::vector<std::shared_ptr<IClippingGeometry>>& retGeom, const TransformationModule& transfo) const override;

    std::unordered_set<Selection> getAcceptableSelections(const ManipulationMode& mode) const override;
    std::unordered_set<ManipulationMode> getAcceptableManipulationModes() const override;

    void setColor(const Color32& color) override;

    scs::MarkerIcon getIconType() const;
    // TODO - future marker interface
    MarkerDrawData getMarkerDrawData(const glm::dmat4& gTransfo) const;

    void updateMarker();

protected:
    // New Marker Data
    bool m_showMarkerTrueColor;
    scs::PrimitiveDef m_primitiveDef;
};

#endif // !POINT_NODE_H_