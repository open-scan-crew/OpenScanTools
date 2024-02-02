#ifndef GRISEXPORTMESSAGE_H_
#define GRISEXPORTMESSAGE_H_

#include <deque>
#include <filesystem>
#include "models/3d/GridBox.h"
#include "controller/messages/IMessage.h"


class GridExportMessage : public IMessage
{
public:
	GridExportMessage(const std::filesystem::path& outputPath, const std::deque<GridBox>& grid);
	~GridExportMessage();
	MessageType	getType() const;	
	IMessage* copy() const;

public:
	const std::deque<GridBox>&		m_grid;
	const std::filesystem::path&	m_output;
};

#endif //! #pragma once
