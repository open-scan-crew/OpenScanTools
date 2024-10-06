#ifndef REFERENTIAL_NODE_H_
#define REFERENTIAL_NODE_H_

#include "models/graph/AGraphNode.h"

#include <deque>

class ReferentialNode : public AGraphNode
{
public:
	ReferentialNode(const std::wstring& name, const TransformationModule& transformation);
	~ReferentialNode();

	Type getGraphType() const;
	bool isSelected() const override;

	void addForceReferential(const std::string& name, const TransformationModule& transformation);
	bool addReferential(const std::string& name, const TransformationModule& transformation);
	bool removeReferential(const std::string& name);
	bool replaceReferential(const std::string& name, const TransformationModule& transformation);
	bool switchReferential(const std::string& name);
	std::string getCurrentReferential() const;

	bool push(const std::string& name);
	bool pop();

protected:
	std::string m_currentReferential;
	std::unordered_map<std::string, TransformationModule> m_referentials;
	std::deque<std::string> m_referentialDeque;
};

#endif //! REFERENTIAL_NODE_H_