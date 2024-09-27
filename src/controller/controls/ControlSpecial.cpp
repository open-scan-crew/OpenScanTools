#include "controller/controls/ControlSpecial.h"
#include "controller/controls/ControlProject.h"
#include "controller/messages/UndoRedoMessages.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "gui/GuiData/GuiDataTree.h"
#include "gui/GuiData/GuiData3dObjects.h"
#include "gui/Texts.hpp"

#include "controller/Controller.h"
#include "controller/FilterSystem.h"
#include "controller/functionSystem/FunctionManager.h"
#include "controller/ControlListener.h"

#include "pointCloudEngine/PCE_core.h"

#include "models/3d/Graph/ScanNode.h"
#include "models/3d/Graph/ScanObjectNode.h"
#include "models/3d/Graph/MeshObjectNode.h"

#include "controller/controls/ControlMeshObject.h"
#include "vulkan/MeshManager.h"

#include "models/Types.hpp"
#include "utils/Logger.h"

#include <QMessageBox>

#include "io/SaveLoadSystem.h"
#include "models/3d/Graph/OpenScanToolsGraphManager.hxx"


namespace control::special
{
    /*
    ** DeleteElement
    */
	
    DeleteElement::DeleteElement(std::unordered_set<SafePtr<AGraphNode>> datasToDelete, bool isUndoAble)
    {
        m_datasToDelete = datasToDelete;
    }

    DeleteElement::~DeleteElement()
    {
		for (SafePtr<AGraphNode> node : m_elemsDeleted)
		{
			node.destroy();
		}
		//To do real delete data
    }
	
    void DeleteElement::doFunction(Controller& controller)
    {
        CONTROLLOG << "control::special::DeleteElement do" << LOGENDL;

		for(const SafePtr<AGraphNode>& obj : m_datasToDelete)
		{ 
			ElementType type;
			{
				WritePtr<AGraphNode> writeNode = obj.get();
				if (!writeNode)
					return;
				writeNode->setDead(true);
				m_elemsDeleted.insert(obj);
				type = writeNode->getType();
			}

			//A gérer dans le setDead() ?
			if (type == ElementType::Scan)
			{
				WritePtr<ScanNode> scan = static_pointer_cast<ScanNode>(obj).get();
				if (!scan)
					continue;
				scan->freeScanFile();
			}

			if (type == ElementType::PCO)
			{
				tls::ScanGuid scanGuid;
				SafePtr<ScanObjectNode> scan = static_pointer_cast<ScanObjectNode>(obj);
				{
					ReadPtr<ScanObjectNode> rScan = scan.cget();
					if (!rScan)
						continue;
					scanGuid = rScan->getScanGuid();
				}

				if (controller.getOpenScanToolsGraphManager().getPCOcounters(scanGuid) == 0)
				{
					WritePtr<ScanObjectNode> wScan = scan.get();
					if (!wScan)
						continue;
					wScan->freeScanFile();
				}
			}

		}

		controller.changeSelection({});
		controller.actualizeTreeView(m_elemsDeleted);

        CONTROLLOG << "control::special::DeleteElement delete " << m_elemsDeleted.size() << " elements" << LOGENDL;
    }

    bool DeleteElement::canUndo() const
    {
		return !m_elemsDeleted.empty();
    }

    void DeleteElement::undoFunction(Controller& controller)
    {
		std::unordered_set<SafePtr<AGraphNode>> toActualize;
		std::unordered_set<SafePtr<AGraphNode>> fileObjectsToReload;
		for (const SafePtr<AGraphNode>& pElem : m_elemsDeleted)
		{
			WritePtr<AGraphNode> readElem = pElem.get();
			if (!readElem)
				continue;

			readElem->setDead(false);

			switch (readElem->getType())
			{
				case ElementType::MeshObject:
				case ElementType::PCO:
				case ElementType::Scan:
					fileObjectsToReload.insert(pElem);
					break;

			}

			toActualize.insert(pElem);
		}

		m_elemsDeleted.clear();

		SaveLoadSystem::LoadFileObjects(controller, fileObjectsToReload, "", false);

        controller.changeSelection({});
		controller.actualizeTreeView(m_elemsDeleted);


		CONTROLLOG << "control::special::DeleteElement undo" << LOGENDL;
    }

