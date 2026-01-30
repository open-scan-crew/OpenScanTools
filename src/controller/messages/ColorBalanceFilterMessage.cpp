#include "controller/messages/ColorBalanceFilterMessage.h"

ColorBalanceFilterMessage::ColorBalanceFilterMessage(int kMinValue, int kMaxValue, double trimPercentValue, double sharpnessBlendValue, ColorBalanceMode modeValue, bool applyOnIntensityAndRgbValue, const std::wstring& outputFolderValue, bool openFolderAfterExportValue)
    : kMin(kMinValue)
    , kMax(kMaxValue)
    , trimPercent(trimPercentValue)
    , sharpnessBlend(sharpnessBlendValue)
    , mode(modeValue)
    , applyOnIntensityAndRgb(applyOnIntensityAndRgbValue)
    , outputFolder(outputFolderValue)
    , openFolderAfterExport(openFolderAfterExportValue)
{}

IMessage::MessageType ColorBalanceFilterMessage::getType() const
{
    return IMessage::MessageType::COLOR_BALANCE_FILTER_PARAMETERS;
}

IMessage* ColorBalanceFilterMessage::copy() const
{
    return new ColorBalanceFilterMessage(kMin, kMax, trimPercent, sharpnessBlend, mode, applyOnIntensityAndRgb, outputFolder, openFolderAfterExport);
}
