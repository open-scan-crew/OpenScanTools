#ifndef CLICK_MESSAGE_H_
#define CLICK_MESSAGE_H_

#include "controller/messages/IMessage.h"
#include "models/OpenScanToolsModelEssentials.h"

class IPanel;

class ClickMessage : public IMessage
{
public:
	ClickMessage(const Pos3D& pos, const IPanel* target = nullptr);
	~ClickMessage();
	MessageType getType() const;
	IMessage* copy() const;

public:
	const Pos3D m_pos;
	const IPanel* m_target;
};

#endif //! CLICKMESSAGE_H_