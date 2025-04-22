#ifndef COLUMN_TILT_MEASURE_NODE_H
#define COLUMN_TILT_MEASURE_NODE_H

#include "models/graph/AObjectNode.h"
#include "models/data/ColumnTiltMeasure/ColumnTiltMeasureData.h"

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

    virtual void setTopPoint(const Pos3D& tp) override;

    void updateColumnTiltMeasure(const ColumnTiltMeasureNode& data);
};

#endif
