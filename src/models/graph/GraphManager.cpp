#include "models/graph/GraphManager.hxx"
#include "vulkan/VulkanManager.h"
#include "gui/IDataDispatcher.h"
#include "gui/GuiData/GuiData3dObjects.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataRendering.h"

#include "models/graph/PointCloudNode.h"
#include "models/graph/TagNode.h"
#include "models/graph/TorusNode.h"
#include "models/graph/MeshObjectNode.h"
#include "models/graph/ClusterNode.h"
#include "models/graph/BoxNode.h"
#include "models/graph/CameraNode.h"
#include "models/graph/TargetNode.h"
#include "models/project/ProjectTypes.h"
#include "models/FilteredTypes.h"

#include "gui/texts/TreePanelTexts.hpp"

#include "pointCloudEngine/PCE_core.h"

GraphManager::GraphManager(IDataDispatcher& dataDispatcher)
    : m_root(make_safe<AGraphNode>())
    , m_meshManager(&MeshManager::getInstance())
    , m_dataDispatcher(dataDispatcher)
{
    WritePtr<AGraphNode> w_root = m_root.get();
    if (w_root)	w_root->setName(L"I_Am_Root");
    // Uncomment this code to test a root different than zero.
    //WritePtr<ReferentialNode> wRoot = m_root.get();
    //wRoot->setPosition(glm::dvec3(0.0, 5.0, 0.0));
    //wRoot->setRotation(glm::dquat(-0.0012293305, 0.0038679366, 0.688302797, 0.725412149));

    registerGuiDataFunction(guiDType::activatedFunctions, &GraphManager::onFunctionChanged);
    registerGuiDataFunction(guiDType::renderTargetClick, &GraphManager::onClick);
    registerGuiDataFunction(guiDType::manipulatorMode, &GraphManager::onManipulationMode);
}

GraphManager::~GraphManager()
{
    cleanProjectObjects();
}

void GraphManager::registerGuiDataFunction(guiDType type, GraphManagerMethod fct)
{
    m_dataDispatcher.registerObserverOnKey(this, type);
    m_methods.insert({ type, fct });
};

void GraphManager::informData(IGuiData* data)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_methods.find(data->getType()) != m_methods.end())
    {
        GraphManagerMethod method = m_methods.at(data->getType());
        (this->*method)(data, true);
    }
}

SafePtr<AGraphNode> GraphManager::getRoot()
{
    return m_root;
}

const SafePtr<ManipulatorNode>& GraphManager::getManipulatorNode()
{
    return m_manipulatorNode;
}

const SafePtr<ClusterNode>& GraphManager::getHierarchyMasterCluster() const
{
    return m_hierarchyMasterCluster;
}

void GraphManager::setHierarchyMasterCluster(const SafePtr<ClusterNode>& hierarchy)
{
    m_hierarchyMasterCluster = hierarchy;
}

void GraphManager::createHierarchyMasterCluster()
{
    SafePtr<ClusterNode> hierarchyMasterCluster = make_safe<ClusterNode>();
    {
        WritePtr<ClusterNode> wHMC = hierarchyMasterCluster.get();
        wHMC->setTreeType(TreeType::Hierarchy);
        wHMC->setName(TEXT_HIERARCHY_TREE_NODE.toStdWString());
        wHMC->m_isMasterCluster = true;
    }

    setHierarchyMasterCluster(hierarchyMasterCluster);
}

void GraphManager::refreshScene()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    m_meshManager->emptyTrash();

    for (IGuiData* data : m_waitingDataToProceed)
    {
        if (m_methods.find(data->getType()) != m_methods.end())
        {
            GraphManagerMethod method = m_methods.at(data->getType());
            (this->*method)(data, false);
        }
        delete data;
    }
    m_waitingDataToProceed.clear();
}

