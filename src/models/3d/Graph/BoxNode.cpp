#include "models/3d/Graph/BoxNode.h"
#include "models/3d/ManipulationTypes.h"
#include "models/3d/GridCalculation.h"
#include "vulkan/MeshManager.h"

BoxNode::BoxNode(const BoxNode& node)
    : SimpleObjectNode(node)
    , GridData(node)
    , m_isSimpleBox(node.m_isSimpleBox)
    , m_gridSBuf(nullptr)
{
    addGenericMeshInstance();
}

BoxNode::BoxNode(bool isSimpleBox)
    : SimpleObjectNode()
    , m_isSimpleBox(isSimpleBox)
    , m_gridSBuf(nullptr)
{
    setName(TEXT_DEFAULT_NAME_BOX.toStdWString());
    addGenericMeshInstance();
}

BoxNode::~BoxNode()
{
    MeshManager::getInstance().removeGridMesh(m_gridSBuf);
    m_gridSBuf.reset();
}

ElementType BoxNode::getType() const
{
    return m_isSimpleBox ? ElementType::Box : ElementType::Grid;
}

TreeType BoxNode::getDefaultTreeType() const
{
    return TreeType::Boxes;
}

ClippingMode BoxNode::getClippingMode() const
{
    return m_isSimpleBox ? m_clippingMode : ClippingMode::showInterior;
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

std::unordered_set<Selection> BoxNode::getAcceptableSelections(const ManipulationMode& mode) const
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

std::unordered_set<ManipulationMode> BoxNode::getAcceptableManipulationModes() const
{
    return { ManipulationMode::Translation,	ManipulationMode::Rotation, ManipulationMode::Scale, ManipulationMode::Extrusion };
}

void BoxNode::setIsSimpleBox(bool isSimpleBox)
{
    m_isSimpleBox = isSimpleBox;
}

bool BoxNode::isSimpleBox() const
{
    return m_isSimpleBox;
}

std::shared_ptr<MeshBuffer> BoxNode::getGridBuffer()
{
    if (m_isSimpleBox)
        return nullptr;

    // TODO : mettre à jour à un autre moment ?
    updateGrid();

    if (m_gridAllocationSucces)
        return m_gridSBuf;
    else
        return std::shared_ptr<MeshBuffer>();
}

void BoxNode::setNeedUpdate(bool needUpdate)
{
    m_gridNeedUpdate = needUpdate;
}

MeshDrawData BoxNode::getGridMeshDrawData(const glm::dmat4& gTransfo)
{
    MeshDrawData meshDrawData = AObjectNode::getMeshDrawData(gTransfo);
    meshDrawData.meshBuffer = getGridBuffer();
    return meshDrawData;
}

void BoxNode::updateGrid()
{
    MeshManager::getInstance().removeGridMesh(m_gridSBuf);
    m_gridSBuf = std::make_shared<MeshBuffer>();

    if (m_gridType == GridType::ByStep)
        m_gridAllocationSucces = GridCalculation::allocGridMeshByStep(*m_gridSBuf, *this, m_gridDivision);
    else
        m_gridAllocationSucces = GridCalculation::allocGridMeshByMultiple(*m_gridSBuf, *this, m_gridDivision);

    m_gridNeedUpdate = false;
}