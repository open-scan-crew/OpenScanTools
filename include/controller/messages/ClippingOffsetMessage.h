#ifndef CLIPPING_OFFSET_MESSAGE_H_
#define CLIPPING_OFFSET_MESSAGE_H_

#include "controller/messages/IMessage.h"
#include "controller/ControllerContext.h"

class ClippingOffsetMessage : public IMessage
{
public:
	ClippingOffsetMessage(const ClippingOffset& offset);
	~ClippingOffsetMessage();
	MessageType getType() const;
	const ClippingOffset& getOffset() const;

private:
	const ClippingOffset& m_offset;
};

#endif //! CLIPPING_OFFSET_MESSAGE_H_