void GraphManager::cleanProjectObjects()
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_temperatureScaleData = TemperatureScaleData{};
    }

    AGraphNode::cleanLinks(m_manipulatorNode);
    m_manipulatorNode.destroy();

    AGraphNode::cleanLinks(m_hierarchyMasterCluster);
    m_hierarchyMasterCluster.destroy();

    for (SafePtr<AGraphNode> projectNode : getProjectNodes())
    {
        AGraphNode::cleanLinks(projectNode);
        projectNode.destroy();
    }

    AGraphNode::cleanLinks(m_root);

    // We keep the camera but we need to reset the examine point
    {
        WritePtr<CameraNode> wCam = m_cameraNode.get();
        if (wCam)
            wCam->setExaminePoint(glm::dvec3(NAN, NAN, NAN));
    }

    // Note(aurÃ©lien) on tente le nettoyage des geometries gÃ©nÃ©rÃ©es Ã  la volÃ©.
    VulkanManager::getInstance().waitIdle();
    m_meshManager->cleanMemory(true);
    //m_meshManager->cleanSimpleGeometryMemory();
}

void GraphManager::setTemperatureScaleData(const TemperatureScaleData& data)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_temperatureScaleData = data;
}

TemperatureScaleData GraphManager::getTemperatureScaleData() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_temperatureScaleData;
}

void GraphManager::deleteClickTargets()
{
    std::unordered_set<SafePtr<AGraphNode>> targets = getNodesByTypes({ ElementType::Target });
    for (SafePtr<AGraphNode> target : targets)
    {
        AGraphNode::cleanLinks(target);
        target.destroy();
    }
}

// Is this function synchrone or asynchrone ?
void GraphManager::setObjectsHovered(std::unordered_set<uint32_t>&& objectsHovered)
{
    resetObjectsHovered();

    std::unordered_set<SafePtr<AGraphNode>> nodesHovered = getNodesFromGraphicIds(objectsHovered);
    for (const SafePtr<AGraphNode>& node : nodesHovered)
    {
        WritePtr<AGraphNode> wNode = node.get();
        if (!wNode)
            continue;

        wNode->setHovered(true);
        m_objectsHovered.insert(node);
    }
}

SafePtr<AGraphNode> GraphManager::getSingleHoverObject() const
{
    if (m_objectsHovered.size() == 1)
        return (*m_objectsHovered.begin());
    else
        return SafePtr<AGraphNode>();
}

void GraphManager::resetObjectsHovered()
{
    for (const SafePtr<AGraphNode>& object : m_objectsHovered)
    {
        WritePtr<AGraphNode> wObj = object.get();
        if (!wObj)
            continue;

        wObj->setHovered(false);
    }
    m_objectsHovered.clear();
}

void GraphManager::onFunctionChanged(IGuiData* iGuiData, bool store)
{
    auto examineData = static_cast<GuiDataActivatedFunctions*>(iGuiData);

    if (store)
    {
        m_waitingDataToProceed.push_back(new GuiDataActivatedFunctions(*examineData));
        return;
    }
    if (examineData->type != ContextType::none)
    {
        deleteClickTargets();
    }
}

void GraphManager::onClick(IGuiData* iGuiData, bool store)
{
    auto targetData = static_cast<GuiDataRenderTargetClick*>(iGuiData);

    if (store)
    {
        m_waitingDataToProceed.push_back(new GuiDataRenderTargetClick(*targetData));
        return;
    }

    if (targetData->m_reset)
    {
        deleteClickTargets();
    }
    else if (!isnan(targetData->m_position.x))
    {
        // Create new target
        SafePtr<TargetNode> target = make_safe<TargetNode>(targetData->m_position, targetData->m_color);
        AGraphNode::addGeometricLink(m_root, target);
    }
}

void GraphManager::onManipulationMode(IGuiData* data, bool store)
{
    WritePtr<ManipulatorNode> wManip = m_manipulatorNode.get();
    if (!wManip)
        return;

    m_manipulationMode = static_cast<GuiDataManipulatorMode*>(data)->m_mode;
    wManip->setManipulationMode(m_manipulationMode);
}

void GraphManager::addNodesToGraph(const std::unordered_set<SafePtr<AGraphNode>>& nodes)
{
    for(const SafePtr<AGraphNode>& node : nodes)
        AGraphNode::addGeometricLink(m_root, node);
}

