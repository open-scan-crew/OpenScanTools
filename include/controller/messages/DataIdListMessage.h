#ifndef DATAID_LIST_MESSAGE_H_
#define DATAID_LIST_MESSAGE_H_

#include "controller/messages/IMessage.h"
#include "models/Types.hpp"
#include "utils/safe_ptr.h"

#include <unordered_set>

class AGraphNode;

class DataListMessage : public IMessage
{
public:
	DataListMessage(const std::unordered_set<SafePtr<AGraphNode>>& dataPtrs, const ElementType& type);
	DataListMessage(const std::unordered_set<SafePtr<AGraphNode>>& dataPtrs);
	template<class T>
	DataListMessage(const std::unordered_set<SafePtr<T>>& dataPtrs);
	~DataListMessage();
	MessageType	getType() const;
	IMessage* copy() const;

public:
	ElementType									m_type;
	std::unordered_set<SafePtr<AGraphNode>>		m_dataPtrs;
};

#endif //! IDATALISTMESSAGE_H_
