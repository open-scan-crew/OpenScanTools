#include "models/graph/BoxNode.h"
#include "models/3d/ManipulationTypes.h"
#include "models/3d/GridCalculation.h"
#include "gui/texts/DefaultNameTexts.hpp"
#include "vulkan/MeshManager.h"

BoxNode::BoxNode()
    : SimpleObjectNode()
    , grid_need_update(true)
    , grid_type(GridType::NoGrid)
    , grid_division(1.f, 1.f, 1.f)
    , grid_sbuf(nullptr)
{
    setName(TEXT_DEFAULT_NAME_BOX.toStdWString());
    addGenericMeshInstance();
    Data::marker_icon_ = scs::MarkerIcon::Box;
}

BoxNode::BoxNode(const BoxNode& node)
    : SimpleObjectNode(node)
    , grid_need_update(true)
    , grid_type(GridType::NoGrid)
    , grid_division(1.f, 1.f, 1.f)
    , grid_sbuf(nullptr)
{
    addGenericMeshInstance();
}

BoxNode::~BoxNode()
{
    MeshManager::getInstance().removeGridMesh(grid_sbuf);
    grid_sbuf.reset();
}

void BoxNode::setClippingMode(ClippingMode mode)
{
    m_clippingMode = isSimpleBox() ? mode : ClippingMode::showInterior;
}

ElementType BoxNode::getType() const
{
    return ElementType::Box;
}

TreeType BoxNode::getDefaultTreeType() const
{
    return TreeType::Boxes;
}

void BoxNode::pushClippingGeometries(ClippingAssembly& clipAssembly, const TransformationModule& transfo) const
{
    glm::vec4 params;
    params.x = m_scale.x;
    params.y = m_scale.y;
    params.z = m_scale.z;

    std::shared_ptr<IClippingGeometry> geom = std::make_shared<BoxClippingGeometry>(getClippingMode(), transfo.getInverseRotationTranslation(), params, 0);
    geom->isSelected = m_selected;

    if (m_clippingMode == ClippingMode::showInterior)
        clipAssembly.clippingUnion.push_back(geom);
    else
        clipAssembly.clippingIntersection.push_back(geom);
}

void BoxNode::pushRampGeometries(std::vector<std::shared_ptr<IClippingGeometry>>& retGeom, const TransformationModule& transfo) const
{
    glm::vec4 params;
    params.x = m_rampClamped ? m_scale.x : INFINITY;
    params.y = m_rampClamped ? m_scale.y : INFINITY;
    params.z = m_scale.z;

    auto geom = std::make_shared<BoxClippingGeometry>(ClippingMode::showInterior, transfo.getInverseRotationTranslation(), params, m_rampSteps);
    geom->color = getColor().toVector();
    geom->isSelected = m_selected;
    retGeom.push_back(geom);
}

void BoxNode::addGenericMeshInstance()
{
    MeshManager::getInstance().getBoxId(m_meshId);
}

std::unordered_set<Selection> BoxNode::getAcceptableSelections(ManipulationMode mode) const
{
    switch (mode)
    {
    case ManipulationMode::Translation:
    case ManipulationMode::Rotation:
        return { Selection::X, Selection::Y, Selection::Z };
    case ManipulationMode::Extrusion:
        return { Selection::X, Selection::Y, Selection::Z, Selection::_X, Selection::_Y, Selection::_Z };
    case ManipulationMode::Scale:
    default:
        return {};
    }
}

void BoxNode::setIsSimpleBox(bool simpleBox)
{
    grid_need_update = true;
    grid_type = simpleBox ? GridType::NoGrid : GridType::ByMultiple;
    Data::marker_icon_ = simpleBox ? scs::MarkerIcon::Box : scs::MarkerIcon::Grid;
}

void BoxNode::setGridType(GridType type)
{
    grid_need_update = (grid_type != type);
    grid_type = type;
    Data::marker_icon_ = isSimpleBox() ? scs::MarkerIcon::Box : scs::MarkerIcon::Grid;
}

void BoxNode::setGridDivision(const glm::vec3& division)
{
    grid_need_update = (grid_division != division);
    grid_division = division;
}

bool BoxNode::isSimpleBox() const
{
    return (grid_type == GridType::NoGrid);
}

GridType BoxNode::getGridType() const
{
    return grid_type;
}
const glm::vec3& BoxNode::getGridDivision() const
{
    return grid_division;
}

MeshDrawData BoxNode::getGridMeshDrawData(const glm::dmat4& gTransfo)
{
    MeshDrawData meshDrawData = AGraphNode::getMeshDrawData(gTransfo);
    updateGrid();
    meshDrawData.meshBuffer = grid_sbuf;
    return meshDrawData;
}

void BoxNode::updateGrid()
{
    if (!grid_need_update)
        return;

    MeshManager::getInstance().removeGridMesh(grid_sbuf);
    grid_sbuf = std::make_shared<MeshBuffer>();

    switch (grid_type) {
    case GridType::ByStep:
        GridCalculation::allocGridMeshByStep(*grid_sbuf, *this, grid_division);
        break;
    case GridType::ByMultiple:
        GridCalculation::allocGridMeshByMultiple(*grid_sbuf, *this, grid_division);
        break;
    case GridType::NoGrid:
        grid_sbuf.reset();
        break;
    }

    grid_need_update = false;
}
