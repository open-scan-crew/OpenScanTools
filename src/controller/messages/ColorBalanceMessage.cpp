#include "controller/messages/ColorBalanceMessage.h"

ColorBalanceMessage::ColorBalanceMessage(ColorBalanceStrength strengthValue, ColorBalanceMode modeValue, bool applyOnRGBAndIntensityValue, const std::wstring& outputFolderValue, bool openFolderAfterExportValue)
    : strength(strengthValue)
    , mode(modeValue)
    , applyOnRGBAndIntensity(applyOnRGBAndIntensityValue)
    , outputFolder(outputFolderValue)
    , openFolderAfterExport(openFolderAfterExportValue)
{
}

IMessage::MessageType ColorBalanceMessage::getType() const
{
    return MessageType::COLOR_BALANCE_FILTER_PARAMETERS;
}

IMessage* ColorBalanceMessage::copy() const
{
    return new ColorBalanceMessage(strength, mode, applyOnRGBAndIntensity, outputFolder, openFolderAfterExport);
}
