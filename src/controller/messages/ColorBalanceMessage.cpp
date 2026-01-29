#include "controller/messages/ColorBalanceMessage.h"

ColorBalanceMessage::ColorBalanceMessage(const ColorBalanceSettings& settings, FileType outputFileType, const std::wstring& outputFolder, bool openFolderAfterExport)
    : settings(settings)
    , outputFileType(outputFileType)
    , outputFolder(outputFolder)
    , openFolderAfterExport(openFolderAfterExport)
{}

IMessage::MessageType ColorBalanceMessage::getType() const
{
    return IMessage::MessageType::COLOR_BALANCE_PARAMETERS;
}

IMessage* ColorBalanceMessage::copy() const
{
    return new ColorBalanceMessage(*this);
}
