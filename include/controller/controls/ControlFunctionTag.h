#ifndef CONTROL_FUNCTION_TAG_H_
#define CONTROL_FUNCTION_TAG_H_

#include "models/application/TagTemplate.h"
#include "controller/controls/AEditionControl.h"
#include "utils/Color32.hpp"
#include "models/graph/TagNode.h"

namespace control
{
	namespace function
	{
		namespace tag
		{
			class ActivateCreate : public AControl
			{
			public:
				ActivateCreate();
				~ActivateCreate();
				void doFunction(Controller& controller) override;
				bool canUndo() const override;
				void undoFunction(Controller& controller) override;
				ControlType getType() const override;
			};

			class ActivateMove : public AControl
			{
			public:
				ActivateMove();
				~ActivateMove();
				void doFunction(Controller& controller) override;
				bool canUndo() const override;
				void undoFunction(Controller& controller) override;
				ControlType getType() const override;
			};

			class ActivateDuplicate : public AControl
			{
			public:
				ActivateDuplicate();
				~ActivateDuplicate();
				void doFunction(Controller& controller) override;
				bool canUndo() const override;
				void undoFunction(Controller& controller) override;
				ControlType getType() const override;
			};

			class SetCurrentTagTemplate : public AControl
			{
			public:
				SetCurrentTagTemplate(SafePtr<sma::TagTemplate> temp);
				~SetCurrentTagTemplate();
				void doFunction(Controller& controller) override;
				bool canUndo() const override;
				void undoFunction(Controller& controller) override;
				ControlType getType() const override;
			private:
				SetCurrentTagTemplate();
			private:
				SafePtr<sma::TagTemplate> m_newTemplate;
			};

            class SetDefaultIcon : public AControl
            {
            public:
                SetDefaultIcon(scs::MarkerIcon newIcon);
                ~SetDefaultIcon();
                void doFunction(Controller& controller) override;
                bool canUndo() const override;
                void undoFunction(Controller& controller) override;
                ControlType getType() const override;
            private:
                SetDefaultIcon();
            private:
                scs::MarkerIcon _newIcon;
            };

			class DuplicateTag : public AControl
			{
			public:
				DuplicateTag(SafePtr<TagNode> srcTag, SafePtr<TagNode> dstTag);
				~DuplicateTag();
				void doFunction(Controller& controller) override;
				bool canUndo() const override;
				void undoFunction(Controller& controller) override;
				ControlType getType() const override;
			private:
				SafePtr<TagNode> m_srcTag;
				SafePtr<TagNode> m_dstTag;

				Data m_oldData;
				TagData m_oldTagData;
				ClippingData m_oldClippingData;

				bool m_canUndo;
			};
		}
	}
}

#endif // !CONTROL_FUNCTION_TAG_H_