SafePtr<CameraNode> GraphManager::createCameraNode(const std::wstring& name)
{
    SafePtr<CameraNode> cameraNode = make_safe<CameraNode>(name, m_dataDispatcher);
    m_cameraNode = cameraNode;
    return cameraNode;
}

SafePtr<CameraNode> GraphManager::duplicateCamera(const SafePtr<CameraNode>& camera)
{
    ReadPtr<CameraNode> rcam = camera.cget();
    if (rcam)
        return make_safe<CameraNode>(*&rcam);
    else
        return SafePtr<CameraNode>();
}

void GraphManager::detachManipulator(AGraphNode* parent)
{
    /*
    if (parent->isGeometricChild(m_manipulator->getId()))
        ManipulatorNode::cancel(m_manipulator);
    */
}

std::unordered_set<SafePtr<AGraphNode>> GraphManager::getNodesFromGraphicIds(const std::unordered_set<uint32_t>& graphicIds) const
{
    return getNodesOnFilter<AGraphNode>(
        [&graphicIds](ReadPtr<AGraphNode>& node) {
            return graphicIds.find(node->getGraphicId()) != graphicIds.end();
        }
    );
}

SafePtr<AGraphNode> GraphManager::getNodeFromGraphicId(uint32_t graphicId) const
{
    std::unordered_set<SafePtr<AGraphNode>> ret = getNodesOnFilter<AGraphNode>(
        [&graphicId](ReadPtr<AGraphNode>& node) {
            return node->getGraphicId() == graphicId;
        }
    );

    if (ret.empty())
        return SafePtr<AGraphNode>();

    assert(ret.size() == 1 && "Same xg::Guid node");
    return (*ret.begin());
}

void GraphManager::getMeshInfoForClick(ClickInfo& click)
{
    click.mesh = nullptr;
    click.meshTransfo = glm::dmat4();

    glm::dmat4 meshTransfo;
    ElementType type;
    {
        ReadPtr<AGraphNode> node = click.hover.cget();
        if (!node)
            return;
        type = node->getType();
        meshTransfo = node->getTransformation();
    }

    if (type == ElementType::Box ||
        type == ElementType::Cylinder ||
        type == ElementType::Sphere)
    {
        ReadPtr<SimpleObjectNode> rSimple = static_pointer_cast<SimpleObjectNode>(click.hover).cget();
        if (!rSimple)
            return;
        click.mesh = m_meshManager->getGenericMesh(rSimple->getGenericMeshId());
    }

    if (type == ElementType::Torus)
    {
        ReadPtr<TorusNode> rTorus = static_pointer_cast<TorusNode>(click.hover).cget();
        if (!rTorus)
            return;
        click.mesh = m_meshManager->getMesh(rTorus->getMeshId()).m_mesh;
    }

    if (type == ElementType::MeshObject)
    {
        ReadPtr<MeshObjectNode> rMesh = static_pointer_cast<MeshObjectNode>(click.hover).cget();
        if (!rMesh)
            return;
        click.mesh = m_meshManager->getMesh(rMesh->getMeshId()).m_mesh;
    }

    click.meshTransfo = meshTransfo;
}

bool isObjectFiltered(ReadPtr<AGraphNode>& rNode, ObjectStatusFilter filter)
{
    switch (filter)
    {
        case ObjectStatusFilter::ALL:
        {
            return true;
        }
        case ObjectStatusFilter::VISIBLE:
        {
            return rNode->isVisible();
        }
        case ObjectStatusFilter::SELECTED:
        {
            return rNode->isSelected();
        }
        case ObjectStatusFilter::NONE:
        {
            return false;
        }
        default:
        {
            return false;
        }
    }
}

/*** From Project ****/

