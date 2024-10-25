#ifndef BEAM_BENDING_MEASURE_NODE_H
#define BEAM_BENDING_MEASURE_NODE_H

#include "models/graph/AObjectNode.h"
#include "models/data/BeamBendingMeasure/BeamBendingMeasureData.h"
#include "models/3d/MarkerDrawData.h"
#include "models/project/Marker.h"

class BeamBendingMeasureNode : public AObjectNode, public BeamBendingMeasureData
{
public:
    BeamBendingMeasureNode(const BeamBendingMeasureNode& node);
    BeamBendingMeasureNode();
    ~BeamBendingMeasureNode();

    void setColor(const Color32& color) override;
    ElementType getType() const override;
    TreeType getDefaultTreeType() const override;

    std::unordered_set<Selection> getAcceptableSelections(const ManipulationMode& mode) const override;
    std::unordered_set<ManipulationMode> getAcceptableManipulationModes() const override;

    scs::MarkerIcon getIconType() const;
    // TODO - future marker interface
    MarkerDrawData getMarkerDrawData(const glm::dmat4& gTransfo) const;

    virtual void setMaxBendingPos(const Pos3D& pos) override;

    void updateMarker();

protected:

    // New Marker Data
    bool m_showMarkerTrueColor;
    scs::PrimitiveDef m_primitiveDef;
};

#endif
