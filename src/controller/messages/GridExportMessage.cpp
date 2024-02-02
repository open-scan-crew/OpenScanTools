#include "controller/messages/GridExportMessage.h"

GridExportMessage::GridExportMessage(const std::filesystem::path& outputPath, const std::deque<GridBox>& grid)
	: m_output(outputPath)
	, m_grid(grid)
{}

GridExportMessage::~GridExportMessage()
{}

IMessage::MessageType GridExportMessage::getType() const
{
	return IMessage::MessageType::GRID_EXPORT;
}

IMessage* GridExportMessage::copy() const
{
	return new GridExportMessage(*this);
}
