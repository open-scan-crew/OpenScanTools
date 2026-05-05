#include "controller/messages/ColorBalanceFilterMessage.h"

ColorBalanceFilterMessage::ColorBalanceFilterMessage(int kMinValue, int kMaxValue, double trimPercentValue, double sharpnessBlendValue, ColorBalanceMode modeValue, bool applyOnIntensityAndRgbValue, FileType outputFileTypeValue, const std::wstring& outputFolderValue, bool openFolderAfterExportValue, FilterExecutionMode executionModeValue)
    : kMin(kMinValue)
    , kMax(kMaxValue)
    , trimPercent(trimPercentValue)
    , sharpnessBlend(sharpnessBlendValue)
    , mode(modeValue)
    , applyOnIntensityAndRgb(applyOnIntensityAndRgbValue)
    , outputFileType(outputFileTypeValue)
    , outputFolder(outputFolderValue)
    , openFolderAfterExport(openFolderAfterExportValue)
    , executionMode(executionModeValue)
{}

IMessage::MessageType ColorBalanceFilterMessage::getType() const
{
    return IMessage::MessageType::COLOR_BALANCE_FILTER_PARAMETERS;
}

IMessage* ColorBalanceFilterMessage::copy() const
{
    return new ColorBalanceFilterMessage(kMin, kMax, trimPercent, sharpnessBlend, mode, applyOnIntensityAndRgb, outputFileType, outputFolder, openFolderAfterExport, executionMode);
}