uint32_t GraphManager::getNextUserId(ElementType type, const IndexationMethod& indexMethod) const
{
    uint32_t i = 1;

    std::set<uint32_t> allIndex;
    if (indexMethod == IndexationMethod::FillMissingIndex)
    {
        for (const SafePtr<AGraphNode>& node : getNodesByTypes({ type }, ObjectStatusFilter::ALL))
        {
            ReadPtr<AGraphNode> readNode = node.cget();
            if (!readNode)
                continue;
            allIndex.insert(readNode->getUserIndex());
        }
        while (true)
        {
            if (allIndex.find(i) == allIndex.end())
                break;
            else
                ++i;
        }
    }
    else
    {
        for (const SafePtr<AGraphNode>& node : getNodesByTypes({ type }, ObjectStatusFilter::ALL))
        {
            
            ReadPtr<AGraphNode> readNode = node.cget();

            if (!readNode)
                continue;
            if(readNode->getUserIndex() >= i)
                i = readNode->getUserIndex() + 1;
        }
    }

    return (i);
}

std::vector<uint32_t> GraphManager::getNextMultipleUserId(ElementType type, const IndexationMethod& indexMethod, int indexAmount) const
{
    std::vector<uint32_t> result(0);
    uint32_t i = 1;
    int indexAdded(0);
    std::set<uint32_t> allIndex;
    if (indexMethod == IndexationMethod::FillMissingIndex)
    {
        for (const SafePtr<AGraphNode>& node : getNodesByTypes({ type }, ObjectStatusFilter::ALL))
        {
            ReadPtr<AGraphNode> readNode = node.cget();
            if (!readNode)
                continue;
            allIndex.insert(readNode->getUserIndex());
        }
        while (indexAdded < indexAmount)
        {
            while (true)
            {
                if (allIndex.find(i) == allIndex.end())
                    break;
                else
                    ++i;
            }
            result.push_back(i);
            allIndex.insert(i);
            indexAdded++;
        }
        
    }
    else
    {
        for (const SafePtr<AGraphNode>& node : getNodesByTypes({ type }, ObjectStatusFilter::ALL))
        {

            ReadPtr<AGraphNode> readNode = node.cget();

            if (!readNode)
                continue;
            if (readNode->getUserIndex() >= i)
                i = readNode->getUserIndex() + 1;
        }
        for (int j = 0; j < indexAmount; j++)
            result.push_back(j+i);
    }


    return result;
}

bool GraphManager::isIdAvailable(const std::unordered_set<ElementType>& types, uint32_t userid) const
{
    for (const SafePtr<AGraphNode>& node : getNodesByTypes(types, ObjectStatusFilter::ALL))
    {
        ReadPtr<AGraphNode> readNode = node.cget();
        if (readNode && readNode->getUserIndex() == userid)
            return false;
    }
    return true;
}

SafePtr<CameraNode> GraphManager::getCameraNode() const
{
    return m_cameraNode;
}

std::unordered_set<SafePtr<AGraphNode>> GraphManager::getProjectNodes() const
{
    return getProjectNodesByFilterType(ObjectStatusFilter::ALL);
}

SafePtr<AGraphNode> GraphManager::getNodeById(xg::Guid id) const
{
    std::unordered_set<SafePtr<AGraphNode>> sameIdNodes = getNodesOnFilter<AGraphNode>([&id](ReadPtr<AGraphNode>& node) { return node->getId() == id; });
    if (sameIdNodes.empty())
        return SafePtr<AGraphNode>();
    if (sameIdNodes.size() > 1)
        assert(!"Deux fois le mÃªme noeud avec le mÃªme id dans le graph");

    return *(sameIdNodes.begin());
}

std::unordered_set<SafePtr<AGraphNode>> GraphManager::getSelectedNodes() const
{
    return getNodesOnFilter<AGraphNode>([](ReadPtr<AGraphNode>& node) { return node->isSelected(); });
}

std::unordered_set<SafePtr<AGraphNode>> GraphManager::getUnSelectedNodes() const
{
    return getNodesOnFilter<AGraphNode>([](ReadPtr<AGraphNode>& node) { return !node->isSelected(); });
}

std::unordered_set<SafePtr<AGraphNode>> GraphManager::getProjectNodesByFilterType(ObjectStatusFilter filter) const
{
    return getNodesOnFilter<AGraphNode>([&filter](ReadPtr<AGraphNode>& node) {return isObjectFiltered(node, filter) && node->getGraphType() == AGraphNode::Type::Default; });
}

