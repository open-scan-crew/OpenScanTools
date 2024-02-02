#include "controller/controls/ControlCluster.h"
#include "controller/messages/TreeIdMessage.h"
#include "controller/ControllerContext.h"
#include "controller/Controller.h"
#include "controller/functionSystem/FunctionManager.h"
#include "utils/Logger.h"
#include "models/project/Project.h"
#include "models/data/Cluster.h"
#include "gui/GuiData/GuiDataTree.h"
#include "gui/GuiData/GuiData3dObjects.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataTree.h"

namespace control
{
	namespace cluster
	{
		CreateCluster::CreateCluster(const std::string& name, const TreeType& type, const Color32& newColor)
			: m_name(name)
			, m_type(type)
			, m_color(newColor)
			, m_cluster(nullptr)
			, m_clusterId(INVALID_DATA_ID)
		{}

		CreateCluster::~CreateCluster()
		{}

		void CreateCluster::doFunction(Controller& controller)
		{

			Project* project = controller.getContext().getCurrentProject();

			if (!project)
			{
				CONTROLLOG << "control::cluster::CreateCluster Project nullptr" << LOGENDL;
				return;
			}
			if (!m_cluster)
			{
				m_cluster = new ModelCluster();
				m_cluster->setName(m_name);
				m_cluster->setColor(m_color);
			}
		
			project->addToTree(m_type, m_cluster, project->getTree(m_type)->root()->getTreeId());
			TreeElement* elem(project->getTreeElementOnDataId(m_type, m_cluster->getId()));
			m_clusterId = m_cluster->getId();
			controller.updateInfo(new GuiDataProjectTree(controller.getContext().getCurrentProject()->getTrees(), controller.getContext().getObjectsForGui()));
			if (controller.getFunctionManager().isActiveContext() != ContextType::none)
			{
				TreeIdMessage message({ elem->getTreeId() }, TreeElementType::Cluster);
				controller.getFunctionManager().feedMessage(controller, &message);
			}
		}
		
		bool CreateCluster::canUndo()
		{
			return m_cluster != nullptr;
		}
		void CreateCluster::undoFunction(Controller& controller)
		{
			assert(m_cluster == nullptr);
			m_cluster = static_cast<ModelCluster*>(controller.getContext().getCurrentProject()->removeData(m_clusterId));
			if (!m_cluster)
				return;

			assert(m_clusterId == m_cluster->getId());

			
			controller.updateInfo(new GuiDataDeleteDataTree(m_clusterId));
			controller.updateInfo(new GuiDataObjectsToDelete({ m_clusterId }));
			CONTROLLOG << "control::CreateCluster undo " << LOGENDL;
		}

		UIControl CreateCluster::getType()
		{
			return UIControl::createCluster;
		}

	}
}
