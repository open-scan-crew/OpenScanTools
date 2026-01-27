#include "controller/messages/ColorDenoiseFilterMessage.h"

ColorDenoiseFilterMessage::ColorDenoiseFilterMessage(int kNeighborsValue, int strengthValue, double radiusFactorValue, int iterationsValue, bool preserveLuminanceValue, const std::wstring& outputFolderValue, bool openFolderAfterExportValue)
    : kNeighbors(kNeighborsValue)
    , strength(strengthValue)
    , radiusFactor(radiusFactorValue)
    , iterations(iterationsValue)
    , preserveLuminance(preserveLuminanceValue)
    , outputFolder(outputFolderValue)
    , openFolderAfterExport(openFolderAfterExportValue)
{}

IMessage::MessageType ColorDenoiseFilterMessage::getType() const
{
    return MessageType::COLOR_DENOISE_FILTER_PARAMETERS;
}

IMessage* ColorDenoiseFilterMessage::copy() const
{
    return new ColorDenoiseFilterMessage(kNeighbors, strength, radiusFactor, iterations, preserveLuminance, outputFolder, openFolderAfterExport);
}
