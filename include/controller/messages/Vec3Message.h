#ifndef VEC3_MESSAGE_H_
#define VEC3_MESSAGE_H_

#include "controller/messages/IMessage.h"
#include <glm/glm.hpp>

class Vec3Message : public IMessage
{
public:
	Vec3Message(const glm::vec3& vec3);
	~Vec3Message();
	MessageType getType() const;	
	IMessage* copy() const;

public:
	const glm::vec3 m_vec3;
};

#endif //! VEC3_MESSAGE_H_