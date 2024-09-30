#include "controller/controls/ControlTagEdition.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "models/3d/Graph/GraphManager.hxx"
#include "gui/GuiData/GuiDataTag.h"
#include "utils/Logger.h"

#include "models/3d/Graph/TagNode.h"
#include "controller/controls/AEditionControl.hxx"

// control::tagEdition::

namespace control
{
	namespace tagEdition
	{
		/*
		** SetMarkerIcon
		*/

		SetMarkerIcon::SetMarkerIcon(SafePtr<TagNode> tag, scs::MarkerIcon newIcon)
			: ATEditionControl({ tag }, newIcon, "SetMarkerIcon", &TagNode::setMarkerIcon, &TagNode::getMarkerIcon)
		{
			m_actualize_tree_view = true;
		}

		SetMarkerIcon::~SetMarkerIcon()
		{
		}

		ControlType SetMarkerIcon::getType() const
		{
			return (ControlType::setShapeTagEdit);
		}

		/*
		** SetFieldData
		*/

		SetFieldData::SetFieldData(SafePtr<TagNode> tag, sma::tFieldId fid, std::wstring newData)
			: ATEditionControl({ tag }, newData, "SetFieldData", [fid](TagNode& node, const std::wstring& newValue) {node.setValue(fid, newValue); }, [fid](const TagNode& node) { return node.getValue(fid); })
		{
		}

		SetFieldData::~SetFieldData()
		{
		}

		ControlType SetFieldData::getType() const
		{
			return (ControlType::setFieldDatatagEdit);
		}
	}
}