    ControlType DeleteElement::getType() const
    {
        return (ControlType::deleteElementSpecial);
    }


	//
	// DeleteTotal
	//

	DeleteTotalData::DeleteTotalData(std::unordered_set<SafePtr<AGraphNode>> datasToDelete)
	{
		m_objectsToDelete = datasToDelete;
	}

	DeleteTotalData::~DeleteTotalData()
	{

	}

	void DeleteTotalData::doFunction(Controller& controller)
	{
		for (SafePtr<AGraphNode> pNode : m_objectsToDelete)
		{
			ElementType type;
			{
				ReadPtr<AGraphNode> readNode = pNode.cget();
				if (!readNode)
					continue;
				type = readNode->getType();
			}

			switch (type)
			{
				case ElementType::Scan:
				case ElementType::PCO:
				{
					WritePtr<APointCloudNode> scan = static_pointer_cast<APointCloudNode>(pNode).get();
					if (!scan)
						continue;

					CONTROLLOG << "The file " << scan->getCurrentScanPath() << " -- {" << scan->getScanGuid() << "} will be definitly deleted." << LOGENDL;
					scan->eraseScanFile();
					break;
				}
				case ElementType::MeshObject:
				{
					ReadPtr<MeshObjectNode> meshObj = static_pointer_cast<MeshObjectNode>(pNode).cget();
					if (!meshObj)
						continue;

					CONTROLLOG << "The file " << meshObj->getFilePath() << " -- {" << meshObj->getMeshId() << "} will be definitly deleted." << LOGENDL;
					std::filesystem::path filePath = controller.getContext().cgetProjectInternalInfo().getObjectsFilesFolderPath() / meshObj->getFilePath().filename();

					if (!std::filesystem::exists(filePath) || std::filesystem::is_directory(filePath))
						break;

					if (std::filesystem::remove(filePath))
					{
						Logger::log(IOLog) << "INFO - the file " << filePath << " has been successfully removed." << Logger::endl;
					}
					else
					{
						Logger::log(IOLog) << "WARNING - the file " << filePath << " cannot be removed from the file system. The file may be accessed elsewhere." << Logger::endl;
					}
					break;
				}
			}

			AGraphNode::cleanLinks(pNode);
		}

		controller.changeSelection({});
		controller.actualizeTreeView(m_objectsToDelete);

		controller.getControlListener()->notifyUIControl(new control::project::StartSave());
		controller.resetHistoric();
	}

	bool DeleteTotalData::canUndo() const
	{
		return (false);
	}

	void DeleteTotalData::undoFunction(Controller& controller)
	{}

	ControlType DeleteTotalData::getType() const
	{
		return (ControlType::deleteTotal);
	}

	
    /*
    ** DeleteSelectedElements
    */

    DeleteSelectedElements::DeleteSelectedElements(bool supprScan) : m_supprScan(supprScan)
    {}

    DeleteSelectedElements::~DeleteSelectedElements()
    {

    }

