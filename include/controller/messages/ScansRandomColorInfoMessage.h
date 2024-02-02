#ifndef SCANSRANDOMCOLORINFO_MESSAGE_H_
#define SCANSRANDOMCOLORINFO_MESSAGE_H_

#include "controller/messages/IMessage.h"
#include "models/pointCloud/TLS.h"

#include <glm/glm.hpp>
#include <vector>


class ScansRandomColorInfoMessage : public IMessage
{
public:
	struct ScanColorInfo
	{
		tls::ScanGuid id;
		glm::dvec3 pos;
		glm::vec3 color;
	};

public:
	ScansRandomColorInfoMessage(const std::vector<ScanColorInfo>& scans);
	~ScansRandomColorInfoMessage() {};
	IMessage::MessageType getType() const;
	IMessage* copy() const;

public:
	const std::vector<ScanColorInfo>& m_scans;
};
#endif // !RENDERCONTEXT_MESSAGE_H_
