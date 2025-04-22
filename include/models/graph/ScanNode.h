#ifndef SCAN_NODE_H
#define SCAN_NODE_H

#include "models/graph/APointCloudNode.h"

class ScanNode : public APointCloudNode
{
public:
    ScanNode(const ScanNode& node);
    ScanNode();

    ~ScanNode();

    ElementType getType() const override;
    TreeType getDefaultTreeType() const override;

    //scs::MarkerIcon getIconType() const;

    void setMarkerColor(const Color32& color);
    Color32 getMarkerColor() const;

    std::wstring getComposedName() const override;

    std::unordered_set<Selection> getAcceptableSelections(const ManipulationMode& mode) const;
    std::unordered_set<ManipulationMode> getAcceptableManipulationModes() const override;

protected:
    Color32 m_markerColor;
};

#endif //! SCAN_NODE_H_