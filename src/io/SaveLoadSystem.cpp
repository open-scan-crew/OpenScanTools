#include "io/SaveLoadSystem.h"

#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/IControlListener.h"
#include "controller/controls/ControlFunction.h"

#include "io/exports/DataSerializer.h"
#include "io/imports/DataDeserializer.h"
#include "pointCloudEngine/PCE_core.h"

#include "utils/Config.h"
#include "utils/Logger.h"
#include "utils/Utils.h"
#include "utils/system.h"
#include "utils/JsonWriter.h"
#include "utils/safe_ptr.h"
#include "utils/OpenScanToolsVersion.h"
#include "utils/FilesAndFoldersDefinitions.h"

#include "vulkan/MeshManager.h"
#include "vulkan/Graph/MemoryReturnCode.h"
#include "gui/GuiData/GuiDataUserOrientation.h"
#include "gui/GuiData/GuiData3dObjects.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/Texts.hpp"

// Model
#include "models/graph/GraphManager.h"
#include "models/graph/PointCloudNode.h"
#include "models/graph/SphereNode.h"
#include "models/graph/TagNode.h"
#include "models/graph/PointNode.h"
#include "models/graph/MeshObjectNode.h"
#include "models/graph/ClusterNode.h"
#include "models/graph/BoxNode.h"
#include "models/graph/CylinderNode.h"
#include "models/graph/TorusNode.h"
#include "models/graph/BeamBendingMeasureNode.h"
#include "models/graph/ColumnTiltMeasureNode.h"
#include "models/graph/ViewPointNode.h"
#include "models/graph/SimpleMeasureNode.h"
#include "models/graph/PolylineMeasureNode.h"
#include "models/graph/PointToPipeMeasureNode.h"
#include "models/graph/PointToPlaneMeasureNode.h"
#include "models/graph/PipeToPipeMeasureNode.h"
#include "models/graph/PipeToPlaneMeasureNode.h"
#include "models/graph/CameraNode.h"
#include "models/application/Author.h"
#include "models/application/Ids.hpp"
#include "models/application/List.h"


// External libs
#include "magic_enum/magic_enum.hpp"
#include <nlohmannJson/json.hpp>

// Standard libs
#include <set>
#include <filesystem>
#include <fstream>
#include <algorithm>

#define SAVELOADSYSTEMVERSION 2.0f

static const std::unordered_map<SaveLoadSystem::ObjectsFileType, std::pair<std::string, std::string>> FilesExtensionArray = 
{ {SaveLoadSystem::ObjectsFileType::Tlo, {File_Extension_Objects, Key_Objects}}
, {SaveLoadSystem::ObjectsFileType::Tld, {File_Extension_Tags, Key_Tags}}
, {SaveLoadSystem::ObjectsFileType::Tlv, {File_Extension_ViewPoints, Key_ViewPoints}}
, {SaveLoadSystem::ObjectsFileType::Tlo_Backup, {std::string(File_Extension_Objects) + File_Extension_Backup, Key_Objects}}
, {SaveLoadSystem::ObjectsFileType::Tld_Backup, {std::string(File_Extension_Tags) + File_Extension_Backup, Key_Tags}}
, {SaveLoadSystem::ObjectsFileType::Tlv_Backup, {std::string(File_Extension_ViewPoints) + File_Extension_Backup, Key_ViewPoints}}
};


std::filesystem::path getExplicitPath(const ProjectInternalInfo& project, const std::filesystem::path& file)
{
    if (file.is_absolute())
        return file;
    return project.getProjectFolderPath() / file;
}

nlohmann::json exportUserOrientationBlock(const ControllerContext& context)
{
    nlohmann::json structureObject = nlohmann::json::array();
    IOLOG << "export UserOrientation" << LOGENDL;
    for (const std::pair<userOrientationId, UserOrientation>& orientation : context.cgetUserOrientations())
        structureObject.push_back(DataSerializer::Serialize(orientation.second));
    return structureObject;
}

std::unordered_map<StandardType, std::vector<StandardList>> ImportStandards(const std::filesystem::path& filePath)
{
    std::filesystem::path importPath(filePath);
    std::unordered_map<StandardType, std::vector<StandardList>> standards;
    IOLOG << "Loading standards [" << importPath << "]" << LOGENDL;
    std::ifstream fileStream(importPath);
    nlohmann::json jsonTemplates;

    if (fileStream.good())
    {
        try
        {
            fileStream >> jsonTemplates;
        }
        catch (std::exception& e)
        {
            IOLOG << "Error : While parsing " << importPath << "\n" << e.what() << LOGENDL;
            return (standards);
        }
    }
    else
    {
        IOLOG << "Error : Cannot find " << importPath << LOGENDL;
        return (standards);
    }

    if (!DataDeserializer::DeserializeStandards(jsonTemplates, standards))
        IOLOG << "Error import standards" << LOGENDL;

    return (standards);
}

std::string importJsonTree(Controller& controller, const nlohmann::json& json, const TreeType& treeType, SafePtr<AGraphNode> parent, std::unordered_map<xg::Guid, SafePtr<AGraphNode>>& nodeById)
{
    GraphManager& graphManager = controller.getGraphManager();
    SafePtr<AGraphNode> child;
    if (json.find(Key_Type) == json.end())
        return std::string();
    auto elemType = magic_enum::enum_cast<ElementType>(json.at(Key_Type).get<std::string>());
    if (!elemType.has_value())
        return std::string();
    switch (elemType.value())
    {
        case ElementType::MasterCluster:
        {
            if (treeType == TreeType::Hierarchy)
            {
                controller.getGraphManager().createHierarchyMasterCluster();
                child = controller.getGraphManager().getHierarchyMasterCluster();
            }
        }
        break;
        case ElementType::HierarchyCluster:
        case ElementType::Cluster:
        {
            SafePtr<ClusterNode> cluster = make_safe<ClusterNode>();
            DataDeserializer::DeserializeClusterNode(cluster, json, controller);
            WritePtr<ClusterNode> wCluster = cluster.get();
            if (!wCluster)
                break;
            wCluster->setTreeType(treeType);
            nodeById[wCluster->getId()] = cluster;
            child = cluster;
            break;
        }
        case ElementType::Scan:
        {
            SafePtr<PointCloudNode> pc = make_safe<PointCloudNode>(false);
            DataDeserializer::DeserializePointCloudNode(pc, json, controller);
            // TODO - check tls file path
            WritePtr<PointCloudNode> wpc = pc.get();
            if (!wpc)
                break;
            nodeById[wpc->getId()] = pc;
            child = pc;
        }
        break;
        case ElementType::Data:
        {
            xg::Guid id;
            if (json.find(Key_Id) != json.end())
                id = xg::Guid(json.at(Key_Id).get<std::string>());
            else if (json.find(Key_InternId) != json.end())
                id = xg::Guid(json.at(Key_InternId).get<std::string>());
            else
                IOLOG << "hierarchy InternId read error" << LOGENDL;

            if(nodeById.find(id) != nodeById.end())
                child = nodeById.at(id);
        }
    }

    AGraphNode::addOwningLink(parent, child);

    switch (elemType.value())
    {
        case ElementType::Cluster:
        case ElementType::HierarchyCluster:
        case ElementType::MasterCluster:
        {
            if (json.find(Key_Children) != json.end())
            {
                std::string tmpError;
                for (const nlohmann::json& it : json.at(Key_Children))
                    if (!(tmpError = importJsonTree(controller, it, treeType, child, nodeById)).empty())
                        return (tmpError);
            }
            else
                IOLOG << "no children error" << LOGENDL;
            break;
        }
    }
    return (std::string());
}

/*
void renameRootTree(Project& project, const TreeType& type, const std::wstring& name)
{
    TreeElement* root = project.getTree(type)->root();
    if (root == nullptr)
        return;
    Data* idata = project.getDataOnId(root->getDataId());
    if (idata == nullptr)
        return;
    static_cast<Data*>(idata)->setName(name);
}
*/

bool importTreeBlock(Controller& controller, const nlohmann::json& jsonProject, const TreeType& type, std::unordered_map<xg::Guid, SafePtr<AGraphNode>>& nodeById)
{
    std::string keyTree(magic_enum::enum_name<TreeType>(type));
    IOLOG << "import " << keyTree << LOGENDL;
    keyTree += Key_Tree;
    std::string error("!");

    TlStreamLock streamLock;

    if (jsonProject.find(keyTree) != jsonProject.end())
        error = importJsonTree(controller, jsonProject.at(keyTree), type, SafePtr<AGraphNode>(), nodeById);
    //Note (Aurélien) : compatibilty check
    else
    {
        keyTree = magic_enum::enum_name<TreeType>(type);
        std::transform(keyTree.begin(), keyTree.end(), keyTree.begin(), ::tolower);
        keyTree += Key_Tree;
        if (jsonProject.find(keyTree) != jsonProject.end())
            error = importJsonTree(controller, jsonProject.at(keyTree), type, SafePtr<AGraphNode>(), nodeById);
        else if (type == TreeType::Hierarchy && jsonProject.find("userTree") != jsonProject.end())
            error = importJsonTree(controller, jsonProject.at("userTree"), type, SafePtr<AGraphNode>(), nodeById);
    }

    return error.empty();
    // NOTE - The TlStreamLock is destructed when leaving this scope and the streaming is resumed automatically.
}

void importAllTrees(Controller& controller, const nlohmann::json& jsonProject, std::unordered_map<xg::Guid, SafePtr<AGraphNode>>& nodeById)
{

    std::unordered_set<TreeType> TreeList = { TreeType::ViewPoint
                                             ,TreeType::Scan
                                             ,TreeType::Hierarchy
                                             ,TreeType::Measures
                                             ,TreeType::Tags
                                             ,TreeType::Sphere
                                             ,TreeType::Boxes
                                             ,TreeType::MeshObjects
                                             ,TreeType::Pco
                                             ,TreeType::Point
                                             ,TreeType::Pipe
                                             ,TreeType::Piping
                                            };

    for (TreeType treeType : TreeList)
        importTreeBlock(controller, jsonProject, treeType, nodeById);
}

int getNumberOfTElemTypeInProject(const Controller& controller, ElementType typeWanted)
{
    return (int)controller.cgetGraphManager().getNodesByTypes({ typeWanted }).size();
}

SafePtr<Author> LoadAuthor(Controller& controller, const nlohmann::json& json)
{
    SafePtr<Author> author;
    Author authorData;

    if (json.find(Key_Author) != json.end())
    {
        //Si ça marche pas, c'est l'ancienne version d'auteur
        if (!DataDeserializer::DeserializeAuthor(*(json.find(Key_Author)), authorData))
        {
            std::wstring authorName = Utils::from_utf8(json.at(Key_Author).get<std::string>());
            std::unordered_set<SafePtr<Author>> authors = controller.getContext().getLocalAuthors();
            for (SafePtr<Author> auth : authors)
            {
                ReadPtr<Author> rAuth = auth.cget();
                if (rAuth && rAuth->getName() == authorName)
                {
                    authorData = *&rAuth;
                    break;
                }
            }
        }
    }

    author = controller.cgetContext().createAuthor(authorData);

    return author;
}

void SaveLoadSystem::checkPointCloudPath(SafePtr<PointCloudNode> pcNode)
{
    WritePtr<PointCloudNode> wPCNode = pcNode.get();
    if (!wPCNode)
        return;
    //wPCNode->setScanPath(findPointCloudPath(wPCNode, ))
}

