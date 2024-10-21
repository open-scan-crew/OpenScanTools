#include "controller/functionSystem/ContextExportOSTObjects.h"
#include "controller/Controller.h"
#include "controller/messages/DataIdListMessage.h"
#include "controller/messages/PrimitivesExportParametersMessage.h"
#include "controller/messages/FilesMessage.h"
#include "controller/messages/ModalMessage.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/GuiData/GuiDataIO.h"
#include "io/SaveLoadSystem.h"
#include "models/graph/MeshObjectNode.h"
#include "models/graph/APointCloudNode.h"

#include "gui/Texts.hpp"

ContextExportOSTObjects::ContextExportOSTObjects(const ContextId& id)
	: AContext(id)
	, m_openExportFolder(false)
{
	m_state = ContextState::waiting_for_input;
}

ContextExportOSTObjects::~ContextExportOSTObjects()
{}

ContextState ContextExportOSTObjects::start(Controller& controller)
{
	return m_state;
}

ContextState ContextExportOSTObjects::feedMessage(IMessage* message, Controller& controller)
{
	switch (message->getType())
	{
	case IMessage::MessageType::FILES:
		m_output = *(static_cast<FilesMessage*>(message)->m_inputFiles.begin());
		m_state = m_objectsToExport.empty() ? ContextState::waiting_for_input : ContextState::ready_for_using;
		break;
	case IMessage::MessageType::DATAID_LIST:
	{
		m_objectsToExport = static_cast<DataListMessage*>(message)->m_dataPtrs;

		if (m_objectsToExport.empty())
			return m_state = ContextState::abort;

		std::unordered_map<std::wstring, std::wstring> infoFileObjects;
		for (const SafePtr<AGraphNode>& exportPtr : m_objectsToExport)
		{
			std::wstring filePath;
			xg::Guid fileId;

			ElementType type;
			{
				ReadPtr<AGraphNode> readNode = exportPtr.cget();
				if (!readNode)
					continue;
				type = readNode->getType();
			}

			switch (type)
			{
				case ElementType::PCO:
				case ElementType::Scan:
				{
					ReadPtr<APointCloudNode> scan = static_pointer_cast<APointCloudNode>(exportPtr).cget();
					filePath = scan->getCurrentScanPath().wstring();
					fileId = scan->getScanGuid();
				}
				break;
				case ElementType::MeshObject:
				{
					ReadPtr<MeshObjectNode> mesh = static_pointer_cast<MeshObjectNode>(exportPtr).cget();
					if (!mesh)
						continue;
					filePath = mesh->getFilePath().wstring();
					fileId = mesh->getMeshId();
				}
				break;
			}

			if (fileId.isValid())
			{
				ReadPtr<AGraphNode> readNode = exportPtr.cget();
				if (!readNode)
					continue;
				infoFileObjects[readNode->getName()] = filePath;
			}
		}
		if (!infoFileObjects.empty())
		{
			controller.updateInfo(new GuiDataExportFileObjectDialog(infoFileObjects));
			m_state = ContextState::waiting_for_input;
		}
		else
			m_state = m_output.empty() ? ContextState::waiting_for_input : ContextState::ready_for_using;
	}
	break;
	case IMessage::MessageType::MODAL:
	{
		switch (static_cast<ModalMessage*>(message)->m_returnedValue)
		{
			case 0:
				m_state = ContextState::ready_for_using;
				break;
		}
	}
	break;
	case IMessage::MessageType::PRIMITIVES_EXPORT_PARAMETERS:
		m_openExportFolder = static_cast<PrimitivesExportParametersMessage*>(message)->m_parameters.openFolderWindowsAfterExport;
		break;
	}

	return m_state;
}

ContextState ContextExportOSTObjects::launch(Controller& controller)
{
	SaveLoadSystem::ExportAuthorObjects(controller, m_output, m_objectsToExport, true);
	controller.updateInfo(new GuiDataTmpMessage(TEXT_LIST_EXPORT_SUCCESS.arg(QString::fromStdWString(m_output.wstring()))));
	if (m_openExportFolder)
		controller.updateInfo(new GuiDataOpenInExplorer(m_output));
	return (m_state = ContextState::done);
}

bool ContextExportOSTObjects::canAutoRelaunch() const
{
	return (false);
}

ContextType ContextExportOSTObjects::getType() const
{
	return (ContextType::exportOpenScanTools);
}