#include "controller/functionSystem/ContextImportOSTObjects.h"
#include "controller/Controller.h"
#include "controller/controls/ControlFunction.h"
#include "controller/IControlListener.h"
#include "controller/messages/FilesMessage.h"
#include "controller/messages/ModalMessage.h"

#include "gui/GuiData/GuiDataIO.h"
#include "gui/GuiData/GuiDataMessages.h"

#include "models/application/Author.h"
#include "models/graph/GraphManager.h"
#include "models/graph/MeshObjectNode.h"
#include "models/graph/AGraphNode.h"
#include "models/graph/PointCloudNode.h"
#include "models/graph/ClusterNode.h"

#include "io/SaveLoadSystem.h"
#include "gui/texts/ContextTexts.hpp"

#include "utils/Logger.h"

#define Ok 0x00000400

ContextImportOSTObjects::ContextImportOSTObjects(const ContextId& id)
	: AContext(id)
{
	m_state = ContextState::waiting_for_input;
}

ContextImportOSTObjects::~ContextImportOSTObjects()
{
}

ContextState ContextImportOSTObjects::start(Controller& controller)
{
	return m_state;
}

ContextState ContextImportOSTObjects::feedMessage(IMessage* message, Controller& controller)
{
	switch (message->getType())
	{
		case IMessage::MessageType::FILES:
		{
			FilesMessage* fm = static_cast<FilesMessage*>(message);
			std::vector<std::filesystem::path> inputFiles = fm->m_inputFiles;
			if (fm->m_typeOfInput == 0)
			{
				m_importFiles = inputFiles;
				m_state = ContextState::ready_for_using;
			}
			else
			{
				if (!fm->m_inputFiles.empty())
				{
					m_folder = *(fm->m_inputFiles.begin());
					m_state = ContextState::ready_for_using;
				}
			}
		}
		break;
		case IMessage::MessageType::MODAL:
		{
			switch (static_cast<ModalMessage*>(message)->m_returnedValue)
			{
				case 0:
					m_missingFileObjects.clear();
					m_state = ContextState::ready_for_using;
					break;
			}
		}
		break;
	}

	return m_state;
}

ContextState ContextImportOSTObjects::launch(Controller& controller)
{
	CONTROLLOG << "control::io::ImportOSTObjects" << LOGENDL;

	if (m_importFiles.empty())
		return m_state = ContextState::abort;

	if (m_importData.empty())
	{
		SaveLoadSystem::ImportAuthorObjects(m_importFiles, m_importData, m_missingFileObjects, controller);

		if (m_importData.empty())
			return m_state = ContextState::abort;
	}

	if(!m_folder.empty())
	{
		m_missingFileObjects = SaveLoadSystem::LoadFileObjects(controller, m_missingFileObjects, m_folder, true);
		m_folder.clear();
	}

	if (!m_missingFileObjects.empty())
	{
		controller.updateInfo(new GuiDataImportFileObjectDialog(m_missingFileObjects, false));
		return m_state = ContextState::waiting_for_input;
	}
	else
		addObjectsToProject(controller, m_importData);

	controller.resetHistoric();

	return m_state = ContextState::done;
}

bool ContextImportOSTObjects::canAutoRelaunch() const
{
	return false;
}

ContextType ContextImportOSTObjects::getType() const
{
	return ContextType::importOSTObjects;
}


void ContextImportOSTObjects::addObjectsToProject(Controller& controller, const std::unordered_set<SafePtr<AGraphNode>>& objects)
{
	std::unordered_map<TreeType, std::unordered_map<SafePtr<Author>, std::vector<SafePtr<AGraphNode>>>> importedObjects;
	for (const SafePtr<AGraphNode>& obj : objects)
	{
		ReadPtr<AGraphNode> rObj = obj.cget();
		if (!rObj)
			continue;

		TreeType ttype = rObj->getDefaultTreeType();

		if (importedObjects.find(ttype) == importedObjects.end())
			importedObjects[ttype] = std::unordered_map<SafePtr<Author>, std::vector<SafePtr<AGraphNode>>>();
		if (importedObjects[ttype].find(rObj->getAuthor()) == importedObjects[ttype].end())
			importedObjects[ttype][rObj->getAuthor()] = std::vector<SafePtr<AGraphNode>>();

		importedObjects[ttype][rObj->getAuthor()].push_back(obj);
	}

	std::unordered_set<SafePtr<AGraphNode>> toAddNodes = objects;
	for (auto ttype : importedObjects)
	{
		for (auto authObjs : ttype.second)
		{
			std::wstring authorName = L"NO_AUTHOR";
			{
				ReadPtr<Author> rAuth = authObjs.first.cget();
				if (rAuth)
					authorName = rAuth->getName();
			}

			SafePtr<ClusterNode> newAuthorCluster = make_safe<ClusterNode>();
			{
				WritePtr<ClusterNode> wCluster = newAuthorCluster.get();
				if (wCluster)
				{
					wCluster->setName(authorName);
					wCluster->setTreeType(ttype.first);
				}
			}

			for (const SafePtr<AGraphNode>& obj : authObjs.second)
				AGraphNode::addOwningLink(newAuthorCluster, obj);

			toAddNodes.insert(newAuthorCluster);
		}
	}

	controller.getControlListener()->notifyUIControl(new control::function::AddNodes(toAddNodes));
}