std::filesystem::path SaveLoadSystem::findPointCloudPath(WritePtr<PointCloudNode>& wPCNode, const ProjectInternalInfo& internalInfo, const std::filesystem::path& searchFolder)
{
    std::vector<std::filesystem::path> possiblePaths;
    possiblePaths.push_back(wPCNode->getTlsFilePath());
    possiblePaths.push_back(wPCNode->getBackupFilePath());
    possiblePaths.push_back(searchFolder / wPCNode->getBackupFilePath().filename());
    possiblePaths.push_back(internalInfo.getPointCloudFolderPath(false) / wPCNode->getBackupFilePath().filename());
    possiblePaths.push_back(internalInfo.getPointCloudFolderPath(true) / wPCNode->getBackupFilePath().filename());

    for (auto path : possiblePaths)
    {
        if (std::filesystem::exists(path))
            return path;
    }
    // If not found, we return the saved path in the node.
    // Any error will be raised later depending of the context.
    return wPCNode->getBackupFilePath();
}

std::unordered_set<SafePtr<AGraphNode>> SaveLoadSystem::LoadFileObjects(Controller& controller, const std::unordered_set<SafePtr<AGraphNode>>& fileObjects, std::filesystem::path folder, bool forceCopy)
{
    ProjectInternalInfo internalInfo = controller.getContext().cgetProjectInternalInfo();
    Utils::System::createDirectoryIfNotExist(internalInfo.getPointCloudFolderPath(false));
    Utils::System::createDirectoryIfNotExist(internalInfo.getPointCloudFolderPath(true));
    Utils::System::createDirectoryIfNotExist(internalInfo.getObjectsFilesFolderPath());

    std::unordered_set<SafePtr<AGraphNode>> failedFileImport;
    for (const SafePtr<AGraphNode>& object : fileObjects)
    {
        ElementType type;
        {
            ReadPtr<AGraphNode> rObject = object.cget();
            if (!rObject)
                continue;
            type = rObject->getType();
        }

        switch (type)
        {
        case ElementType::PCO:
        case ElementType::Scan:
        {
            WritePtr<PointCloudNode> wPCNode = static_pointer_cast<PointCloudNode>(object).get();
            if (!wPCNode)
                continue;

            std::filesystem::path pcPath = findPointCloudPath(wPCNode, internalInfo, folder);
            wPCNode->setTlsFilePath(pcPath, false);
            if (wPCNode->getScanGuid() == tls::ScanGuid())
                failedFileImport.insert(object);
            else if (forceCopy)
            {
                std::filesystem::path dstPath = internalInfo.getPointCloudFolderPath(wPCNode->getType() == ElementType::PCO) / wPCNode->getTlsFilePath().filename();;
                tlCopyScanFile(wPCNode->getScanGuid(), dstPath, true, true, false);
            }
        }
        break;
        case ElementType::MeshObject:
        {
            std::filesystem::path from;
            ObjectAllocation::ReturnCode ret;
            SafePtr<MeshObjectNode> mesh = static_pointer_cast<MeshObjectNode>(object);
            {
                WritePtr<MeshObjectNode> wMesh = mesh.get();
                if (!wMesh)
                    continue;
                if (MeshManager::getInstance().isMeshLoaded(wMesh->getMeshId()))
                    continue;
                if (folder.empty())
                    folder = internalInfo.getObjectsFilesFolderPath();
                from = folder / wMesh->getFilePath().filename();

                MeshManager& meshManager = MeshManager::getInstance();
                ret = meshManager.reloadMeshFile(*&wMesh, folder, &controller);
            }

            if (ret != ObjectAllocation::ReturnCode::Success)
            {
                failedFileImport.insert(object);
            }
            else if (forceCopy)
            {
                std::filesystem::path to = internalInfo.getObjectsFilesFolderPath() / from.filename();
                if (!std::filesystem::exists(to))
                {
                    try
                    {
                        std::filesystem::create_directories(to.parent_path());
                        std::filesystem::copy_file(from, to);
                    }
                    catch (std::exception e)
                    {
                        assert(false);
                    }
                }
            }
        }
        break;
        default:
            continue;
        }
    }

    return failedFileImport;
}

void SaveLoadSystem::ExportToProjectFileObjects(Controller& controller, const ProjectInternalInfo& exportProjectInfo, const std::unordered_set<SafePtr<AGraphNode>>& objectsToExport)
{
    const GraphManager& graphManager = controller.getGraphManager();
    const ControllerContext& context = controller.getContext();
    for (const SafePtr<AGraphNode>& object : objectsToExport)
    {
        ElementType type;
        {
            ReadPtr<AGraphNode> readObject = object.cget();
            if (!readObject)
                continue;
            type = readObject->getType();
        }
        std::filesystem::path from;
        std::filesystem::path to;
        switch (type)
        {
            case ElementType::Scan:
            case ElementType::PCO:
            {
                ReadPtr<PointCloudNode> rScan = static_pointer_cast<PointCloudNode>(object).cget();
                if (!rScan)
                    continue;
                from = rScan->getTlsFilePath();
                to = exportProjectInfo.getPointCloudFolderPath(type == ElementType::PCO) / rScan->getTlsFilePath().filename();
            }
            break;
            case ElementType::MeshObject:
            {
                ReadPtr<MeshObjectNode> rMesh = static_pointer_cast<MeshObjectNode>(object).cget();
                if (!rMesh)
                    continue;
                from = context.cgetProjectInternalInfo().getObjectsFilesFolderPath() / rMesh->getFilePath().filename();
                to = exportProjectInfo.getObjectsFilesFolderPath() / rMesh->getFilePath().filename();
            }
            break;
            default:
                continue;
        }
        if (!std::filesystem::exists(from))
            continue;

        try
        {
            std::filesystem::create_directories(to.parent_path());
            if (std::filesystem::exists(to))
                std::filesystem::remove(to);
            std::filesystem::copy_file(from, to);
        }
        catch (std::exception e)
        {
            assert(false);
        }
    }
}

void LoadObjFile(Controller& controller, std::unordered_map<SafePtr<AGraphNode>, std::pair<xg::Guid, nlohmann::json>>& loadObj, const std::filesystem::path& objFilePath)
{
    std::ifstream fileStream(objFilePath);
    nlohmann::json jsonProject;
    if (fileStream.good())
    {
        try
        {
            fileStream >> jsonProject;
        }
        catch (std::exception& e)
        {
            IOLOG << "Error : While parsing " << objFilePath << "\n" << e.what() << LOGENDL;
            return;
        }
    }
    else
    {
        IOLOG << "Error : Cannot find " << objFilePath << LOGENDL;
        controller.updateInfo(new GuiDataWarning(TEXT_LOAD_FAILED_FILE_NOT_FOUND.arg(QString::fromStdString(objFilePath.filename().string())).arg(QString::fromStdString(objFilePath.string()))));
        return;
    }


    SafePtr<Author> author = LoadAuthor(controller, jsonProject);
    if (!author)
    {
        IOLOG << "The file : [" << objFilePath.string() << "] do not have a correct Author" << LOGENDL;
        controller.updateInfo(new GuiDataWarning(TEXT_CANT_LOAD_TAG_NO_AUTHOR.arg(QString::fromStdWString(objFilePath.wstring()))));
        return;
    }

    nlohmann::json json_objects;
    if (jsonProject.find(Key_Objects) != jsonProject.end())
        json_objects = jsonProject.at(Key_Objects);
    //Note (Aurélien) : compatibilty check
    // old key was "objs"
    else if (jsonProject.find("objs") != jsonProject.end())
        json_objects = jsonProject.at("objs");

    if (!json_objects.is_array())
        return;

    GraphManager& graphManager = controller.getGraphManager();

    for (const nlohmann::json& iterator : json_objects)
    {
        if (iterator.find(Key_Type) == iterator.end())
            continue;

        std::string elemTypeStr = iterator.at(Key_Type).get<std::string>();
        //retrocompatibility
        if (elemTypeStr == "ScanObject")
            elemTypeStr = "PCO";

        // (05-04-2025) retrocompatibility for the Grid
        if (elemTypeStr == "Grid")
        {
            elemTypeStr = "Box";
            // also the Box::grid_type must be forced to 'NoGrid'.
        }

        auto elemType(magic_enum::enum_cast<ElementType>(elemTypeStr));

        //Note (Aurélien) : compatibilty check
        // remove std::string key for Key_Objects
        ElementType type;
        if (elemType.has_value())
            type = elemType.value();
        else
            type = ElementType::Box;

        if (iterator.find(Key_Id) == iterator.end())
            continue;
        xg::Guid id = xg::Guid(iterator.at(Key_Id).get<std::string>());
        if (!id.isValid())
            continue;

        SafePtr<AGraphNode> loadedObject;
        switch (type)
        {
        case ElementType::Scan:
        case ElementType::PCO:
        {
            SafePtr<PointCloudNode> node = make_safe<PointCloudNode>(type == ElementType::PCO);
            DataDeserializer::DeserializePointCloudNode(node, iterator, controller);
            //checkPointCloudPath(node);
            loadedObject = node;
            break;
        }
        case ElementType::Tag:
        {
            SafePtr<TagNode> node = make_safe<TagNode>();
            DataDeserializer::DeserializeTagNode(node, iterator, controller);
            loadedObject = node;
            break;
        }
        case ElementType::SimpleMeasure:
        {
            SafePtr<SimpleMeasureNode> node = make_safe<SimpleMeasureNode>();
            DataDeserializer::DeserializeSimpleMeasureNode(node, iterator, controller);
            loadedObject = node;
            break;
        }
        case ElementType::PolylineMeasure:
        {
            SafePtr<PolylineMeasureNode> node = make_safe<PolylineMeasureNode>();
            DataDeserializer::DeserializePolylineMeasureNode(node, iterator, controller);
            loadedObject = node;
            break;
        }
        case ElementType::PointToPlaneMeasure:
        {
            SafePtr<PointToPlaneMeasureNode> node = make_safe<PointToPlaneMeasureNode>();
            DataDeserializer::DeserializePointToPlaneMeasureNode(node, iterator, controller);
            loadedObject = node;
            break;
        }
        case ElementType::PipeToPipeMeasure:
        {
            SafePtr<PipeToPipeMeasureNode> node = make_safe<PipeToPipeMeasureNode>();
            DataDeserializer::DeserializePipeToPipeMeasureNode(node, iterator, controller);
            loadedObject = node;
            break;
        }
        case ElementType::PipeToPlaneMeasure:
        {
            SafePtr<PipeToPlaneMeasureNode> node = make_safe<PipeToPlaneMeasureNode>();
            DataDeserializer::DeserializePipeToPlaneMeasureNode(node, iterator, controller);
            loadedObject = node;
            break;
        }
        case ElementType::PointToPipeMeasure:
        {
            SafePtr<PointToPipeMeasureNode> node = make_safe<PointToPipeMeasureNode>();
            DataDeserializer::DeserializePointToPipeMeasureNode(node, iterator, controller);
            loadedObject = node;
            break;
        }
        case ElementType::BeamBendingMeasure:
        {
            SafePtr<BeamBendingMeasureNode> node = make_safe<BeamBendingMeasureNode>();
            DataDeserializer::DeserializeBeamBendingMeasureNode(node, iterator, controller);
            loadedObject = node;
            break;
        }
        case ElementType::ColumnTiltMeasure:
        {
            SafePtr<ColumnTiltMeasureNode> node = make_safe<ColumnTiltMeasureNode>();
            DataDeserializer::DeserializeColumnTiltMeasureNode(node, iterator, controller);
            loadedObject = node;
            break;
        }
        case ElementType::Box:
        {
            SafePtr<BoxNode> box = make_safe<BoxNode>();
            DataDeserializer::DeserializeBoxNode(box, iterator, controller);
            loadedObject = box;
            break;
        }
        case ElementType::Cylinder:
        {
            SafePtr<CylinderNode> node = make_safe<CylinderNode>();
            DataDeserializer::DeserializeCylinderNode(node, iterator, controller);
            loadedObject = node;
            break;
        }
        case ElementType::Torus:
        {
            SafePtr<TorusNode> node = make_safe<TorusNode>();
            DataDeserializer::DeserializeTorusNode(node, iterator, controller);
            loadedObject = node;
            break;
        }
        //retro-compatibility
        case ElementType::Piping:
        {
            SafePtr<ClusterNode> node = make_safe<ClusterNode>();
            DataDeserializer::DeserializePiping(node, iterator, controller);
            loadedObject = node;
            break;
        }
        // !retro-compatibility
        case ElementType::Cluster:
        {
            SafePtr<ClusterNode> node = make_safe<ClusterNode>();
            DataDeserializer::DeserializeClusterNode(node, iterator, controller);
            loadedObject = node;
            break;
        }
        case ElementType::Point:
        {
            SafePtr<PointNode> node = make_safe<PointNode>();
            DataDeserializer::DeserializePointNode(node, iterator, controller);
            loadedObject = node;
            break;
        }
        case ElementType::Sphere:
        {
            SafePtr<SphereNode> node = make_safe<SphereNode>();
            DataDeserializer::DeserializeSphereNode(node, iterator, controller);
            loadedObject = node;
            break;
        }
        case ElementType::MeshObject:
        {
            SafePtr<MeshObjectNode> node = make_safe<MeshObjectNode>();
            DataDeserializer::DeserializeMeshObjectNode(node, iterator, controller);
            loadedObject = node;
            break;
        }
        case ElementType::ViewPoint:
        {
            SafePtr<ViewPointNode> node = make_safe<ViewPointNode>();
            DataDeserializer::DeserializeViewPointNode(node, iterator, controller);
            loadedObject = node;
            break;
        }
        }

        // Check if the object already exist in the graph
        SafePtr<AGraphNode> old_obj = graphManager.getNodeById(id);
        if (old_obj)
        {
            // Copy
            IOLOG << "Replacing object [" << id << "] data " << LOGENDL;
        }

        {
            WritePtr<AGraphNode> wLoaded = loadedObject.get();
            if (wLoaded)
            {
                wLoaded->setAuthor(author);
                loadObj[loadedObject] = std::pair(wLoaded->getId(), iterator);
            }
        }
    }

    controller.getContext().addProjectAuthors({ author });

    IOLOG << "project import objs from " << objFilePath.stem() << LOGENDL;
    return;
}