    void DeleteSelectedElements::doFunction(Controller& controller)
    {
		OpenScanToolsGraphManager& graphManager = controller.getOpenScanToolsGraphManager();

		std::unordered_set<SafePtr<AGraphNode>> toDeletes;
		
		std::unordered_set<SafePtr<AGraphNode>> importantDatas;
		std::unordered_set<SafePtr<AGraphNode>> otherDatas;

		std::unordered_map<MeshId, std::unordered_set<SafePtr<AGraphNode>>> meshIdToMeshObjects;
		std::unordered_map<tls::ScanGuid, std::unordered_set<SafePtr<AGraphNode>>> scanObjPathToTls;


		for (const SafePtr<AGraphNode>& node : graphManager.getSelectedNodes())
		{
			ElementType type;
			{
				ReadPtr<AGraphNode> readNode = node.cget();
				if (!readNode)
					continue;
				type = readNode->getType();
			}

			switch (type)
			{
				case ElementType::Cluster:
				{
					bool deleteCluster = true;
					std::unordered_set<SafePtr<AGraphNode>> recChildren;
					recChildren.merge(AGraphNode::getOwningChildren_rec(node));

					if (!m_supprScan)
					{
						for (const SafePtr<AGraphNode>& obj : recChildren)
						{
							ReadPtr<AGraphNode> readObj = obj.cget();
							if (readObj && readObj->getType() == ElementType::Scan)
								deleteCluster = false;
						}
					}

					if (deleteCluster)
					{
						toDeletes.insert(node);
						toDeletes.merge(recChildren);
					}
				}
				break;
				case ElementType::Scan:
				{
					if(m_supprScan)
						toDeletes.insert(node);
				}
				break;
				case ElementType::MasterCluster:
					break;
				default:
				{
					toDeletes.insert(node);
					break;
				}
			}
		}


        for (const SafePtr<AGraphNode>& toDelete : toDeletes)
        {
			ElementType type;
			{
				ReadPtr<AGraphNode> readNode = toDelete.cget();
				if (!readNode)
					continue;
				type = readNode->getType();
			}
            
			switch (type)
            {
				case ElementType::None:
					{
						CONTROLLOG << "Warning: trying to delete a data missing" << LOGENDL;
						assert(false);
					}
					break;
				case ElementType::MasterCluster:
					break;
				case ElementType::Scan:
				{
					ReadPtr<ScanNode> scan = static_pointer_cast<ScanNode>(toDelete).cget();
					if (!scan)
						continue;
					if(scan->getScanGuid().isValid())
						importantDatas.insert(toDelete);
					else
						otherDatas.insert(toDelete);
				}
				break;
				case ElementType::PCO:
				{
					ReadPtr<ScanObjectNode> scanObj = static_pointer_cast<ScanObjectNode>(toDelete).cget();
					if (!scanObj)
						continue;
					tls::ScanGuid scanGuid = scanObj->getScanGuid();
					if (scanObjPathToTls.find(scanGuid) != scanObjPathToTls.end())
						scanObjPathToTls[scanGuid].insert(toDelete);
					else
						scanObjPathToTls[scanGuid] = { toDelete };
				}
				break;
				case ElementType::MeshObject:
				{
					ReadPtr<MeshObjectNode> meshObject = static_pointer_cast<MeshObjectNode>(toDelete).cget();
					if (!meshObject)
						continue;
					xg::Guid meshId = meshObject->getMeshId();
					if (meshIdToMeshObjects.find(meshId) != meshIdToMeshObjects.end())
						meshIdToMeshObjects[meshId].insert(toDelete);
					else
						meshIdToMeshObjects[meshId] = { toDelete };
					break;
				}
				default:
					otherDatas.insert(toDelete);
					break;
		    }
        }

		for (auto meshElement : meshIdToMeshObjects)
		{
			MeshManager& manager = MeshManager::getInstance();
			if (meshElement.first.isValid() && manager.getMeshCounters(meshElement.first) <= meshElement.second.size())
				importantDatas.insert(meshElement.second.begin(), meshElement.second.end());
			else
				otherDatas.insert(meshElement.second.begin(), meshElement.second.end());
		}

		for (auto scanObjElement : scanObjPathToTls)
		{
			if (scanObjElement.first.isValid() && graphManager.getPCOcounters(scanObjElement.first) <= scanObjElement.second.size())
				importantDatas.insert(scanObjElement.second.begin(), scanObjElement.second.end());
			else
				otherDatas.insert(scanObjElement.second.begin(), scanObjElement.second.end());
		}

		if (!importantDatas.empty())
		{
			std::unordered_map<SafePtr<AGraphNode>, std::pair<QString, QString>> importantObject;
			for (const SafePtr<AGraphNode>& importantData : importantDatas)
			{
				ElementType type;
				std::wstring name;
				{
					ReadPtr<AGraphNode> readNode = importantData.cget();
					if (!readNode)
						continue;
					type = readNode->getType();
					name = readNode->getName();
				}

				std::filesystem::path filePath = "";
				switch (type)
				{
					case ElementType::PCO:
					case ElementType::Scan:
					{
						ReadPtr<APointCloudNode> scan = static_pointer_cast<APointCloudNode>(importantData).cget();
						if (scan)
							filePath = scan->getCurrentScanPath();
						break;
					}
					case ElementType::MeshObject:
					{
						ReadPtr<MeshObjectNode> meshObject = static_pointer_cast<MeshObjectNode>(importantData).cget();
						if (meshObject)
							filePath = controller.getContext().cgetProjectInternalInfo().getObjectsFilesFolderPath() / meshObject->getFilePath().filename();
						break;
					}
				}

				if (!filePath.empty() && std::filesystem::exists(filePath))
					importantObject[importantData] = { QString::fromStdWString(name), QString::fromStdWString(filePath.wstring()) };

				if (!std::filesystem::exists(filePath))
					otherDatas.insert(importantData);
			}
			controller.updateInfo(new GuiDataDeleteFileDependantObjectDialog(importantObject, otherDatas));
		}
		else if (otherDatas.size() > 0)
			controller.getControlListener()->notifyUIControl(new control::special::DeleteElement(otherDatas, true));

        CONTROLLOG << "control::special::DeleteSelectedElements do" << LOGENDL;
    }