std::unordered_set<SafePtr<AGraphNode>> GraphManager::getNodesByTypes(std::unordered_set<ElementType> types, ObjectStatusFilter filter) const
{
    return getNodesOnFilter<AGraphNode>([&types, &filter](ReadPtr<AGraphNode>& node) { return isObjectFiltered(node, filter) && types.find(node->getType()) != types.end(); });
}

std::unordered_set<SafePtr<AClippingNode>> GraphManager::getClippingObjects(bool filterActive, bool filterSelected) const
{
    return getNodesOnFilter<AClippingNode>(
        [](ReadPtr<AGraphNode>& node)
        { return s_clippingTypes.find(node->getType()) != s_clippingTypes.end(); },

        [filterActive, filterSelected](ReadPtr<AClippingNode>& clip)
        // clipping status for viewpoints issue correction
        {
            bool activeOk = !filterActive || clip->isClippingActive();
            bool selectedOk = !filterSelected || clip->isSelected();

            return activeOk && selectedOk;
        });
}

std::unordered_set<SafePtr<AClippingNode>> GraphManager::getRampObjects(bool filterActive, bool filterSelected) const
{
    return getNodesOnFilter<AClippingNode>(
        [](ReadPtr<AGraphNode>& node)
    { return s_clippingTypes.find(node->getType()) != s_clippingTypes.end(); },
        [filterActive, filterSelected](ReadPtr<AClippingNode>& clip)
		// ramp status for viewpoints issue correction
        {
            bool activeOk = !filterActive || clip->isRampActive();
            bool selectedOk = !filterSelected || clip->isSelected();

            return activeOk && selectedOk;
        });
}

std::unordered_set<SafePtr<AClippingNode>> GraphManager::getActivatedOrSelectedClippingObjects() const
{
    return getNodesOnFilter<AClippingNode>(
        [](ReadPtr<AGraphNode>& node)
        { return s_clippingTypes.find(node->getType()) != s_clippingTypes.end(); },

        [](ReadPtr<AClippingNode>& clip)
        {return (clip->isClippingActive() || clip->isSelected()); });
}

std::unordered_set<SafePtr<BoxNode>> GraphManager::getGrids() const
{
    return getNodesOnFilter<BoxNode>(
        [](ReadPtr<AGraphNode>& node)
        {
            if (node->getType() != ElementType::Box)
                return false;
            return !((const BoxNode*)(&node))->isSimpleBox();
        });
}

uint32_t GraphManager::getActiveClippingCount() const
{
    return (uint32_t)getClippingObjects(true, false).size();
}

uint32_t GraphManager::getActiveClippingAndRampCount() const
{
    uint32_t count = 0;
    getNodesOnFilter<AClippingNode>(
        [](ReadPtr<AGraphNode>& node)
        { return s_clippingTypes.find(node->getType()) != s_clippingTypes.end(); },

        [&count](ReadPtr<AClippingNode>& clip)
        {
            if (clip->isClippingActive())
                count++;
            if (clip->isRampActive())
                count++;
            return false; 
        });
    return count;
}

uint32_t GraphManager::getPCOcounters(const tls::ScanGuid& scan) const
{
    return (uint32_t)getNodesOnFilter<PointCloudNode>(
            [](ReadPtr<AGraphNode>& node)
            { return node->getType() == ElementType::PCO || node->getType() == ElementType::Scan; },
            [&scan](ReadPtr<PointCloudNode>& node)
            { return node->getScanGuid() == scan; }
        ).size();
}

std::unordered_set<SafePtr<TagNode>> GraphManager::getTagsWithTemplate(SafePtr<sma::TagTemplate> tagTemplate) const
{
    return getNodesOnFilter<TagNode>(
        [](ReadPtr<AGraphNode>& node)
        { return node->getType() == ElementType::Tag; },

        [&tagTemplate](ReadPtr<TagNode>& tagNode)
        {return tagNode->getTemplate() == tagTemplate; });
}