void LoadTagFile(Controller& controller, std::unordered_map<SafePtr<AGraphNode>, std::pair<xg::Guid, nlohmann::json>>& loadObj, const std::filesystem::path& tagFilePath)
{
    std::ifstream fileStream(tagFilePath);
    nlohmann::json jsonProject;
    if (fileStream.good())
    {
        try
        {
            fileStream >> jsonProject;
        }
        catch (std::exception& e)
        {
            IOLOG << "Error : While parsing " << tagFilePath << "\n" << e.what() << LOGENDL;
            return;
        }
    }
    else
    {
        IOLOG << "Error : Cannot find " << tagFilePath << LOGENDL;
        controller.updateInfo(new GuiDataWarning(TEXT_CANT_LOAD_TAG));
        return;
    }


    SafePtr<Author> author = LoadAuthor(controller, jsonProject);

    if (!author)
    {
        IOLOG << "The file : [" << tagFilePath.string() << "] do not have Author" << LOGENDL;
        controller.updateInfo(new GuiDataWarning(TEXT_CANT_LOAD_TAG_NO_AUTHOR.arg(QString::fromStdWString(tagFilePath.wstring()))));
        return;
    }

    //Note (Aurélien) : compatibilty check
    // remove std::string key for Key_Objects
    std::string key(Key_Tags);
    if (jsonProject.find(Key_Tags) == jsonProject.end())
        key = "tags";

    if (jsonProject.find(key) != jsonProject.end() && jsonProject.at(key).is_array())
    {
        for (const nlohmann::json& iterator : jsonProject.at(key))
        {
            xg::Guid guid;
            SafePtr<TagNode> tagNode = make_safe<TagNode>();
            DataDeserializer::DeserializeTagNode(tagNode, iterator, controller);
            WritePtr<TagNode> wTagNode = tagNode.get();
            if (!wTagNode)
            {
                guid = wTagNode->getId();
                wTagNode->setAuthor(author);
            }

            loadObj[tagNode] = std::pair(guid, iterator);
        }
    }

    controller.getContext().addProjectAuthors({ author });
    IOLOG << "project import tag from " << tagFilePath.stem() << LOGENDL;

    return;
}

void LoadViewPointsFile(Controller& controller, std::unordered_map<SafePtr<AGraphNode>, std::pair<xg::Guid, nlohmann::json>>& loadObj, const std::filesystem::path& objFilePath)
{
    std::ifstream fileStream(objFilePath);
    nlohmann::json jsonProject;
    if (fileStream.good())
    {
        try
        {
            fileStream >> jsonProject;
        }
        catch (std::exception& e)
        {
            IOLOG << "Error : While parsing " << objFilePath << "\n" << e.what() << LOGENDL;
            return;
        }
    }
    else
    {
        IOLOG << "Error : Cannot find " << objFilePath << LOGENDL;
        controller.updateInfo(new GuiDataWarning(TEXT_LOAD_FAILED_FILE_NOT_FOUND.arg(QString::fromStdWString(objFilePath.filename().wstring())).arg(QString::fromStdWString(objFilePath.wstring()))));
        return;
    }


    SafePtr<Author> author = LoadAuthor(controller, jsonProject);
    if (!author)
    {
        IOLOG << "The file : [" << objFilePath.string() << "] do not have a correct Author" << LOGENDL;
        controller.updateInfo(new GuiDataWarning(TEXT_CANT_LOAD_TAG_NO_AUTHOR.arg(QString::fromStdWString(objFilePath.wstring()))));
        return;
    }

    if (jsonProject.find(Key_ViewPoints) != jsonProject.end() && jsonProject.at(Key_ViewPoints).is_array())
    {
        std::string tmpError = "";

        for (const nlohmann::json& iterator : jsonProject.at(Key_ViewPoints))
        {
            if (iterator.find(Key_Type) == iterator.end())
                continue;
            auto elemType(magic_enum::enum_cast<ElementType>(iterator.at(Key_Type).get<std::string>()));
            if (!elemType.has_value())
                continue;

            if (iterator.find(Key_Id) == iterator.end())
                continue;
            xg::Guid id = xg::Guid(iterator.at(Key_Id).get<std::string>());
            if (!id.isValid())
                continue;

            SafePtr<AGraphNode> loadedObject;
            if (elemType == ElementType::ViewPoint)
            {
                SafePtr<ViewPointNode> node = make_safe<ViewPointNode>();
                DataDeserializer::DeserializeViewPointNode(node, iterator, controller);
                loadedObject = node;
            }

            {
                WritePtr<AGraphNode> wLoaded = loadedObject.get();
                if (wLoaded)
                {
                    wLoaded->setAuthor(author);
                    loadObj[loadedObject] = std::pair(wLoaded->getId(), iterator);
                }
            }
        }
    }

    controller.getContext().addProjectAuthors({ author });

    IOLOG << "project import viewpoints from " << objFilePath.stem() << LOGENDL;
    return;
}

std::list<std::wstring> SaveLoadSystem::splitOnChar(std::wstring origin, std::wstring charList)
{
    std::list<std::wstring> list;
    std::wstring tmp;
    std::wstring::iterator it = origin.begin();

    while (it != origin.end())
    {
        if (charList.find(*it) != std::string::npos)
        {
            if (tmp.size() == 0)
            {
                it++;
                continue;
            }
            list.push_back(tmp);
            tmp.clear();
        }
        else
            tmp += *it;
        it++;
        if (it == origin.end() && tmp.size() > 0)
            list.push_back(tmp);
    }
    return (list);
}

std::filesystem::path removeLastExtension(const std::filesystem::path& file)
{
    return file.string().substr(0, file.string().find_last_of("."));
}

bool SaveLoadSystem::RestoreBackupFiles(const std::vector<std::filesystem::path>& backups)
{
    for (const std::filesystem::path& backup : backups)
    {
        std::filesystem::path newFile(removeLastExtension(backup));
        if (std::filesystem::exists(newFile))
            std::filesystem::remove(newFile);
        std::filesystem::rename(backup, newFile);
    }
    return backups.empty();
}

bool SaveLoadSystem::IsBackFilesExists(const std::filesystem::path& filePath, std::vector<std::filesystem::path>& backups)
{
    std::vector<std::filesystem::path> backupsToCheck(Utils::System::getFilesFromDirectory(filePath, File_Extension_Backup, true));
    if (backupsToCheck.empty())
        return false;
    for (const std::filesystem::path& backup : backupsToCheck)
    {
        std::filesystem::file_time_type backupTime(std::filesystem::last_write_time(backup));
        std::filesystem::path real(removeLastExtension(backup));
        if (std::filesystem::exists(real))
        {
            std::filesystem::file_time_type realTime(std::filesystem::last_write_time(real));
            if (realTime > backupTime)
                continue;
        }
        backups.push_back(backup);
    }
    return backups.empty();
}

bool SaveLoadSystem::readProjectTypes(const Controller& controller, const std::filesystem::path& filePath)
{
    IOLOG << "Loading file [" << filePath << "]" << LOGENDL;
    std::ifstream fileStream(filePath);
    nlohmann::json jsonProject;

    if (fileStream.good())
    {
        try
        {
            fileStream >> jsonProject;
        }
        catch (std::exception& e)
        {
            IOLOG << "Error : While parsing " << filePath << "\n" << e.what() << LOGENDL;
            return (false);
        }
    }
    else
    {
        IOLOG << "Error : Cannot find " << filePath << LOGENDL;
        return (false);
    }
    IOLOG << "project import generation" << LOGENDL;

    ProjectInfos info;
    if (!DataDeserializer::DeserializeProjectInfos(jsonProject, controller, info))
    {
        IOLOG << "Failed to import Project" << LOGENDL;
        return (false);
    }

    return true;
}

