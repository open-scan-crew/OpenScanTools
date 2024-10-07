#include "controller/controls/ControlDataEdition.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "gui/GuiData/GuiDataTree.h"
#include "utils/StrCompare.h"
#include "utils/Logger.h"

#include "models/graph/AObjectNode.h"
#include "controller/controls/AEditionControl.hxx"

#include "models/graph/GraphManager.hxx"

namespace control
{
	namespace dataEdition
	{
		/*
		** SetColor
		*/

		SetColor::SetColor(SafePtr<AObjectNode> toEditData, const Color32& newColor)
			: ATEditionControl({ toEditData }, newColor, "SetColor", &Data::setColor, &Data::getColor)
		{
			m_actualize_tree_view = true;
		}

		SetColor::SetColor(const std::unordered_set<SafePtr<AObjectNode>> & toEditDatas, const Color32& newColor)
			: ATEditionControl(toEditDatas, newColor, "SetColor", &AObjectNode::setColor, &AObjectNode::getColor)
		{
			m_actualize_tree_view = true;
		}

		SetColor::~SetColor()
		{
		}

		ControlType SetColor::getType() const
		{
			return (ControlType::setColorEdit);
		}

		/*
		** SetUserId
		*/

		SetUserId::SetUserId(SafePtr<AObjectNode> toEditData, uint32_t newId)
			: AEditionControl()
			, m_toEditData(toEditData)
			, m_newId(newId)
		{}

		SetUserId::~SetUserId()
		{}

		bool SetUserId::changeId(Controller & controller, uint32_t toChangeId, std::string actionText)
		{
			if (!controller.getGraphManager().isIdAvailable({ m_type }, toChangeId))
			{
				CONTROLLOG << "control::SetUserId " << actionText << " : id unavailable" << Logger::endl;
				return false;
			}

			{
				WritePtr<AObjectNode> doWriteData = m_toEditData.get();
				if (!doWriteData)
				{
					CONTROLLOG << "control::SetUserId " << actionText << " : data null" << Logger::endl;
					return false;
				}

				if (doWriteData->getUserIndex() == toChangeId)
				{
					CONTROLLOG << "control::SetUserId " << actionText << " : same value" << Logger::endl;
					return false;
				}

				doWriteData->setUserIndex(toChangeId);
				doTimeModified(*&doWriteData);
			}

			controller.actualizeTreeView(m_toEditData);

			return true;
		}

		void SetUserId::doFunction(Controller& controller)
		{
			{
				ReadPtr<AObjectNode> rObj = m_toEditData.cget();
				if (!rObj)
				{
					CONTROLLOG << "control::SetUserId do : data null" << Logger::endl;
					m_toEditData.reset();
					return;
				}
				m_type = rObj->getType();
				m_oldId = rObj->getUserIndex();
			}

			if (!changeId(controller, m_newId, "do"))
			{
				m_toEditData.reset();
				return;
			}

			CONTROLLOG << "control::SetUserId do" << Logger::endl;
		}

		bool SetUserId::canUndo() const
		{
			return bool(m_toEditData);
		}

		void SetUserId::undoFunction(Controller& controller)
		{
			if (!changeId(controller, m_oldId, "undo"))
			{
				m_toEditData.reset();
				return;
			}

			CONTROLLOG << "control::SetUserId undo" << Logger::endl;
		}

		void SetUserId::redoFunction(Controller& controller)
		{
			if (!changeId(controller, m_newId, "redo"))
			{
				m_toEditData.reset();
				return;
			}

			CONTROLLOG << "control::SetUserId redo" << Logger::endl;
		}

		ControlType SetUserId::getType() const
		{
			return (ControlType::setUserIdEdit);
		}

		/*
		** SetDescription
		*/

		SetDescription::SetDescription(SafePtr<AObjectNode> toEditData, const std::wstring& newDesc)
			: ATEditionControl({ toEditData }, newDesc, "SetDescription", & AObjectNode::setDescription, & AObjectNode::getDescription)
		{
		}

		SetDescription::SetDescription(const std::unordered_set<SafePtr<AObjectNode>>& toEditDatas, const std::wstring& newDesc)
			: ATEditionControl(toEditDatas, newDesc, "SetDescription", &AObjectNode::setDescription, &AObjectNode::getDescription)
		{
		}

		SetDescription::~SetDescription()
		{
		}

		ControlType SetDescription::getType() const
		{
			return (ControlType::setDescriptionEdit);
		}

		/*
		** setDiscipline
		*/