bool GraphManager::isFilePathOrScanExists(const std::wstring& name, const std::filesystem::path& filePath) const
{
    //A remplacer avec un scanRootNode ?
    for (const SafePtr<AGraphNode>& objectPtr : AGraphNode::getGeometricChildren(m_root)) //A utiliser autre chose que le lien gÃ©ometrique
    {
        ElementType type;
        {
            ReadPtr<AGraphNode> rObject = objectPtr.cget();
            if (!rObject)
                continue;
            type = rObject->getType();
        }

        if (type == ElementType::Scan)
        {
            ReadPtr<PointCloudNode> readScan = static_pointer_cast<PointCloudNode>(objectPtr).cget();
            if (readScan && (readScan->getName() == name || readScan->getTlsFilePath() == filePath))
                return (true);
        }
    }
    return (false);
}

void GraphManager::replaceObjectsSelected(std::unordered_set<SafePtr<AGraphNode>> toSelectDatas)
{
    for (const SafePtr<AGraphNode>& nodePtr : getSelectedNodes())
    {
        WritePtr<AGraphNode> writeNode = nodePtr.get();
        if(writeNode)
            writeNode->setSelected(false);
    }

    for (const SafePtr<AGraphNode>& toSelectData : toSelectDatas)
    {
        WritePtr<AGraphNode> writeNode = toSelectData.get();
        if (writeNode)
            writeNode->setSelected(true);
    }

    AGraphNode::cleanLinks(m_manipulatorNode);
    m_manipulatorNode.reset();
    m_manipulatorNode = make_safe<ManipulatorNode>(m_dataDispatcher);
    bool is_manipulator_empty = false;
    {
        WritePtr<ManipulatorNode> wManip = m_manipulatorNode.get();
        wManip->setManipulationMode(m_manipulationMode);
        // The ManipulatorNode sorts out itself the nodes that can be manipulated
        wManip->setTargets(toSelectDatas);
        is_manipulator_empty = !wManip->hasTargets();
    }

    if (is_manipulator_empty)
        m_manipulatorNode.reset();
    else
        addNodesToGraph({ m_manipulatorNode });
}

void GraphManager::getClippingAssembly(ClippingAssembly& retAssembly, bool filterActive, bool filterSelected) const
{
    std::unordered_set<SafePtr<AClippingNode>> clippings = getClippingObjects(filterActive, filterSelected);
    getClippingAssembly(retAssembly, clippings);
}

void GraphManager::getClippingAssembly(ClippingAssembly& retAssembly, const std::unordered_set<SafePtr<AClippingNode>>& clippings) const
{
    for (const SafePtr<AClippingNode>& clip : clippings)
    {
        ReadPtr<AClippingNode> rClip = clip.cget();
        if (!rClip)
            continue;

        // NOTE - Il a Ã©tÃ© dÃ©crÃ©tÃ© que l'on doit faire l'union des clipping intÃ©rieur 
        //        et l'intersection des clipping extÃ©rieur.
        //        Bien sÃ»r d'autres assemblages logiques sont possibles mais il faudrait pour cela
        //        crÃ©er les opÃ©rateurs adÃ©quats et rendre cela lisible pour l'utilisateur.
        rClip->pushClippingGeometries(retAssembly, TransformationModule(*&rClip));
    }
}

BoundingBoxD GraphManager::getScanBoundingBox(ObjectStatusFilter status) const
{
    auto pc_instances = getPointCloudInstances(xg::Guid(), true, true, status);

    BoundingBoxD project_bbox;
    project_bbox.setEmpty();

    for (const tls::PointCloudInstance inst : pc_instances)
    {
        const tls::Limits& limits = inst.header.limits;
        BoundingBoxD scan_bbox{ limits.xMin, limits.xMax,
                                limits.yMin, limits.yMax,
                                limits.zMin, limits.zMax };

        project_bbox.extend(scan_bbox.transform(inst.transfo.getTransformation()));
    }
    return project_bbox;
}