void SaveLoadSystem::importJsonProject(const std::filesystem::path& importPath, Controller& controller, std::string& errorMsg)
{
    ControllerContext& context = controller.getContext();
    GraphManager& graphManager = controller.getGraphManager();

    IOLOG << "Loading file [" << importPath.string() << "]" << LOGENDL;
    std::ifstream fileStream(importPath);
    nlohmann::json jsonProject;

    if (fileStream.good())
    {
        try
        {
            fileStream >> jsonProject;
        }
        catch (std::exception& e)
        {
            IOLOG << "Error : While parsing " << importPath << "\n" << e.what() << LOGENDL;
            return;
        }
    }
    else
    {
        IOLOG << "Error : Cannot find " << importPath << LOGENDL;
        errorMsg = TEXT_CANT_LOAD_PROJECT_TLP_ERROR.toStdString();
        return;
    }
    IOLOG << "project import generation" << LOGENDL;

    if (jsonProject.find(Key_SaveLoadSystemVersion) != jsonProject.end())
    {
        float projectSaveLoadVersion = jsonProject.at(Key_SaveLoadSystemVersion).get<float>();
        if (projectSaveLoadVersion > SAVELOADSYSTEMVERSION)
        {
            IOLOG << "Error : Project version : " << projectSaveLoadVersion << " > SaveLoadSytem version : " << SAVELOADSYSTEMVERSION << LOGENDL;
            errorMsg = TEXT_CANT_LOAD_PROJECT_OUTDATED_SAVELOADSYTEM.toStdString();
            return;
        }
    }

    // Get basic information
    ProjectInfos info;
    //info.m_id = importPath.
    info.m_projectName = importPath.stem();
    if (!DataDeserializer::DeserializeProjectInfos(jsonProject, controller, info))
    {
        IOLOG << "Failed to import Project" << LOGENDL;
        return;
    }

    SafePtr<CameraNode> cameraNode = controller.getGraphManager().getCameraNode();
    if (jsonProject.find(Key_ViewPoint) != jsonProject.end())
        DataDeserializer::DeserializeCameraNode(cameraNode, jsonProject.at(Key_ViewPoint));
    {
        WritePtr<CameraNode> wCamera = cameraNode.get();
        if (wCamera)
        {
            wCamera->m_unitUsage = controller.getContext().m_unitUsage;
        }
    }

    // Create project
    context.initProjectInfo(importPath.parent_path(), info);

    const ProjectInternalInfo& internalInfo = context.cgetProjectInternalInfo();
    LanguageType lang = Config::getLanguage();

    //Note (Aurélien) : compatibilty check
    // remove std::filesystem::path  templatePath for getTemplatesFolderPath()
    std::filesystem::path templatePath(internalInfo.getTemplatesFolderPath());
    if (!std::filesystem::exists(templatePath))
        templatePath = templatePath.string().substr(0, templatePath.string().size() - 1);

    if (!controller.getContext().setUserLists(SaveLoadSystem::ImportLists<UserList>(templatePath / File_Lists), true))
    {
        //If failed to load list go back to default.
        // Note (aurélien) : improvement with message box.
        controller.getContext().setUserLists(generateDefaultLists(lang), true);
        IOLOG << "Fall back to load default [" << File_Lists << "]" << LOGENDL;
    }

    if (!controller.getContext().setStandards(SaveLoadSystem::ImportLists<StandardList>(templatePath / File_Pipes), StandardType::Pipe, true))
    {
        //If failed to load templates go back to default.
        // Note (aurélien) : improvement with message box.
        controller.getContext().setStandards(generateDefaultPipeStandardList(), StandardType::Pipe, true);
        IOLOG << "Fall back to load default [" << File_Pipes << "]" << LOGENDL;
    }

    std::vector<sma::TagTemplate> templates = SaveLoadSystem::ImportTemplates(controller, templatePath / File_Templates);
    //If failed to load templates go back to default.
    // Note (aurélien) : improvement with message box.
    if (!controller.getContext().setTemplates(templates, true))
    {
        templates = sma::GenerateDefaultTemplates(lang);
        controller.getContext().setTemplates(templates, true);
        IOLOG << "Fall back to generate default templates" << LOGENDL;
    }

    std::unordered_set<SafePtr<sma::TagTemplate>> loadedTemplates = controller.getContext().getTemplates();
    bool annotationFound = false;
    for (const SafePtr<sma::TagTemplate>& temp : loadedTemplates)
    {
        xg::Guid id;
        {
            ReadPtr<sma::TagTemplate> rTemp = temp.cget();
            if (!rTemp)
                continue;
            id = rTemp->getId();
        }

        if (id == xg::Guid(ANNOTATION_TEMP_ID))
        {
            controller.getContext().setCurrentTemplate(temp);
            annotationFound = true;
            break;
        }
    }

    if (annotationFound == false && !loadedTemplates.empty())
        controller.getContext().setCurrentTemplate(*(loadedTemplates.begin()));

    std::filesystem::path tagPath(internalInfo.getTagsFolderPath());

    std::list<std::filesystem::path> objectPathsList;
    if (!std::filesystem::exists(tagPath))
        tagPath = tagPath.string().substr(0, tagPath.string().size() - 1);
    if (std::filesystem::exists(tagPath))
    {
        for (const auto& p : std::filesystem::directory_iterator(tagPath))
            objectPathsList.push_back(p.path());
    }

    if (std::filesystem::exists(internalInfo.getObjectsProjectPath()))
    {
        for (const auto& p : std::filesystem::directory_iterator(internalInfo.getObjectsProjectPath()))
            objectPathsList.push_back(p.path());
    }

    if (jsonProject.find(Key_HierarchyMasterCluster) != jsonProject.end())
    {
        SafePtr<ClusterNode> hmc = make_safe<ClusterNode>();
        DataDeserializer::DeserializeClusterNode(hmc, jsonProject.at(Key_HierarchyMasterCluster), controller);
        {
            WritePtr<ClusterNode> wHmc = hmc.get();
            if (wHmc)
            {
                wHmc->setTreeType(TreeType::Hierarchy);
                wHmc->m_isMasterCluster = true;
            }
        }
        controller.getGraphManager().setHierarchyMasterCluster(hmc);
    }

    std::unordered_map<SafePtr<AGraphNode>, std::pair<xg::Guid, nlohmann::json>> loadObjs;

    for (const std::filesystem::path& p : objectPathsList)
    {
        controller.getContext().setUserLists(ImportLists<UserList>(p));
        controller.getContext().setTemplates(ImportTemplates(controller, p));
        for (const auto& standardType : ImportStandards(p))
            controller.getContext().setStandards(standardType.second, standardType.first);

        if (p.extension() == File_Extension_Tags)
            LoadTagFile(controller, loadObjs, p);
        else if (p.extension() == File_Extension_Objects)
            LoadObjFile(controller, loadObjs, p);
        else if (p.extension() == File_Extension_ViewPoints)
            LoadViewPointsFile(controller, loadObjs, p);
    }

    std::unordered_map<xg::Guid, SafePtr<AGraphNode>> nodeById;
    std::unordered_set<SafePtr<AGraphNode>> nodes;
    for (auto loadObj : loadObjs)
    {
        nodes.insert(loadObj.first);
        nodeById[loadObj.second.first] = loadObj.first;
    }

    //trees (old version for compatibility)
//    importAllTrees(controller, jsonProject, nodeById);

    for (const std::pair<xg::Guid, SafePtr<AGraphNode>>& pair : nodeById)
        graphManager.addNodesToGraph({ pair.second });

    {
        SafePtr<ClusterNode> hmc = graphManager.getHierarchyMasterCluster();
        ReadPtr<ClusterNode> rHMC = hmc.cget();
        if (rHMC)
            nodeById[rHMC->getId()] = hmc;
    }

    for (auto loadObj : loadObjs)
        DataDeserializer::PostDeserializeNode(loadObj.second.second, loadObj.first, nodeById);

    LoadFileObjects(controller, nodes, "", false);

    //user orientations
    //Note (Aurélien) : compatibilty check
    // remove std::string key for Key_UserOrientations
    std::string key(Key_UserOrientations);
    if (jsonProject.find(Key_UserOrientations) == jsonProject.end())
        key = "userOrientations";
    if (jsonProject.find(key) != jsonProject.end() && jsonProject.at(key).is_array())
    {
        std::unordered_map<uint32_t, std::pair<userOrientationId, QString>> toUi;
        for (const nlohmann::json& userOrientation : jsonProject.at(key))
        {
            UserOrientation uo;
            if (DataDeserializer::DeserializeUserOrientation(userOrientation, uo))
            {
                toUi[uo.getOrder()] = { uo.getId(), uo.getName() };
                context.getUserOrientations().insert({ uo.getId(), uo });
            }
            else
                IOLOG << "userOrientation not found" << LOGENDL;
        }
        controller.updateInfo(new GuiDataSendUserOrientationList(toUi));
    }
    else
    {
        controller.updateInfo(new GuiDataSendUserOrientationList(std::unordered_map<uint32_t, std::pair<userOrientationId, QString>>()));
        errorMsg += TEXT_TEMPLATE_INVALID_USER_ORIENTATION.toStdString();
    }

    IOLOG << "Importation over\n" << LOGENDL;

    const ProjectInfos& projectInfos = context.getProjectInfo();

    std::wstring authorName = L"NO_AUTHOR";
    {
        ReadPtr<Author> rAuth = projectInfos.m_author.cget();
        if (rAuth)
            authorName = rAuth->getName();
    }

    IOLOG << " /! Project header" << LOGENDL;
    IOLOG << "Project Imported" << LOGENDL;
    IOLOG << "Name : " << projectInfos.m_projectName << LOGENDL;
    IOLOG << "Author : " << authorName << LOGENDL;
    IOLOG << "Company : " << projectInfos.m_company << LOGENDL;
    IOLOG << "Location : " << projectInfos.m_location << LOGENDL;
    //IOLOG << "DefScan: " << context.getDefaultScan() << LOGENDL;
    IOLOG << "Desc : " << projectInfos.m_description << "\n" << LOGENDL;

    IOLOG << " /! paths" << LOGENDL;
    IOLOG << "FilePath : " << internalInfo.getProjectFilePath() << LOGENDL;
    IOLOG << "Project folder : " << internalInfo.getProjectFolderPath() << LOGENDL;
    IOLOG << "Scanfolder : " << internalInfo.getPointCloudFolderPath(false) << LOGENDL;
    //IOLOG << "Tag folder : " << project->getProjectInternalInfo().getTagsFolderPath() << LOGENDL;
    IOLOG << "Object folder : " << internalInfo.getObjectsProjectPath() << "\n" << LOGENDL;

    IOLOG << " /! general info" << LOGENDL;
    IOLOG << "Project contains : " << graphManager.getProjectPointsCount() << " cloud points." << LOGENDL;
    IOLOG << "Default Scan: " << ((context.getDefaultScan()) ? "exists" : "failed") << LOGENDL;
    IOLOG << "Project contains : " << graphManager.getProjectNodes().size() << " elements in total" << LOGENDL;
    IOLOG << "Project contains : " << controller.getContext().getTemplates().size() << " templates" << LOGENDL;
    IOLOG << "Project contains : " << controller.getContext().getUserLists().size() << " lists" << LOGENDL;
    IOLOG << "Project contains : " << controller.getContext().getStandards(StandardType::Pipe).size() << " pipes standards" << LOGENDL;
    IOLOG << "Project contains : " << controller.getContext().getStandards(StandardType::Sphere).size() << " spheres standards" << LOGENDL;
    IOLOG << "Project contains : " << getNumberOfTElemTypeInProject(controller, ElementType::Cluster) << " folders" << LOGENDL;
    IOLOG << "Project contains : " << getNumberOfTElemTypeInProject(controller, ElementType::Scan) << " scans" << LOGENDL;
    IOLOG << "Project contains : " << getNumberOfTElemTypeInProject(controller, ElementType::Tag) << " tags" << LOGENDL;
    IOLOG << "Project contains : " << getNumberOfTElemTypeInProject(controller, ElementType::Box) << " boxes" << LOGENDL;
    IOLOG << "Project contains : " << getNumberOfTElemTypeInProject(controller, ElementType::Cylinder) << " cylinders" << LOGENDL;
    IOLOG << "Project contains : " << getNumberOfTElemTypeInProject(controller, ElementType::Sphere) << " spheres" << LOGENDL;
    IOLOG << "Project contains : " << getNumberOfTElemTypeInProject(controller, ElementType::PCO) << " PCObjects" << LOGENDL;
    IOLOG << "Project contains : " << getNumberOfTElemTypeInProject(controller, ElementType::Point) << " points" << LOGENDL;
    IOLOG << "Project contains : " << getNumberOfTElemTypeInProject(controller, ElementType::PipeToPipeMeasure)
        + getNumberOfTElemTypeInProject(controller, ElementType::PointToPipeMeasure)
        + getNumberOfTElemTypeInProject(controller, ElementType::PointToPlaneMeasure)
        + getNumberOfTElemTypeInProject(controller, ElementType::PipeToPlaneMeasure)
        + getNumberOfTElemTypeInProject(controller, ElementType::SimpleMeasure)
        + getNumberOfTElemTypeInProject(controller, ElementType::PolylineMeasure) << " measures" << LOGENDL;
    IOLOG << "Project contains : " << context.cgetUserOrientations().size() << " user orientations" << LOGENDL;

    return;
}