// ContextLinkOSTObjects

ContextLinkOSTObjects::ContextLinkOSTObjects(const ContextId& id)
	: AContext(id)
{}

ContextLinkOSTObjects::~ContextLinkOSTObjects()
{}

ContextState ContextLinkOSTObjects::start(Controller& controller)
{
	for (const SafePtr<AGraphNode>& projObj : controller.getGraphManager().getProjectNodes())
	{
		ElementType type;
		{
			ReadPtr<AGraphNode> readObj = projObj.cget();
			if (!readObj)
				continue;
			type = readObj->getType();

		}

		bool insertData = false;
		switch (type)
		{
			case ElementType::Scan:
			case ElementType::PCO:
			{
				ReadPtr<PointCloudNode> scan = static_pointer_cast<PointCloudNode>(projObj).cget();
				if (!scan)
					continue;
				insertData = (scan->getScanGuid() == tls::ScanGuid());
			}
			break;
			case ElementType::MeshObject:
			{
				ReadPtr<MeshObjectNode> mesh = static_pointer_cast<MeshObjectNode>(projObj).cget();
				if (!mesh)
					continue;
				insertData = (mesh->getMeshId() == xg::Guid());
			}
			break;
		}

		if (insertData)
		{
			m_currentMissing.insert(projObj);
			m_startMissingFile.insert(projObj);
		}
	}

	if (m_currentMissing.empty())
	{
		controller.updateInfo(new GuiDataModal(Ok, TEXT_IMPORT_NO_MISSING_FILES));
		m_state = ContextState::done;
	}
	else
		m_state = ContextState::ready_for_using;


	return m_state;
}

ContextState ContextLinkOSTObjects::feedMessage(IMessage* message, Controller& controller)
{
	switch (message->getType())
	{
		case IMessage::MessageType::FILES:
		{
			FilesMessage* fm = static_cast<FilesMessage*>(message);
			if (!fm->m_inputFiles.empty())
			{
				m_folder = *(fm->m_inputFiles.begin());
				m_state = ContextState::ready_for_using;
			}
		}
		break;

		/*case IMessage::MessageType::DATAID_LIST:
		{
			DataListMessage* dm = static_cast<DataListMessage*>(message);
			m_currentMissing = static_cast<DataListMessage*>(message)->m_dataPtrs;
			m_state = ContextState::ready_for_using;
		}
		break;*/
	}

	return m_state;
}

ContextState ContextLinkOSTObjects::abort(Controller& controller)
{
	finish(controller);
	return AContext::abort(controller);
}

ContextState ContextLinkOSTObjects::launch(Controller& controller)
{
	if (!m_folder.empty())
	{
		m_currentMissing = SaveLoadSystem::LoadFileObjects(controller, m_currentMissing, m_folder, true);
		m_folder.clear();
	}

	if (!m_currentMissing.empty())
	{
		controller.updateInfo(new GuiDataImportFileObjectDialog(m_currentMissing, true));
		return m_state = ContextState::waiting_for_input;
	}

	finish(controller);

	return m_state = ContextState::done;
}

bool ContextLinkOSTObjects::canAutoRelaunch() const
{
	return false;
}

ContextType ContextLinkOSTObjects::getType() const
{
	return ContextType::linkFileOSTObjects;
}

void ContextLinkOSTObjects::finish(Controller& controller)
{
	std::unordered_set<SafePtr<AGraphNode>> linkedDataNodes = m_startMissingFile;
	for (const SafePtr<AGraphNode>& id : m_currentMissing)
		linkedDataNodes.erase(id);

	controller.actualizeTreeView(linkedDataNodes);
}
