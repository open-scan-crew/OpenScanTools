#ifndef CONTROL_TAG_EDITION_H_
#define CONTROL_TAG_EDITION_H_

#include "controller/controls/AEditionControl.h"
#include "models/project/Marker.h"
#include "models/application/TagTemplate.h"

class TagNode;

namespace control
{
	namespace tagEdition
	{
		class SetMarkerIcon : public ATEditionControl<TagNode, scs::MarkerIcon>
		{
		public:
			SetMarkerIcon(SafePtr<TagNode> tag, scs::MarkerIcon newIcon);
			~SetMarkerIcon();
			ControlType getType() const override;
		};

		class SetFieldData : public ATEditionControl<TagNode, std::wstring>
		{
		public:
			SetFieldData(SafePtr<TagNode> tag, sma::tFieldId fid, std::wstring newData);
			~SetFieldData();
			ControlType getType() const override;
		};

	}
}

#endif // !CONTROL_TAG_EDITION_H_