SafePtr<PointCloudNode> SaveLoadSystem::ImportNewTlsFile(const std::filesystem::path& filePath, bool is_object, Controller& controller, ErrorCode& errorCode)
{
    GraphManager& graphManager = controller.getGraphManager();
    ControllerContext& context = controller.getContext();

    tls::ScanGuid scanGuid;
    if (tlGetScanGuid(filePath, scanGuid) == false)
    {
        IOLOG << "Error: " << filePath << " is not a valid tls file." << LOGENDL;
        errorCode = ErrorCode::Failed_To_Open;
        return SafePtr<PointCloudNode>();
    }

    // Check the availability of the name in the project model
    if (graphManager.isFilePathOrScanExists(filePath.stem().wstring(), filePath) == true)
    {
        IOLOG << "Error : file or name already exists in the project : " << filePath.stem().string() << LOGENDL;
        // TODO - Ask the user if he want to save the Scanunder an other name (or append it)
        //return ("Error : file or name already exists and the Scanhas been copied");
        errorCode = ErrorCode::Failed_Write_Permission;
        return SafePtr<PointCloudNode>();
    }

    std::filesystem::path filename(filePath.filename());
    std::filesystem::path dst_path = context.cgetProjectInternalInfo().getPointCloudFolderPath(is_object) / filename;
    // Asynchronous copy
    // The availability of the name in the filesystem is checked by the PCE
    if (!std::filesystem::exists(dst_path))
        tlCopyScanFile(scanGuid, dst_path, true, false, false);
    else
        IOLOG << "INFO: " << filePath << " already exist." << LOGENDL;

    uint64_t nbScanBeforeImport = controller.getGraphManager().getNodesByTypes({ ElementType::Scan }).size();
    SafePtr<PointCloudNode> pc = make_safe<PointCloudNode>(is_object);
    {
        WritePtr<PointCloudNode> wpc = pc.get();
        if (!wpc)
        {
            IOLOG << "Error : cannot create object" << LOGENDL;
            errorCode = ErrorCode::Failed_Write_Permission;
            return SafePtr<PointCloudNode>();
        }

        wpc->setDefaultData(controller);
        wpc->setTlsFilePath(dst_path, true);
        if (!is_object)
            wpc->setColor(Color32(rand() % 255, rand() % 255, rand() % 255, 255));
    }

    controller.getDataDispatcher().sendControl(new control::function::AddNodes(pc, is_object));

    if (nbScanBeforeImport == 0)
        controller.updateInfo(new GuiDataMoveToData(pc));

    errorCode = ErrorCode::Success;
    return pc;
}

std::vector<sma::TagTemplate> SaveLoadSystem::ImportTemplates(const Controller& controller, const std::filesystem::path& filePath)
{
    std::filesystem::path importPath(filePath);
    std::vector<sma::TagTemplate> templates;

    if (filePath.empty())
    {
        std::filesystem::path path = Utils::System::getOSTProgramDataPath();
        path = Utils::System::getAndCreateDirectory(path, Folder_Template);
        importPath = path / File_Templates;
    }

    IOLOG << "Loading templates [" << importPath << "]" << LOGENDL;
    std::ifstream fileStream(importPath);
    nlohmann::json jsonTemplates;
    if (fileStream.good())
    {
        try
        {
            fileStream >> jsonTemplates;
        }
        catch (std::exception& e)
        {
            IOLOG << "Error : While parsing " << importPath << "\n" << e.what() << LOGENDL;
            return templates;
        }
    }
    else
    {
        IOLOG << "Error : Cannot find " << importPath << LOGENDL;
        return templates;
    }

    if (jsonTemplates.find(Key_Templates) != jsonTemplates.end())
    {
        for (const nlohmann::json& iterator : jsonTemplates.at(Key_Templates))
        {
            sma::TagTemplate tagTemplateData;
            if (DataDeserializer::DeserializeTagTemplate(iterator, controller, tagTemplateData))
            {
                IOLOG << "import template : " << tagTemplateData.getName() << " with " << tagTemplateData.getFieldSize() << " element(s)" << LOGENDL;
                templates.push_back(tagTemplateData);
            }
            else
                IOLOG << "import template : error(s) while parsing, not included in project!" << LOGENDL;
        }
    }
    IOLOG << "import " << templates.size() << " templates" << LOGENDL;
    return (templates);
}

std::unordered_set<SafePtr<Author>> SaveLoadSystem::loadLocalAuthors(const Controller& controller, ErrorCode& errorCode, const std::filesystem::path & filePath)
{
    std::filesystem::path importPath(filePath);
    std::unordered_set<SafePtr<Author>> authors;

    if (filePath.empty())
    {
        std::filesystem::path path = Utils::System::getOSTProgramDataPath();
        importPath = path / Folder_Authors / File_Authors;
    }

    IOLOG << "Loading authors [" << importPath << "]" << LOGENDL;
    std::ifstream fileStream(importPath);
    nlohmann::json jsonTemplates;
    if (fileStream.good())
    {
        try
        {
            fileStream >> jsonTemplates;
        }
        catch (std::exception& e)
        {
            IOLOG << "Error : While parsing " << importPath << "\n" << e.what() << LOGENDL;
            return (authors);
        }
    }
    else
    {
        IOLOG << "Error : Cannot find " << importPath << LOGENDL;
        return (authors);
    }

    if (jsonTemplates.find(Key_Authors) != jsonTemplates.end() && jsonTemplates.at(Key_Authors).is_array())
    {
        std::set<std::wstring> names;
        for (nlohmann::json iterator : jsonTemplates.at(Key_Authors))
        {
            Author authorData;
            if (DataDeserializer::DeserializeAuthor(iterator, authorData))
                authors.insert(controller.cgetContext().createAuthor(authorData));
            else
            {
                try //Note (Aurélien) : compatibilty check
                {
                    names.insert(Utils::from_utf8(iterator.get<std::string>()));
                }
                catch (...)
                {
                    if (iterator.find(Key_Name) != iterator.end())
                    {
                        std::wstring authorName(Utils::from_utf8(iterator.at(Key_Name).get<std::string>()));
                        IOLOG << "import author : " << authorName << LOGENDL;
                        if (names.find(authorName) == names.end())
                            names.insert(authorName);
                        else
                            IOLOG << "try to import two times the same author" << LOGENDL;
                    }
                    //Note (Aurélien) : compatibilty check
                    else if (iterator.find("AuthorName") != iterator.end())
                    {
                        std::wstring authorName(Utils::from_utf8(iterator.at("AuthorName").get<std::string>()));
                        IOLOG << "import author : " << authorName << LOGENDL;
                        if (names.find(authorName) == names.end())
                            names.insert(authorName);
                        else
                            IOLOG << "try to import two times the same author" << LOGENDL;
                    }
                }
            }
        }

        for (std::wstring name : names)
        {
            SafePtr<Author> oldAuth = make_safe<Author>(name);
            authors.insert(oldAuth);
        }
    }
    IOLOG << "import " << authors.size() << " authors" << LOGENDL;
    return (authors);
}

std::filesystem::path SaveLoadSystem::saveAuthors(const std::unordered_set<SafePtr<Author>>& authors, ErrorCode& errorCode, const std::filesystem::path & filePath)
{
    std::filesystem::path exportPath;

    if (filePath.empty())
    {
        std::filesystem::path path = Utils::System::getOSTProgramDataPath();
        path = Utils::System::getAndCreateDirectory(path, Folder_Authors);
        exportPath = path / File_Authors;
    }
    else
        exportPath = filePath;

    nlohmann::json jsonTemplates;
    IOLOG << "Export authors at " << exportPath << LOGENDL;
    nlohmann::json templatesArray = nlohmann::json::array();
    for (const SafePtr<Author>& author : authors)
    {
        ReadPtr<Author> rAuth = author.cget();
        if (!rAuth)
            continue;

        templatesArray.push_back(DataSerializer::Serialize(*&rAuth));
    }

    jsonTemplates[Key_Authors] = templatesArray;

    if (!utils::writeJsonFile(exportPath, jsonTemplates))
    {
        IOLOG << "Error : failed to save json : " << exportPath << LOGENDL;
        errorCode = ErrorCode::Failed_Write_Permission;
        return ("");
    }
    errorCode = ErrorCode::Success;
    return ("");
}

std::wstring SaveLoadSystem::getAuthorFilename(const Author& auth)
{
    std::wstring filename = auth.getName().c_str(); // Remove any null characters
    filename += L"_";
    filename += Utils::from_utf8(auth.getId().str());
    return filename;
}

std::vector<std::filesystem::path> renameOldObjectFiles(const std::vector<std::filesystem::path>& oldFiles)
{
    std::vector<std::filesystem::path> renamedFiles;
    for (const std::filesystem::path& file : oldFiles)
    {
        std::filesystem::path backFileName = file.wstring() + File_Extension_Backup_Wide;
        try
        {
            std::filesystem::rename(file, backFileName);
        }
        catch (std::exception e)
        {
            assert(false);
            continue;
        }
        renamedFiles.push_back(backFileName);
    }

    return renamedFiles;
}

