#ifndef COLORAVAILABLELIST_MESSAGE_H_
#define COLORAVAILABLELIST_MESSAGE_H_

#include "IMessage.h"
#include <glm/glm.hpp>
#include <list>

class ColorAvailableListMessage : public IMessage
{
public:
	ColorAvailableListMessage(const std::list<glm::vec3>& colors);
	~ColorAvailableListMessage() {};
	IMessage::MessageType getType() const;
	IMessage* copy() const;

public:
	std::list<glm::vec3> m_colorList;
};
#endif // !COLORAVAILABLELIST_MESSAGE_H_