std::unordered_set<SafePtr<PointCloudNode>> GraphManager::getVisibleScans(const tls::ScanGuid& pano) const
{
    return getNodesOnFilter<PointCloudNode>(
        [](ReadPtr<AGraphNode>& node) {
            return node->getType() == ElementType::Scan; },
        [&pano](ReadPtr<PointCloudNode>& scan) {
            return ((pano == xg::Guid() && scan->isVisible()) || scan->getScanGuid() == pano); }
            );
}

std::vector<tls::PointCloudInstance> GraphManager::getVisiblePointCloudInstances(const tls::ScanGuid& pano, bool scans, bool pcos) const
{
    return getPointCloudInstances(pano, scans, pcos, ObjectStatusFilter::VISIBLE);
}

std::vector<tls::PointCloudInstance> GraphManager::getPointCloudInstances(const tls::ScanGuid& pano, bool getScans, bool getPcos, ObjectStatusFilter filterStatus) const
{
    std::vector<tls::PointCloudInstance> result;

    if (pano != xg::Guid())
    {
        std::unordered_set<SafePtr<PointCloudNode>> scans = 
            getNodesOnFilter<PointCloudNode>(
                [&pano, &getScans, &getPcos](ReadPtr<AGraphNode>& node)
                {
                    return node->getType() == ElementType::Scan;
                },
                [&pano](ReadPtr<PointCloudNode>& node)
                {
                    return (pano != xg::Guid() && node->getScanGuid() == pano);
                }
            );

        if (scans.size() > 1)
            assert(false);

        SafePtr<PointCloudNode> scan = *scans.begin();
        ReadPtr<PointCloudNode> rScan = scan.cget();
        if (!rScan)
            return result;

        tls::ScanHeader header;
        tlGetScanHeader(rScan->getScanGuid(), header);
        result.push_back(tls::PointCloudInstance{ header, *&rScan, rScan->getClippable(), rScan->getPhase() });

        // QUESTION - Est-ce que le scan panoramique annule l'export des pcos ?
        return result;
    }

    std::unordered_set<SafePtr<PointCloudNode>> pcs =
        getNodesOnFilter<PointCloudNode>(
            [&getScans, &getPcos, &filterStatus](ReadPtr<AGraphNode>& node)
            { 
                bool verifType = getScans && node->getType() == ElementType::Scan
                    || getPcos && node->getType() == ElementType::PCO;
                bool verifState = (filterStatus == ObjectStatusFilter::ALL ||
                    (filterStatus == ObjectStatusFilter::VISIBLE && node->isVisible()) ||
                    (filterStatus == ObjectStatusFilter::SELECTED && node->isSelected()));
                return verifType && verifState;
            }
        );

    for (const SafePtr<PointCloudNode>& pc : pcs)
    {
        ReadPtr<PointCloudNode> rPc = pc.cget();
        if (!rPc)
            continue;

        tls::ScanHeader header;
        if (!tlGetScanHeader(rPc->getScanGuid(), header))
            continue;

        header.name = rPc->getComposedName();
        result.push_back(tls::PointCloudInstance{ header, rPc->getTransformationModule(), rPc->getClippable(), rPc->getPhase() });
    }

    return (result);
}

uint64_t GraphManager::getProjectPointsCount() const
{
    uint64_t points = 0;
    for (const SafePtr<AGraphNode>& scan : getNodesByTypes({ ElementType::Scan }, ObjectStatusFilter::ALL))
    {
        ReadPtr<PointCloudNode> rScan = static_pointer_cast<PointCloudNode>(scan).cget();
        if (!rScan)
            continue;

        points += rScan->getNbPoint();
    }
    return (points);
}

std::unordered_set<SafePtr<AGraphNode>> GraphManager::getNodesOnFilter(std::function<bool(const SafePtr<AGraphNode>&)> graphNodeFilter) const
{
    std::unordered_set<SafePtr<AGraphNode>> nodes;

    std::unordered_set<SafePtr<AGraphNode>> geoChildren = AGraphNode::getGeometricChildren_rec(m_root);

    for (const SafePtr<AGraphNode>& graphNode : geoChildren)
    {
        if (!graphNodeFilter(graphNode))
            continue;
        nodes.insert(graphNode);
    }
    return (nodes);
}