		SetDiscipline::SetDiscipline(SafePtr<AObjectNode> toEditData, const std::wstring& discipline)
			: ATEditionControl({ toEditData }, discipline, "SetDiscipline", &AObjectNode::setDiscipline, &AObjectNode::getDiscipline)
		{
		}

		SetDiscipline::SetDiscipline(const std::unordered_set<SafePtr<AObjectNode>>& toEditDatas, const std::wstring& discipline)
			: ATEditionControl(toEditDatas, discipline, "SetDiscipline", &AObjectNode::setDiscipline, &AObjectNode::getDiscipline)
		{
		}

		SetDiscipline::~SetDiscipline()
		{
		}

		ControlType SetDiscipline::getType() const
		{
			return (ControlType::setDisciplineEdit);
		}

		/*
		** SetPhase
		*/

		SetPhase::SetPhase(SafePtr<AObjectNode> toEditData, const std::wstring& prefix)
			: ATEditionControl({ toEditData }, prefix, "SetPhase", &AObjectNode::setPhase, &AObjectNode::getPhase)
		{
		}

		SetPhase::SetPhase(const std::unordered_set<SafePtr<AObjectNode>>& toEditDatas, const std::wstring& prefix)
			: ATEditionControl(toEditDatas, prefix, "SetPhase", &AObjectNode::setPhase, &AObjectNode::getPhase)
		{
		}

		SetPhase::~SetPhase()
		{ }

		ControlType SetPhase::getType() const
		{
			return (ControlType::setPhaseEdit);
		}

		/*
		** SetIdentifier
		*/

		SetIdentifier::SetIdentifier(SafePtr<AObjectNode> toEditData, const std::wstring& identifer)
			: ATEditionControl({ toEditData }, identifer, "SetIdentifier", &AObjectNode::setIdentifier, &AObjectNode::getIdentifier)
		{
			m_actualize_tree_view = true;
		}

		SetIdentifier::SetIdentifier(const std::unordered_set<SafePtr<AObjectNode>>& toEditDatas, const std::wstring& identifer)
			: ATEditionControl(toEditDatas, identifer, "SetIdentifier", &AObjectNode::setIdentifier, &AObjectNode::getIdentifier)
		{
			m_actualize_tree_view = true;
		}

		SetIdentifier::~SetIdentifier()
		{ }

		ControlType SetIdentifier::getType() const
		{
			return (ControlType::setIdentifierEdit);
		}

		/*
		** SetName
		*/

		SetName::SetName(SafePtr<AObjectNode> toEditData, const std::wstring& name)
			: ATEditionControl({ toEditData }, name, "SetName", &AObjectNode::setName, &AObjectNode::getName)
		{
			m_actualize_tree_view = true;
		}

		SetName::SetName(const std::unordered_set<SafePtr<AObjectNode>>& toEditDatas, const std::wstring& name)
			: ATEditionControl(toEditDatas, name, "SetName", &AObjectNode::setName, &AObjectNode::getName)
		{
			m_actualize_tree_view = true;
		}

		SetName::~SetName()
		{
		}

		ControlType SetName::getType() const
		{
			return (ControlType::setNameEdit);
		}

		/*
		** SetHyperLinks
		*/

		SetHyperLinks::SetHyperLinks(SafePtr<AObjectNode> toEditData, const std::vector<s_hyperlink>& links)
			: ATEditionControl({toEditData}, std::unordered_map<hLinkId, s_hyperlink>(), "SetHyperLinks", &AObjectNode::setHyperlinks, &AObjectNode::getHyperlinks)
		{
			std::unordered_map<hLinkId, s_hyperlink> hyperlinks;
			for (const s_hyperlink& link : links)
			{
				hLinkId newId = xg::newGuid();
				hyperlinks[newId] = link;
			}

			for (auto& pair : m_toEditDatas)
				pair.second = hyperlinks;
		}

		SetHyperLinks::SetHyperLinks(const std::unordered_set<SafePtr<AObjectNode>>& toEditDatas, const std::vector<s_hyperlink>& links)
			: ATEditionControl(toEditDatas, std::unordered_map<hLinkId, s_hyperlink>(), "SetHyperLinks", & AObjectNode::setHyperlinks, & AObjectNode::getHyperlinks)
		{
			std::unordered_map<hLinkId, s_hyperlink> hyperlinks;
			for (const s_hyperlink& link : links)
			{
				hLinkId newId = xg::newGuid();
				hyperlinks[newId] = link;
			}

			for (auto& pair : m_toEditDatas)
				pair.second = hyperlinks;
		}

		SetHyperLinks::~SetHyperLinks()
		{}

		ControlType SetHyperLinks::getType() const
		{
			return ControlType::setHyperLinks;
		}


		/*
		** addHyperlink
		*/