    bool DeleteSelectedElements::canUndo() const
    {
		return false;
    }

    void DeleteSelectedElements::undoFunction(Controller& controller)
    {
     
    }

    ControlType DeleteSelectedElements::getType() const
    {
        return (ControlType::deleteSelectedElementSpecial);
    }

	/*
	** MultiSelect
	*/

	MultiSelect::MultiSelect(const std::unordered_set<SafePtr<AGraphNode>>& nodesToSelect, bool updateTree)
		: m_nodesToSelect(nodesToSelect)
		, m_updateTree(updateTree)
	{
	}

	MultiSelect::~MultiSelect()
	{ }

	void MultiSelect::doFunction(Controller& controller)
	{
		/*if (controller.getFunctionManager().isActiveContext() != ContextType::none)
			return;
			*/

		controller.changeSelection(m_nodesToSelect, m_updateTree);
	}

	bool MultiSelect::canUndo() const
	{
		return (false);
	}

	void MultiSelect::undoFunction(Controller& controller)
	{ }

	ControlType MultiSelect::getType() const
	{
		return (ControlType::multiSelect);
	}


	/*
	** ShowHideDatas
	*/

	ShowHideDatas::ShowHideDatas()
	{}

	ShowHideDatas::ShowHideDatas(std::unordered_set<SafePtr<AGraphNode>> datasToShowHide, bool state, bool isUndoable)
		: m_state(state)
		, m_datasToShowHide(datasToShowHide)
		, m_isUndoable(isUndoable)
	{
	}

	ShowHideDatas::~ShowHideDatas()
	{}

	void ShowHideDatas::doFunction(Controller & controller)
	{
		std::unordered_set<SafePtr<AGraphNode>> tempDatasToShowHide = m_datasToShowHide;

		for (const SafePtr<AGraphNode>& objToDel : tempDatasToShowHide)
			m_datasToShowHide.merge(AGraphNode::getOwningChildren_rec(objToDel));

		for (const SafePtr<AGraphNode>& data : m_datasToShowHide)
		{
			WritePtr<AGraphNode> writeData = data.get();
			if (writeData && writeData->isVisible() != m_state) {
				writeData->setVisible(m_state);
				m_toUndoDatas.insert(data);
			}
		}

		controller.actualizeTreeView(m_toUndoDatas);

		CONTROLLOG << "control::special::ShowHideDatas do" << LOGENDL;
	}
	bool ShowHideDatas::canUndo() const
	{
		return (m_isUndoable && !m_toUndoDatas.empty());
	}
	void ShowHideDatas::undoFunction(Controller & controller)
	{
		for (const SafePtr<AGraphNode>& data : m_toUndoDatas)
		{
			WritePtr<AGraphNode> writeData = data.get();
			if (!writeData)
				continue;
			writeData->setVisible(!m_state);
		}

		controller.actualizeTreeView(m_toUndoDatas);

		m_toUndoDatas.clear();

		CONTROLLOG << "control::special::ShowHideDatas undo" << LOGENDL;
	}

	void ShowHideDatas::redoFunction(Controller& controller)
	{
		for (const SafePtr<AGraphNode>& data : m_datasToShowHide)
		{
			WritePtr<AGraphNode> writeData = data.get();
			if (writeData && writeData->isVisible() != m_state) {
				writeData->setVisible(m_state);
				m_toUndoDatas.insert(data);
			}
		}

		controller.actualizeTreeView(m_datasToShowHide);

		CONTROLLOG << "control::special::ShowHideDatas redo" << LOGENDL;
	}

	ControlType ShowHideDatas::getType() const
	{
		return ControlType::showhideDatas;
	}