bool SaveLoadSystem::ExportProject(Controller& controller, const std::unordered_set<SafePtr<AGraphNode>>& objects, const ProjectInternalInfo& internalInfo, const ProjectInfos& projectInfos, const SafePtr<CameraNode>& camera)
{
    const ControllerContext& context = controller.getContext();

    if (std::filesystem::exists(internalInfo.getProjectFolderPath()) == false)
    {
        IOLOG << "Create folder " << internalInfo.getProjectFolderPath() << LOGENDL;
        // Crash if the path is not valid
        try
        {
            std::filesystem::create_directory(internalInfo.getProjectFolderPath());
        }
        catch (std::exception&) {
            IOLOG << "Error: invalid folder" << LOGENDL;
            false;
        }
    }
    std::filesystem::path exportPath = internalInfo.getProjectFilePath();

    IOLOG << "Export JSON at " << exportPath << LOGENDL;
    nlohmann::json jsonProject(DataSerializer::Serialize(projectInfos));

    //OpenScanToolsVersion
    jsonProject[Key_OpenScanToolsVersion] = OPENSCANTOOLS_VERSION;
    jsonProject[Key_SaveLoadSystemVersion] = SAVELOADSYSTEMVERSION;

    //DefaultScanId
    ReadPtr<PointCloudNode> rDefaultScan = controller.getContext().getDefaultScan().cget();
    jsonProject[Key_DefaultScanId] = rDefaultScan ? rDefaultScan->getId() : xg::Guid();

    nlohmann::json hierarchyMasterCluster;
    DataSerializer::Serialize(hierarchyMasterCluster, controller.getGraphManager().getHierarchyMasterCluster());
    jsonProject[Key_HierarchyMasterCluster] = hierarchyMasterCluster;

    //Camera
    if (camera)
    {
        nlohmann::json cameraInfo;
        DataSerializer::Serialize(cameraInfo, camera);
        jsonProject[Key_ViewPoint] = cameraInfo;
    }

    jsonProject[Key_UserOrientations] = exportUserOrientationBlock(context);

    if (!utils::writeJsonFile(exportPath, jsonProject))
    {
        IOLOG << "Error : failed to save json : " << exportPath << LOGENDL;
        assert(false);
        return false;
    }

    Utils::System::createDirectoryIfNotExist(internalInfo.getPointCloudFolderPath(false));
    Utils::System::createDirectoryIfNotExist(internalInfo.getPointCloudFolderPath(true));
    Utils::System::createDirectoryIfNotExist(internalInfo.getObjectsProjectPath());
    Utils::System::createDirectoryIfNotExist(internalInfo.getObjectsFilesFolderPath());
    Utils::System::createDirectoryIfNotExist(internalInfo.getTemplatesFolderPath());

    Utils::System::createDirectoryIfNotExist(internalInfo.getQuickScreenshotsFolderPath());
    Utils::System::createDirectoryIfNotExist(internalInfo.getOrthoHDFolderPath());
    Utils::System::createDirectoryIfNotExist(internalInfo.getPerspHDFolderPath());

    std::vector<std::filesystem::path> toDeleteFiles;
    std::vector<std::filesystem::path> dumpFiles;

    IOLOG << "Start backing up old objects files" << LOGENDL;

    dumpFiles = Utils::System::getFilesFromDirectory(internalInfo.getObjectsProjectPath(), File_Extension_ViewPoints, false);
    dumpFiles = renameOldObjectFiles(dumpFiles);
    toDeleteFiles.insert(toDeleteFiles.begin(), dumpFiles.begin(), dumpFiles.end());

    dumpFiles = Utils::System::getFilesFromDirectory(internalInfo.getObjectsProjectPath(), File_Extension_Objects, false);
    dumpFiles = renameOldObjectFiles(dumpFiles);
    toDeleteFiles.insert(toDeleteFiles.begin(), dumpFiles.begin(), dumpFiles.end());

    dumpFiles = Utils::System::getFilesFromDirectory(internalInfo.getTagsFolderPath(), File_Extension_Tags, false);
    dumpFiles = renameOldObjectFiles(dumpFiles);
    toDeleteFiles.insert(toDeleteFiles.begin(), dumpFiles.begin(), dumpFiles.end());

    IOLOG << "Finish backing up old objects files" << LOGENDL;

    //On détruit les anciens fichiers objets si l'export des nouveaux fichiers objets c'est bien passé
    if (ExportAuthorObjects(controller, internalInfo.getObjectsProjectPath(), objects, true))
    {
        for (std::filesystem::path filePath : toDeleteFiles)
        {
            try
            {
                IOLOG << "Removing " << filePath << LOGENDL;
                std::filesystem::remove(filePath);
            }
            catch (...) {}
        }
    }

    return true;
}

std::filesystem::path SaveLoadSystem::ExportTemplates(const std::unordered_set<SafePtr<sma::TagTemplate>>& templates, ErrorCode& errorCode, const std::filesystem::path& filePath)
{
    std::vector<sma::TagTemplate> exportTemps;
    for (SafePtr<sma::TagTemplate> temp : templates)
    {
        ReadPtr<sma::TagTemplate> rTemp = temp.cget();
        if (rTemp)
            exportTemps.push_back(*&rTemp);
    }
    return ExportTemplates(exportTemps, errorCode, filePath);
}

std::filesystem::path SaveLoadSystem::ExportTemplates(const std::vector<sma::TagTemplate>& templatesData, ErrorCode& errorCode, const std::filesystem::path& filePath)
{
    if (templatesData.empty())
        return filePath;

    std::filesystem::path exportPath;

    if (filePath.empty())
    {
        std::filesystem::path path = Utils::System::getOSTProgramDataPath();
        path = Utils::System::getAndCreateDirectory(path, Folder_Template);
        exportPath = path / File_Templates;
    }
    else
        exportPath = filePath;

    nlohmann::json jsonTemplates;
    IOLOG << "Export lists at " << exportPath << LOGENDL;
    nlohmann::json templatesArray = nlohmann::json::array();

    std::unordered_set<UserList> listsData;
    for (const sma::TagTemplate& tempData : templatesData)
    {
        for (std::pair<sma::tFieldId, sma::tField> field : tempData.getFields())
        {
            if (field.second.m_type == sma::tFieldType::list)
            {
                ReadPtr<UserList> rList = field.second.m_fieldReference.cget();
                if (rList)
                    listsData.insert(*&rList);
            }
        }
        templatesArray.push_back(DataSerializer::Serialize(tempData));
    }

    nlohmann::json listsArray = nlohmann::json::array();
    for (const UserList& iterator : listsData)
        listsArray.push_back(DataSerializer::SerializeList<UserList>(iterator));


    jsonTemplates[Key_Lists] = listsArray;
    jsonTemplates[Key_Templates] = templatesArray;

    if (!utils::writeJsonFile(exportPath, jsonTemplates))
    {
        IOLOG << "Error : failed to save json : " << exportPath << LOGENDL;
        assert(false);
        errorCode = ErrorCode::Failed_Write_Permission;
        return "";
    }

    errorCode = ErrorCode::Success;
    return (exportPath);
}

void SaveLoadSystem::loadArboFile(Controller& controller, const std::filesystem::path& folderPath)
{
    IOLOG << "Loading arbo file from folder : [" << folderPath << "]" << LOGENDL;
    std::vector<std::filesystem::path> newArboFiles;

    SafePtr<Author> activeAuthor = controller.getContext().getActiveAuthor();

    for (const auto& p : std::filesystem::directory_iterator(folderPath))
    {
        std::filesystem::path path = p.path();

        if (path.extension() != File_Extension_Clusters)
            continue;

        std::ifstream fileStream(path);
        nlohmann::json jsonArbo;

        if (fileStream.good())
        {
            try
            {
                fileStream >> jsonArbo;
            }
            catch (std::exception& e)
            {
                IOLOG << "Error : While parsing " << path << "\n" << e.what() << LOGENDL;
                return;
            }
        }
        else
        {
            IOLOG << "Error : Cannot find " << path << LOGENDL;
            return;
        }
        IOLOG << "arbo import" << LOGENDL;

        bool oldVersion = true;
        if (jsonArbo.find(Key_SaveLoadSystemVersion) != jsonArbo.end())
        {
            float projectSaveLoadVersion = jsonArbo.at(Key_SaveLoadSystemVersion).get<float>();
            if (projectSaveLoadVersion > SAVELOADSYSTEMVERSION)
            {
                IOLOG << "Error : Project version : " << projectSaveLoadVersion << " > SaveLoadSytem version : " << SAVELOADSYSTEMVERSION << LOGENDL;
                return;
            }
            oldVersion = false;
        }

        if (oldVersion)
        {
            std::unordered_map<xg::Guid, SafePtr<AGraphNode>> dumpNodeById;
            importAllTrees(controller, jsonArbo, dumpNodeById);
            std::unordered_set<SafePtr<AGraphNode>> toAddNodes;
            for (const auto& pair : dumpNodeById)
                toAddNodes.insert(pair.second);
            controller.getGraphManager().addNodesToGraph(toAddNodes);
        }
        else
        {
            std::unordered_map<xg::Guid, SafePtr<AGraphNode>> nodeById;

            if (jsonArbo.find(Key_HierarchyMasterCluster) != jsonArbo.end())
            {
                SafePtr<ClusterNode> hmc = make_safe<ClusterNode>();
                DataDeserializer::DeserializeClusterNode(hmc, jsonArbo.at(Key_HierarchyMasterCluster), controller);
                {
                    WritePtr<ClusterNode> wHmc = hmc.get();
                    if (wHmc)
                    {
                        wHmc->setTreeType(TreeType::Hierarchy);
                        wHmc->m_isMasterCluster = true;
                        nodeById[wHmc->getId()] = hmc;
                        wHmc->setId(xg::newGuid());
                        wHmc->setAuthor(activeAuthor);
                    }
                }
                controller.getGraphManager().setHierarchyMasterCluster(hmc);
            }
            else
                controller.getGraphManager().createHierarchyMasterCluster();

            std::unordered_set<SafePtr<AGraphNode>> toAddNodes;
            std::unordered_map<SafePtr<AGraphNode>, nlohmann::json> toPostDeserialize;

            if (jsonArbo.find(Key_Clusters) != jsonArbo.end())
            {
                for (const nlohmann::json& iterator : jsonArbo.at(Key_Clusters))
                {
                    SafePtr<ClusterNode> newClu = make_safe<ClusterNode>();
                    DataDeserializer::DeserializeClusterNode(newClu, iterator, controller);
                    {
                        WritePtr<ClusterNode> wClu = newClu.get();
                        if (!wClu)
                            continue;

                        wClu->setAuthor(activeAuthor);
                        nodeById[wClu->getId()] = newClu;
                        wClu->setId(xg::newGuid());
                    }
                    toAddNodes.insert(newClu);
                    toPostDeserialize[newClu] = iterator;
                }
            }

            for (const std::pair<SafePtr<AGraphNode>, nlohmann::json>& postDeser : toPostDeserialize)
                DataDeserializer::PostDeserializeNode(postDeser.second, postDeser.first, nodeById);
            controller.getGraphManager().addNodesToGraph(toAddNodes);
        }
    }
}

void SaveLoadSystem::exportArboFile(const std::filesystem::path& folderPath, const Controller& controller)
{
    nlohmann::json jsonArboFile;

    jsonArboFile[Key_SaveLoadSystemVersion] = SAVELOADSYSTEMVERSION;

    nlohmann::json hierarchyMasterCluster;
    DataSerializer::Serialize(hierarchyMasterCluster, controller.cgetGraphManager().getHierarchyMasterCluster());
    jsonArboFile[Key_HierarchyMasterCluster] = hierarchyMasterCluster;

    std::unordered_set<SafePtr<AGraphNode>> clusters = controller.cgetGraphManager().getNodesByTypes({ ElementType::Cluster });

    nlohmann::json clusIn = nlohmann::json::array();
    for (const SafePtr<AGraphNode>& cluster : clusters)
    {
        ElementType type;
        {
            ReadPtr<AGraphNode> rNode = cluster.cget();
            if (!rNode)
                continue;
            type = rNode->getType();
        }

        if (type != ElementType::Cluster)
            continue;

        nlohmann::json jsonObject;
        {
            DataSerializer::Serialize(jsonObject, static_pointer_cast<ClusterNode>(cluster));
        }
        if (!jsonObject.empty())
            clusIn.push_back(jsonObject);
    }
    jsonArboFile[Key_Clusters] = clusIn;

    std::filesystem::path filepath = folderPath / "arbo.tlc";

    if (!utils::writeJsonFile(filepath, jsonArboFile))
    {
        IOLOG << "Error : failed to save json : " << filepath << LOGENDL;
        assert(false);
        return;
    }
}