		addHyperlink::addHyperlink(SafePtr<AObjectNode> toEditData, const std::wstring& link, std::wstring name)
			: m_dataPtrs({ toEditData })
			, m_link(link)
			, m_name(name)
		{
		}

		addHyperlink::addHyperlink(const std::unordered_set<SafePtr<AObjectNode>>& toEditDatas, const std::wstring& link, std::wstring name)
			: m_dataPtrs(toEditDatas)
			, m_link(link)
			, m_name(name)
		{
		}

		addHyperlink::~addHyperlink()
		{
		}

		void addHyperlink::doFunction(Controller& controller)
		{
			std::unordered_set<SafePtr<AGraphNode>> toActualize;
			for (const SafePtr<AObjectNode>& dataPtr : m_dataPtrs)
			{
				WritePtr<AObjectNode> writeData = dataPtr.get();
				if (!writeData)
				{
					CONTROLLOG << "control::dataEdition::addHyperlink do : data null" << LOGENDL;
					continue;
				}
				auto links = writeData->getHyperlinks();
				if (m_dataPtrLinks.find(dataPtr) == m_dataPtrLinks.end())
					m_dataPtrLinks[dataPtr] = xg::newGuid();
				links.insert(std::pair<hLinkId, s_hyperlink>(m_dataPtrLinks[dataPtr], { m_link, m_name }));
				writeData->setHyperlinks(links);

				doTimeModified(*&writeData);
				toActualize.insert(dataPtr);
			}

			CONTROLLOG << "control::dataEdition::addHyperlink do linkId to " << m_link << LOGENDL;
		}

		bool addHyperlink::canUndo() const
		{
			return (!m_dataPtrLinks.empty());
		}

		void addHyperlink::undoFunction(Controller& controller)
		{
			std::unordered_set<SafePtr<AGraphNode>> toActualize;

			for (std::pair<SafePtr<AObjectNode>, hLinkId> pairDataPtrLink : m_dataPtrLinks)
			{
				WritePtr<AObjectNode> writeData = pairDataPtrLink.first.get();
				if (!writeData)
				{
					CONTROLLOG << "control::dataEdition::addHyperlink do : data null" << LOGENDL;
					continue;
				}
				auto links = writeData->getHyperlinks();
				links.erase(pairDataPtrLink.second);
				writeData->setHyperlinks(links);
				undoTimeModified(*&writeData);
				toActualize.insert(pairDataPtrLink.first);
			}

			CONTROLLOG << "control::dataEdition::addHyperlink undo" << LOGENDL;
		}

		ControlType addHyperlink::getType() const
		{
			return (ControlType::addHyperLink);
		}

		/*
		** removeHyperlink
		*/

		removeHyperlink::removeHyperlink(SafePtr<AObjectNode> dataPtr, hLinkId idToDel)
			: m_dataPtr(dataPtr)
			, m_idToDel(idToDel)
			, m_canUndo(false)
		{
		}

		removeHyperlink::~removeHyperlink()
		{
		}

		void removeHyperlink::doFunction(Controller& controller)
		{
			WritePtr<AObjectNode> writeData = m_dataPtr.get();
			if (!writeData)
			{
				CONTROLLOG << "control::dataEdition::addHyperlink do : data null" << LOGENDL;
				return;
			}

			auto links = writeData->getHyperlinks();
			if (links.find(m_idToDel) == links.end())
				return;

			m_link = links.at(m_idToDel).hyperlink;
			m_name = links.at(m_idToDel).name;
			links.erase(m_idToDel);
			m_canUndo = true;

			writeData->setHyperlinks(links);
			doTimeModified(*&writeData);

			CONTROLLOG << "control::dataEdition::removeHyperlink do elemid link " << m_link << LOGENDL;
		}

		bool removeHyperlink::canUndo() const
		{
			return (m_canUndo);
		}

		void removeHyperlink::undoFunction(Controller& controller)
		{
			WritePtr<AObjectNode> writeData = m_dataPtr.get();
			if (!writeData)
			{
				CONTROLLOG << "control::dataEdition::addHyperlink do : data null" << LOGENDL;
				return;
			}

			auto links = writeData->getHyperlinks();

			links.insert(std::pair<hLinkId, s_hyperlink>(m_idToDel, { m_link, m_name }));
			writeData->setHyperlinks(links);
			undoTimeModified(*&writeData);

			CONTROLLOG << "control::dataEdition::removeHyperlink undo linkId " << m_idToDel << " to " << m_link << LOGENDL;
		}

		ControlType removeHyperlink::getType() const
		{
			return (ControlType::removeHyperLink);
		}
	}
}