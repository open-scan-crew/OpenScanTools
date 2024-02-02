#include "models/3d/Graph/ReferentialNode.h"
#include "models/3d/Graph/CameraNode.h"
#include "utils/Logger.h"

#define SGLog Logger::log(LoggerMode::SceneGraphLog)

ReferentialNode::ReferentialNode(const std::wstring& name, const TransformationModule& transformation)
	: AGraphNode()
{
	setTransformationModule(transformation);
	setName(name);
	setSelected(false);
	setId(xg::Guid());
	SGLog << "ReferentialNode -> created with graphid " << m_graphicId << " & name: " << name << Logger::endl;
}

ReferentialNode::~ReferentialNode()
{}

AGraphNode::Type ReferentialNode::getGraphType() const
{
	return Type::Referential;
}

bool ReferentialNode::isSelected() const
{
	return false;
}

void ReferentialNode::addForceReferential(const std::string& name, const TransformationModule& transformation)
{
	m_referentials.insert({ name,transformation });
}

bool ReferentialNode::addReferential(const std::string& name, const TransformationModule& transformation)
{
	if (m_referentials.find(name) != m_referentials.end())
		return false;
	m_referentials.insert({ name,transformation });
	return true;
}

bool ReferentialNode::removeReferential(const std::string& name)
{
	if (m_referentials.find(name) == m_referentials.end())
		return false;
	m_referentials.erase(name);
	return true;
}

bool ReferentialNode::replaceReferential(const std::string& name, const TransformationModule& transformation)
{
	if (m_referentials.find(name) == m_referentials.end())
		return false;
	m_referentials.at(name) = transformation;
	return true;
}

bool ReferentialNode::switchReferential(const std::string& name)
{
	if (m_referentials.find(name) == m_referentials.end())
		return false;
	m_currentReferential = name;
	setTransformationModule(m_referentials.at(name));
	return true;
}

std::string ReferentialNode::getCurrentReferential() const
{
	return m_currentReferential;
}

bool ReferentialNode::push(const std::string& name)
{
	if (switchReferential(name))
	{
		m_referentialDeque.push_front(name);
		return true;
	}
	return false;
}

bool ReferentialNode::pop()
{
	if (m_referentialDeque.empty())
		return false;
	if (switchReferential(*m_referentialDeque.begin()))
	{
		m_referentialDeque.pop_front();
		return true;
	}
	return false;
}