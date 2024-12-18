#ifndef COLUMN_TILT_MEASURE_NODE_H
#define COLUMN_TILT_MEASURE_NODE_H

#include "models/graph/AObjectNode.h"
#include "models/data/ColumnTiltMeasure/ColumnTiltMeasureData.h"
#include "models/3d/MarkerDrawData.h"
#include "models/project/Marker.h"

// NOTE(robin) - Pas besoin d'hériter de AMeasureNode pour l'instant: pas de segment à afficher
class ColumnTiltMeasureNode : public AObjectNode, public ColumnTiltMeasureData
{
public:
    ColumnTiltMeasureNode(const ColumnTiltMeasureNode& node);
    ColumnTiltMeasureNode();

    ~ColumnTiltMeasureNode();

    ElementType getType() const override;
    TreeType getDefaultTreeType() const override;

    std::unordered_set<Selection> getAcceptableSelections(const ManipulationMode& mode) const override;
    std::unordered_set<ManipulationMode> getAcceptableManipulationModes() const override;

    void setColor(const Color32& color) override;
    scs::MarkerIcon getIconType() const;
    // TODO - future marker interface
    MarkerDrawData getMarkerDrawData(const glm::dmat4& gTransfo) const;

    virtual void setTopPoint(const Pos3D& tp) override;

    void updateColumnTiltMeasure(const ColumnTiltMeasureNode& data);
    void updateMarker();

protected:

    // New Marker Data
    bool m_showMarkerTrueColor;
    scs::PrimitiveDef m_primitiveDef;
};

#endif
