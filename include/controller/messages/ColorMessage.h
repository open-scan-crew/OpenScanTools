#ifndef COLOR_MESSAGE_H_
#define COLOR_MESSAGE_H_

#include "controller/messages/IMessage.h"
#include "models/OpenScanToolsModelEssentials.h"

class ColorMessage : public IMessage
{
public:
	ColorMessage(const glm::vec3& color);
	~ColorMessage();
	MessageType getType() const;	
	IMessage* copy() const;

public:
	const glm::vec3& m_color;
};

#endif //! COLOR_MESSAGE_H_