    //
    // ShowHideObjects
    //

    ShowHideObjects::ShowHideObjects(const std::unordered_set<ElementType>& ObjType, const bool& value)
        : m_types(ObjType)
    {
        m_isUndoable = false;
        m_state = value;
    }

    ShowHideObjects::~ShowHideObjects()
    {}

    void ShowHideObjects::doFunction(Controller& controller)
    {
		m_datasToShowHide = controller.getOpenScanToolsGraphManager().getNodesByTypes(m_types);
        ShowHideDatas::doFunction(controller);
		
	    CONTROLLOG << "control::special::ShowHideObjects to " << m_state << LOGENDL;
    }

    bool ShowHideObjects::canUndo() const
    {
        return (false);
    }

    void ShowHideObjects::undoFunction(Controller& controller)
    {
    }

    ControlType ShowHideObjects::getType() const
    {
        return (ControlType::showHideObjects);
    }

    /*
    ** ShowHideCurrentObjects
    */

    ShowHideCurrentObjects::ShowHideCurrentObjects(bool state)
    {
        m_isUndoable = true;
        m_state = state;
    }

    ShowHideCurrentObjects::~ShowHideCurrentObjects()
    {
    }

    void ShowHideCurrentObjects::doFunction(Controller& controller)
    {
		m_datasToShowHide = controller.getOpenScanToolsGraphManager().getSelectedNodes();
        ShowHideDatas::doFunction(controller);
        CONTROLLOG << "control::special::ShowHideCurrentObjects to " << m_state << LOGENDL;
    }

    void ShowHideCurrentObjects::undoFunction(Controller& controller)
    {
		ShowHideDatas::undoFunction(controller);
	}

	void ShowHideCurrentObjects::redoFunction(Controller& controller)
	{
		ShowHideDatas::redoFunction(controller);
	}

    ControlType ShowHideCurrentObjects::getType() const
    {
        return (ControlType::showhideCurrentMarkers);
    }

    /*
    ** ShowHideUncurrentObjects
    */

    ShowHideUncurrentObjects::ShowHideUncurrentObjects(bool state)
    {
        m_isUndoable = true;
        m_state = state;
    }

    ShowHideUncurrentObjects::~ShowHideUncurrentObjects()
    {
    }

    void ShowHideUncurrentObjects::doFunction(Controller& controller)
    {
		m_datasToShowHide = controller.getOpenScanToolsGraphManager().getUnSelectedNodes();

		for (const SafePtr<AGraphNode>& data : controller.getOpenScanToolsGraphManager().getSelectedNodes())
		{
			std::unordered_set<SafePtr<AGraphNode>> recOwningParents = AGraphNode::getOwningParents_rec(data);
			for (const SafePtr<AGraphNode>& parent : recOwningParents)
				m_datasToShowHide.erase(parent);
		}
		ShowHideDatas::doFunction(controller);

        CONTROLLOG << "control::special::ShowHideCurrentObjects to " << m_state << LOGENDL;
    }

    void ShowHideUncurrentObjects::undoFunction(Controller& controller)
    {
		ShowHideDatas::undoFunction(controller);
	}

	void ShowHideUncurrentObjects::redoFunction(Controller& controller)
	{
		ShowHideDatas::redoFunction(controller);
	}

    ControlType ShowHideUncurrentObjects::getType() const
    {
        return (ControlType::showhideUncurrentMarkers);
    }

    /*
    ** ShowAll
    */

    ShowAll::ShowAll(bool state)
    {
        m_isUndoable = false;
        m_state = state;
    }

    ShowAll::~ShowAll()
    {
    }

    void ShowAll::doFunction(Controller& controller)
    {
		m_datasToShowHide = controller.getOpenScanToolsGraphManager().getProjectNodes();
        ShowHideDatas::doFunction(controller);
        
        CONTROLLOG << "control::special::ShowAll to " << m_state << LOGENDL;
    }

    void ShowAll::undoFunction(Controller& controller)
    {
		ShowHideDatas::undoFunction(controller);
	}

	void ShowAll::redoFunction(Controller& controller)
	{
		ShowHideDatas::redoFunction(controller);
	}

    ControlType ShowAll::getType() const
    {
        return (ControlType::showhideAll);
    }
}