bool SaveLoadSystem::ExportAuthorObjects(const Controller& controller, const std::filesystem::path& exportFolder, const std::unordered_set<SafePtr<AGraphNode>>& objectsToExport, bool exportListTemplateWith)
{
    const ControllerContext& context = controller.cgetContext();
    
    std::unordered_map<Author, std::unordered_set<SafePtr<AGraphNode>>> exportContent;

    for (const SafePtr<AGraphNode>& data : objectsToExport)
    {
        Author author = Author::createNullAuthor();

        ReadPtr<AGraphNode> rData = data.cget();
        if (!rData)
            continue;
        ReadPtr<Author> rAuth = rData->getAuthor().cget();
        if (rAuth)
            author = *&rAuth;

        if (exportContent.find(author) == exportContent.end())
            exportContent[author] = std::unordered_set<SafePtr<AGraphNode>>();

        exportContent[author].insert(data);
    }

    std::filesystem::path exportPath = exportFolder;

    for (std::pair<Author, std::unordered_set<SafePtr<AGraphNode>>> authObjs : exportContent)
    {
        nlohmann::json objsIn = nlohmann::json::array();
        std::unordered_set<SafePtr<sma::TagTemplate>> templates;
        std::unordered_map<StandardType, std::unordered_set<SafePtr<StandardList>>> standardsList;

        IOLOG << "generate " << authObjs.first.getName() << " obj list" << LOGENDL;

        nlohmann::json jsonAuthor;
        jsonAuthor[Key_Author] = DataSerializer::Serialize(authObjs.first);

        for (const SafePtr<AGraphNode>& object : authObjs.second)
        {
            ElementType type;
            nlohmann::json jsonObj;
            {
                ReadPtr<AGraphNode> data = object.cget();
                if (!data)
                    continue;
                type = data->getType();
                jsonObj[Key_Type] = magic_enum::enum_name(type);
            }

            switch (type)
            {
            case ElementType::Scan:
            case ElementType::PCO:
            {
                DataSerializer::Serialize(jsonObj, static_pointer_cast<PointCloudNode>(object));
            }
            break;
            case ElementType::Tag:
            {
                SafePtr<TagNode> tagNode = static_pointer_cast<TagNode>(object);
                DataSerializer::Serialize(jsonObj, tagNode);
                {
                    ReadPtr<TagNode> rObj = tagNode.cget();
                    if (!rObj)
                        break;
                    if (exportListTemplateWith)
                        templates.insert(rObj->getTemplate());
                }
            }
            break;
            case ElementType::SimpleMeasure:
                {
                    DataSerializer::Serialize(jsonObj, static_pointer_cast<SimpleMeasureNode>(object));
                }
                break;
            case ElementType::PolylineMeasure:
                {
                    DataSerializer::Serialize(jsonObj, static_pointer_cast<PolylineMeasureNode>(object));
                }
                break;
            case ElementType::PointToPlaneMeasure:
                {
                    DataSerializer::Serialize(jsonObj, static_pointer_cast<PointToPlaneMeasureNode>(object));
                }
                break;
            case ElementType::PipeToPipeMeasure:
                {
                    DataSerializer::Serialize(jsonObj, static_pointer_cast<PipeToPipeMeasureNode>(object));
                }
                break;
            case ElementType::PipeToPlaneMeasure:
                {
                    DataSerializer::Serialize(jsonObj, static_pointer_cast<PipeToPlaneMeasureNode>(object));
                }
                break;
            case ElementType::PointToPipeMeasure:
                {
                    DataSerializer::Serialize(jsonObj, static_pointer_cast<PointToPipeMeasureNode>(object));
                }
                break;
            case ElementType::BeamBendingMeasure:
                {
                    DataSerializer::Serialize(jsonObj, static_pointer_cast<BeamBendingMeasureNode>(object));
                }
                break;
            case ElementType::ColumnTiltMeasure:
                {
                    DataSerializer::Serialize(jsonObj, static_pointer_cast<ColumnTiltMeasureNode>(object));
                }
                break;
            case ElementType::Box:
                {
                    DataSerializer::Serialize(jsonObj, static_pointer_cast<BoxNode>(object));
                }
                break;
            case ElementType::Cylinder:
                {
                    SafePtr<CylinderNode> cylNode = static_pointer_cast<CylinderNode>(object);
                    DataSerializer::Serialize(jsonObj, cylNode);
                    {
                        ReadPtr<CylinderNode> rObj = cylNode.cget();
                        if (!rObj)
                            break;
                        if (standardsList.find(StandardType::Pipe) == standardsList.end())
                            standardsList[StandardType::Pipe] = { rObj->getStandard() };
                        else
                            standardsList[StandardType::Pipe].insert(rObj->getStandard());
                    }
                }
                break;
            case ElementType::Sphere:
            {
                DataSerializer::Serialize(jsonObj, static_pointer_cast<SphereNode>(object));
            }
                break;
            case ElementType::Torus:
            {
                DataSerializer::Serialize(jsonObj, static_pointer_cast<TorusNode>(object));
            }
                break;
            case ElementType::Piping:
                break;
            case ElementType::Point:
            {
                DataSerializer::Serialize(jsonObj, static_pointer_cast<PointNode>(object));
            }
                break;
            case ElementType::MeshObject:
            {
                DataSerializer::Serialize(jsonObj, static_pointer_cast<MeshObjectNode>(object));
            }
                break;
            case ElementType::ViewPoint:
            {
                DataSerializer::Serialize(jsonObj, static_pointer_cast<ViewPointNode>(object));
            }
                break;
            case ElementType::Cluster:
            {
                DataSerializer::Serialize(jsonObj, static_pointer_cast<ClusterNode>(object));
            }
            break;
            }

            if (!jsonObj.empty())
                objsIn.push_back(jsonObj);
        }

        if (objsIn.empty())
            continue;

        if (exportListTemplateWith)
        {
            std::unordered_set<SafePtr<UserList>> lists;
            lists.insert(context.getUserList(listId(LIST_DISCIPLINE_ID)));
            lists.insert(context.getUserList(listId(LIST_PHASE_ID)));
            
            nlohmann::json templatesArray = nlohmann::json::array();
            for (const SafePtr<sma::TagTemplate>& temp : templates)
            {
                ReadPtr<sma::TagTemplate> rTemp = temp.cget();
                if (rTemp)
                {
                    for (std::pair<sma::tFieldId, sma::tField> field : rTemp->getFields())
                        if (field.second.m_type == sma::tFieldType::list)
                            lists.insert(field.second.m_fieldReference);
                    templatesArray.push_back(DataSerializer::Serialize(*&rTemp));
                }
            }

            nlohmann::json listsArray = nlohmann::json::array();
            for (const SafePtr<UserList>& list : lists)
            {
                ReadPtr<UserList> rList = list.cget();
                if(rList)
                    listsArray.push_back(DataSerializer::SerializeList<UserList>(*&rList));
            }

            nlohmann::json standardArray = nlohmann::json::array();
            for (auto standard : standardsList)
            {
                std::unordered_set<List<double>> standardLists;
                for (SafePtr<StandardList> standard : standard.second)
                {
                    ReadPtr<StandardList> rStandard = standard.cget();
                    if(rStandard)
                        standardLists.insert(*&rStandard);
                }
                standardArray.push_back(DataSerializer::SerializeStandard(standardLists, standard.first));
            }


            if (!listsArray.empty())
                jsonAuthor[Key_Lists] = listsArray;
            if (!templatesArray.empty())
                jsonAuthor[Key_Templates] = templatesArray;
            if (!standardArray.empty())
                jsonAuthor[Key_Standards] = standardArray;

        }

        jsonAuthor[FilesExtensionArray.at(SaveLoadSystem::ObjectsFileType::Tlo).second] = objsIn;

        if (exportFolder == "")
            exportPath = context.cgetProjectInternalInfo().getObjectsProjectPath();

        std::wstring filename = getAuthorFilename(authObjs.first);
        std::filesystem::path name = exportPath / filename;
        name += FilesExtensionArray.at(SaveLoadSystem::ObjectsFileType::Tlo).first;

        if (!utils::writeJsonFile(name, jsonAuthor))
        {
            IOLOG << "Error : failed to save json : " << exportPath << LOGENDL;
            assert(false);
            return false;
        }

        IOLOG << "Creation of objs file " << name << LOGENDL;
    }
    return true;
}

void SaveLoadSystem::importAuthorObjects(const std::vector<std::filesystem::path>& importFiles, std::unordered_set<SafePtr<AGraphNode>>& succesfulImport, std::unordered_set<SafePtr<AGraphNode>>& fileNotFoundObjectImport, Controller& controller)
{
    std::unordered_map<SafePtr<AGraphNode>, std::pair<xg::Guid, nlohmann::json>> loadObjs;
    for (const std::filesystem::path& p : importFiles)
    {
        controller.getContext().setUserLists(ImportLists<UserList>(p));
        controller.getContext().setTemplates(ImportTemplates(controller, p));
        for (const auto& standardType : ImportStandards(p))
            controller.getContext().setStandards(standardType.second, standardType.first);

        if (p.extension() == File_Extension_Tags)
            LoadTagFile(controller, loadObjs, p);
        else if (p.extension() == File_Extension_Objects)
            LoadObjFile(controller, loadObjs, p);
        else if (p.extension() == File_Extension_ViewPoints)
            LoadViewPointsFile(controller, loadObjs, p);
    }

    std::unordered_map<xg::Guid, SafePtr<AGraphNode>> nodeById;
    for (SafePtr<AGraphNode> node : controller.getGraphManager().getProjectNodes())
    {
        ReadPtr<AGraphNode> rNode = node.cget();
        if(rNode)
            nodeById[rNode->getId()] = node;
    }
    
    for (auto loadObj : loadObjs)
    {
        succesfulImport.insert(loadObj.first);
        nodeById[loadObj.second.first] = loadObj.first;
    }

    SafePtr<ClusterNode> hmc = controller.getGraphManager().getHierarchyMasterCluster();
    {
        ReadPtr<ClusterNode> rHmc = hmc.cget();
        if (rHmc)
            nodeById[rHmc->getId()] = hmc;
    }

    for (auto loadObj : loadObjs)
        DataDeserializer::PostDeserializeNode(loadObj.second.second, loadObj.first, nodeById);


    fileNotFoundObjectImport = LoadFileObjects(controller, succesfulImport, "", false);
}


