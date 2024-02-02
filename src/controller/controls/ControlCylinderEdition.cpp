#include "controller/controls/ControlCylinderEdition.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "gui/GuiData/GuiData3dObjects.h"

#include "models/3d/Graph/CylinderNode.h"
#include "models/3d/Graph/SphereNode.h"
#include "controller/controls/AEditionControl.hxx"

#include "utils/Logger.h"

namespace control
{
	namespace cylinderEdition
	{
		/*
		*		SetRadius
		*/

		SetForcedRadius::SetForcedRadius(SafePtr<CylinderNode> toEditData, const double& radius)
			: m_toEditData(toEditData)
			, m_newRadius(radius)
			, m_canUndo(false)
		{}

		SetForcedRadius::~SetForcedRadius()
		{}

		void SetForcedRadius::doFunction(Controller& controller)
		{
			{
				WritePtr<CylinderNode> wCyl = m_toEditData.get();
				if (!wCyl)
				{
					CONTROLLOG << "control::cylinderEdition::SetRadius do : cylinder null" << LOGENDL;
					return;
				}

				m_oldRadius = wCyl->getForcedRadius();
				m_oldDiameterSet = wCyl->getDiameterSet();
				wCyl->setForcedRadius(m_newRadius);
				wCyl->setDiameterSet(StandardRadiusData::DiameterSet::Forced);

				wCyl->updateScale();
				doTimeModified(*&wCyl);
			}

			m_canUndo = true;
			controller.actualizeNodes(ActualizeOptions(false), m_toEditData);

			CONTROLLOG << "control::cylinderEdition::SetRadius do " << LOGENDL;
		}

		bool SetForcedRadius::canUndo() const
		{
			return (m_canUndo);
		}

		void SetForcedRadius::undoFunction(Controller& controller)
		{
			{
				WritePtr<CylinderNode> wCyl = m_toEditData.get();
				if (!wCyl)
				{
					CONTROLLOG << "control::cylinderEdition::SetRadius do : cylinder null" << LOGENDL;
					return;
				}

				wCyl->setForcedRadius(m_oldRadius);
				wCyl->setDiameterSet(m_oldDiameterSet);

				wCyl->updateScale();
				undoTimeModified(*&wCyl);
			}

			controller.actualizeNodes(ActualizeOptions(false), m_toEditData);

			CONTROLLOG << "control::cylinderEdition::SetRadius undo " << LOGENDL;
		}

		ControlType SetForcedRadius::getType() const
		{
			return (ControlType::setForcedRadiusEdit);
		}

		/*
		*		SetDetectedRadius
		*/

		SetDetectedRadius::SetDetectedRadius(SafePtr<CylinderNode> toEditData)
			: m_toEditData(toEditData)
			, m_canUndo(false)
		{}

		SetDetectedRadius::~SetDetectedRadius()
		{}

		void SetDetectedRadius::doFunction(Controller& controller)
		{
			{
				WritePtr<CylinderNode> wCyl = m_toEditData.get();
				if (!wCyl)
				{
					CONTROLLOG << "control::cylinderEdition::SetRadius do : cylinder null" << LOGENDL;
					return;
				}

				m_oldDiameterSet = wCyl->getDiameterSet();
				wCyl->setDiameterSet(StandardRadiusData::DiameterSet::Detected);

				wCyl->updateScale();
				doTimeModified(*&wCyl);
			}

			m_canUndo = true;

			controller.actualizeNodes(ActualizeOptions(false), m_toEditData);

			CONTROLLOG << "control::cylinderEdition::SetRadius do " << LOGENDL;
		}

		bool SetDetectedRadius::canUndo() const
		{
			return (m_canUndo);
		}

		void SetDetectedRadius::undoFunction(Controller& controller)
		{
			{
				WritePtr<CylinderNode> wCyl = m_toEditData.get();
				if (!wCyl)
				{
					CONTROLLOG << "control::cylinderEdition::SetRadius do : cylinder null" << LOGENDL;
					return;
				}

				wCyl->setDiameterSet(m_oldDiameterSet);

				wCyl->updateScale();
				undoTimeModified(*&wCyl);
			}

			controller.actualizeNodes(ActualizeOptions(false), m_toEditData);

			CONTROLLOG << "control::cylinderEdition::SetRadius undo " << LOGENDL;
		}

		ControlType SetDetectedRadius::getType() const
		{
			return ControlType::setDetectedRadiusEdit;
		}

		/*
		*		SetStandard
		*/

		SetStandard::SetStandard(SafePtr<CylinderNode> toEditData, const SafePtr<StandardList>& standard)
			: m_toEditData(toEditData)
			, m_standard(standard)
			, m_oldDiameterSet(StandardRadiusData::DiameterSet::Standard)
			, m_oldStandard()
		{}

		SetStandard::~SetStandard()
		{}

		void SetStandard::doFunction(Controller& controller)
		{
			{
				WritePtr<CylinderNode> wCyl = m_toEditData.get();
				if (!wCyl)
				{
					CONTROLLOG << "control::cylinderEdition::SetRadius do : cylinder null" << LOGENDL;
					return;
				}

				m_oldStandard = wCyl->getStandard();
				m_oldDiameterSet = wCyl->getDiameterSet();

				if (m_standard == m_oldStandard
					&& m_oldDiameterSet == StandardRadiusData::DiameterSet::Standard)
					return;

				wCyl->setStandard(m_standard);
				wCyl->setDiameterSet(StandardRadiusData::DiameterSet::Standard);

				wCyl->updateScale();
				doTimeModified(*&wCyl);
			}

			m_canUndo = true;

			controller.actualizeNodes(ActualizeOptions(false), m_toEditData);

			CONTROLLOG << "control::cylinderEdition::CheckStandard do " << LOGENDL;
		}

		bool SetStandard::canUndo() const
		{
			return m_canUndo;
		}

		void SetStandard::undoFunction(Controller& controller)
		{
			{
				WritePtr<CylinderNode> wCyl = m_toEditData.get();
				if (!wCyl)
				{
					CONTROLLOG << "control::cylinderEdition::SetRadius do : cylinder null" << LOGENDL;
					return;
				}

				wCyl->setStandard(m_oldStandard);
				wCyl->setDiameterSet(m_oldDiameterSet);

				wCyl->updateScale();
				undoTimeModified(*&wCyl);
			}

			controller.actualizeNodes(ActualizeOptions(false), m_toEditData);

			CONTROLLOG << "control::cylinderEdition::CheckStandard undo " << LOGENDL;
		}

		ControlType SetStandard::getType() const
		{
			return (ControlType::setStandardEdit);
		}
		
		/*
		*		SetLength
		*/

		SetLength::SetLength(SafePtr<CylinderNode> toEditData, const double& larger)
			: ATEditionControl({ toEditData }, larger, "SetLength", &CylinderNode::setLength, &CylinderNode::getLength)
		{
		}

		SetLength::SetLength(const std::unordered_set<SafePtr<CylinderNode>>& toEditDatas, const double& larger)
			: ATEditionControl(toEditDatas, larger, "SetLength", &CylinderNode::setLength, &CylinderNode::getLength)
		{
		}

		SetLength::~SetLength()
		{}

		ControlType SetLength::getType() const
		{
			return (ControlType::setCylinderLengthEdit);
		}

	}
}