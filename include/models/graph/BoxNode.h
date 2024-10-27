#ifndef BOX_NODE_H
#define BOX_NODE_H

#include "SimpleObjectNode.h"

enum class GridType
{
    NoGrid,
    ByStep,
    ByMultiple
};

class BoxNode : public SimpleObjectNode
{
public:
    BoxNode(const BoxNode& object);
    BoxNode(bool isSimpleBox);
    ~BoxNode();

    ElementType getType() const override;
    TreeType getDefaultTreeType() const override;

    virtual ClippingMode getClippingMode() const override;
    virtual void pushClippingGeometries(ClippingAssembly& clipAssembly, const TransformationModule& transfo) const override;
    virtual void pushRampGeometries(std::vector<std::shared_ptr<IClippingGeometry>>& retGeom, const TransformationModule& transfo) const override;

    virtual void addGenericMeshInstance() override;

    std::unordered_set<Selection> getAcceptableSelections(const ManipulationMode& mode) const override;
    std::unordered_set<ManipulationMode> getAcceptableManipulationModes() const override;

    void setIsSimpleBox(bool simpleBox);
    void setGridType(GridType type);
    void setGridDivision(const glm::vec3& division);

    bool isSimpleBox() const;
    GridType getGridType() const;
    const glm::vec3& getGridDivision() const;
    MeshDrawData getGridMeshDrawData(const glm::dmat4& gTransfo);

private:
    void updateGrid();

private:
    bool grid_need_update;
    GridType grid_type;
    glm::vec3 grid_division;
    // box buffer
    std::shared_ptr<MeshBuffer> grid_sbuf;
};

#endif // !BOX_NODE_H