//new
/*
std::map<xg::Guid, std::pair <std::array<Data*, 2>, std::set<std::string>>> SaveLoadSystem::compareListObjects(std::set<Data*> avant, std::set<Data*> apres)
{
    std::map<xg::Guid, std::pair <std::array<Data*, 2>, std::set<std::string>>> modification;

    for (Data* ini : avant)
    {
        std::pair< std::array<Data*, 2>, std::set<std::string>> img;
        img.first[0] = ini;
        modification[ini->getId()] = img;
    }

    for (Data* fin : apres)
    {
        if (modification.find(fin->getId()) == modification.end())
        {
            std::pair< std::array<Data*, 2>, std::set<std::string>> img;
            img.first[1] = fin;

            modification[fin->getId()] = img;
        }
        else
            modification[fin->getId()].first[1] = fin;
    }

    for (auto p = modification.begin(); p != modification.end(); p++)
    {
        if (p->second.first[0] != nullptr && p->second.first[1] != nullptr)
        {
            switch (p->second.first[0]->getType())
            {
            case ElementType::Cylinder:
                try {
                    UICylinder* elem1 = static_cast<UICylinder*>(p->second.first[0]); //l'image avant
                    Cylinder* elem2 = static_cast<Cylinder*>(p->second.first[1]); //l'image apres

                    std::set<std::string> description = DataComparator::Compare(*elem1, *elem2);

                    for (std::string i : description)
                    {
                        p->second.second.insert(i);
                    }
                    break;
                }
                catch (const std::exception& e)
                {
                    std::cerr << e.what();
                    break;
                }
            case ElementType::Point:
                try {
                    UIPoint* elem1 = static_cast<UIPoint*>(p->second.first[0]); //l'image avant
                    Point* elem2 = static_cast<Point*>(p->second.first[1]); //l'image apres

                    std::set<std::string> description = DataComparator::Compare(*elem1, *elem2);

                    for (std::string i : description)
                    {
                        p->second.second.insert(i);
                    }
                    break;
                }
                catch (const std::exception& e)
                {
                    std::cerr << e.what();
                    break;
                }
            case ElementType::Torus:
                try {
                    UITorus* elem1 = static_cast<UITorus*>(p->second.first[0]); //l'image avant
                    Torus* elem2 = static_cast<Torus*>(p->second.first[1]); //l'image apres

                    std::set<std::string> description = DataComparator::Compare(*elem1, *elem2);

                    for (std::string i : description)
                    {
                        p->second.second.insert(i);
                    }
                    break;
                }
                catch (const std::exception& e)
                {
                    std::cerr << e.what();
                    break;
                }

            case ElementType::Piping:
                try {
                    UIPiping* elem1 = static_cast<UIPiping*>(p->second.first[0]); //l'image avant
                    Piping* elem2 = static_cast<Piping*>(p->second.first[1]); //l'image apres

                    std::set<std::string> description = DataComparator::Compare(*elem1, *elem2);

                    for (std::string i : description)
                    {
                        p->second.second.insert(i);
                    }
                    break;
                }
                catch (const std::exception& e)
                {
                    std::cerr << e.what();
                    break;
                }
            case ElementType::Sphere:
                try {
                    UISphere* elem1 = static_cast<UISphere*>(p->second.first[0]); //l'image avant
                    Sphere* elem2 = static_cast<Sphere*>(p->second.first[1]); //l'image apres

                    std::set<std::string> description = DataComparator::Compare(*elem1, *elem2);

                    for (std::string i : description)
                    {
                        p->second.second.insert(i);
                    }
                    break;
                }
                catch (const std::exception& e)
                {
                    std::cerr << e.what();
                    break;
                }
            case ElementType::Grid:
                try {
                    UIGrid* elem1 = static_cast<UIGrid*>(p->second.first[0]); //l'image avant
                    Grid* elem2 = static_cast<Grid*>(p->second.first[1]); //l'image apres

                    std::set<std::string> description = DataComparator::Compare(UIGrid(*elem1), UIGrid(*elem2));

                    for (std::string i : description)
                    {
                        p->second.second.insert(i);
                    }
                    break;
                }
                catch (const std::exception& e)
                {
                    std::cerr << e.what();
                    break;
                }
            case ElementType::Box:
                try {
                    UIBox* elem1 = static_cast<UIBox*>(p->second.first[0]); //l'image avant
                    Box* elem2 = static_cast<Box*>(p->second.first[1]); //l'image apres

                    std::set<std::string> description = DataComparator::Compare(UIBox(*elem1), UIBox(*elem2));

                    for (std::string i : description)
                    {
                        p->second.second.insert(i);
                    }
                    break;
                }
                catch (const std::exception& e)
                {
                    std::cerr << e.what();
                    break;
                }
            case ElementType::MeshObject:
                try {
                    UIMeshObject* elem1 = static_cast<UIMeshObject*>(p->second.first[0]); //l'image avant
                    MeshObject* elem2 = static_cast<MeshObject*>(p->second.first[1]); //l'image apres

                    std::set<std::string> description = DataComparator::Compare(*elem1, *elem2);

                    for (std::string i : description)
                    {
                        p->second.second.insert(i);
                    }
                    break;
                }
                catch (const std::exception& e)
                {
                    std::cerr << e.what();
                    break;
                }
            case ElementType::PipeToPipeMeasure:
                try {
                    UIPipeToPipeMeasure* elem1 = static_cast<UIPipeToPipeMeasure*>(p->second.first[0]); //l'image avant
                    PipeToPipeMeasure* elem2 = static_cast<PipeToPipeMeasure*>(p->second.first[1]); //l'image apres

                    std::set<std::string> description = DataComparator::Compare(*elem1, *elem2);

                    for (std::string i : description)
                    {
                        p->second.second.insert(i);
                    }
                    break;
                }
                catch (const std::exception& e)
                {
                    std::cerr << e.what();
                    break;
                }
            case ElementType::PipeToPlaneMeasure:
                try {
                    UIPipeToPlaneMeasure* elem1 = static_cast<UIPipeToPlaneMeasure*>(p->second.first[0]); //l'image avant
                    PipeToPlaneMeasure* elem2 = static_cast<PipeToPlaneMeasure*>(p->second.first[1]); //l'image apres

                    std::set<std::string> description = DataComparator::Compare(*elem1, *elem2);

                    for (std::string i : description)
                    {
                        p->second.second.insert(i);
                    }
                    break;
                }
                catch (const std::exception& e)
                {
                    std::cerr << e.what();
                    break;
                }
            case ElementType::PointToPipeMeasure:
                try {
                    UIPointToPipeMeasure* elem1 = static_cast<UIPointToPipeMeasure*>(p->second.first[0]); //l'image avant
                    PointToPipeMeasure* elem2 = static_cast<PointToPipeMeasure*>(p->second.first[1]); //l'image apres

                    std::set<std::string> description = DataComparator::Compare(*elem1, *elem2);

                    for (std::string i : description)
                    {
                        p->second.second.insert(i);
                    }
                    break;
                }
                catch (const std::exception& e)
                {
                    std::cerr << e.what();
                    break;
                }
            case ElementType::PointToPlaneMeasure:
                try {
                    UIPointToPlaneMeasure* elem1 = static_cast<UIPointToPlaneMeasure*>(p->second.first[0]); //l'image avant
                    PointToPlaneMeasure* elem2 = static_cast<PointToPlaneMeasure*>(p->second.first[1]); //l'image apres

                    std::set<std::string> description = DataComparator::Compare(*elem1, *elem2);

                    for (std::string i : description)
                    {
                        p->second.second.insert(i);
                    }
                    break;
                }
                catch (const std::exception& e)
                {
                    std::cerr << e.what();
                    break;
                }
            case ElementType::ColumnTiltMeasure:
                try {
                    UIColumnTiltMeasure* elem1 = static_cast<UIColumnTiltMeasure*>(p->second.first[0]); //l'image avant
                    ColumnTiltMeasure* elem2 = static_cast<ColumnTiltMeasure*>(p->second.first[1]); //l'image apres

                    std::set<std::string> description = DataComparator::Compare(*elem1, *elem2);

                    for (std::string i : description)
                    {
                        p->second.second.insert(i);
                    }
                    break;
                }
                catch (const std::exception& e)
                {
                    std::cerr << e.what();
                    break;
                }
            case ElementType::BeamBendingMeasure:
                try {
                    UIBeamBendingMeasure* elem1 = static_cast<UIBeamBendingMeasure*>(p->second.first[0]); //l'image avant
                    BeamBendingMeasure* elem2 = static_cast<BeamBendingMeasure*>(p->second.first[1]); //l'image apres

                    std::set<std::string> description = DataComparator::Compare(*elem1, *elem2);

                    for (std::string i : description)
                    {
                        p->second.second.insert(i);
                    }
                    break;
                }
                catch (const std::exception& e)
                {
                    std::cerr << e.what();
                    break;
                }
            case ElementType::PolylineMeasure:
                try {
                    UIPolylineMeasure* elem1 = static_cast<UIPolylineMeasure*>(p->second.first[0]); //l'image avant
                    PolylineMeasure* elem2 = static_cast<PolylineMeasure*>(p->second.first[1]); //l'image apres

                    std::set<std::string> description = DataComparator::Compare(*elem1, *elem2);

                    for (std::string i : description)
                    {
                        p->second.second.insert(i);
                    }
                    break;
                }
                catch (const std::exception& e)
                {
                    std::cerr << e.what();
                    break;
                }
            case ElementType::SimpleMeasure:
                try {
                    UISimpleMeasure* elem1 = static_cast<UISimpleMeasure*>(p->second.first[0]); //l'image avant
                    SimpleMeasure* elem2 = static_cast<SimpleMeasure*>(p->second.first[1]); //l'image apres

                    std::set<std::string> description = DataComparator::Compare(*elem1, *elem2);

                    for (std::string i : description)
                    {
                        p->second.second.insert(i);
                    }
                    break;
                }
                catch (const std::exception& e)
                {
                    std::cerr << e.what();
                    break;
                }
            case ElementType::MasterCluster:
                try {
                    UIMasterCluster* elem1 = static_cast<UIMasterCluster*>(p->second.first[0]); //l'image avant
                    MasterCluster* elem2 = static_cast<MasterCluster*>(p->second.first[1]); //l'image apres

                    UIMasterCluster u1 = UIMasterCluster(*elem1);
                    UIMasterCluster u2 = UIMasterCluster(*elem2);

                    std::set<std::string> description = DataComparator::Compare(u1,u2);

                    for (std::string i : description)
                    {
                        p->second.second.insert(i);
                    }
                    break;
                }
                catch (const std::exception& e)
                {
                    std::cerr << e.what();
                    break;
                }
            case ElementType::Scan:
                try {
                    UIScan* elem1 = static_cast<UIScan*>(p->second.first[0]); //l'image avant
                    Scan* elem2 = static_cast<Scan*>(p->second.first[1]); //l'image apres

                    std::set<std::string> description = DataComparator::Compare(*elem1, *elem2);

                    for (std::string i : description)
                    {
                        p->second.second.insert(i);
                    }
                    break;
                }
                catch (const std::exception& e)
                {
                    std::cerr << e.what();
                    break;
                }
            case ElementType::Cluster:
                try {
                    UICluster* elem1 = static_cast<UICluster*>(p->second.first[0]); //l'image avant
                    Cluster* elem2 = static_cast<Cluster*>(p->second.first[1]); //l'image apres

                    std::set<std::string> description = DataComparator::Compare(*elem1, *elem2);

                    for (std::string i : description)
                    {
                        p->second.second.insert(i);
                    }
                    break;
                }
                catch (const std::exception& e)
                {
                    std::cerr << e.what();
                    break;
                }
            case ElementType::Tag:
                try {
                    UITag* elem1 = static_cast<UITag*>(p->second.first[0]); //l'image avant
                    Tag* elem2 = static_cast<Tag*>(p->second.first[1]); //l'image apres

                    std::set<std::string> description = DataComparator::Compare(*elem1, *elem2);

                    for (std::string i : description)
                    {
                        p->second.second.insert(i);
                    }
                    break;
                }
                catch (const std::exception& e)
                {
                    std::cerr << e.what();
                    break;
                }

            }
        }
    }
    return modification;
}
*/
