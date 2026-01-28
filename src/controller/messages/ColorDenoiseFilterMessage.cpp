#include "controller/messages/ColorDenoiseFilterMessage.h"

ColorDenoiseFilterMessage::ColorDenoiseFilterMessage(int kNeighborsValue, int strengthValue, int luminanceStrengthValue, double radiusFactorValue, int iterationsValue, bool preserveLuminanceValue, DenoiseFilterMode modeValue, const std::wstring& outputFolderValue, bool openFolderAfterExportValue)
    : kNeighbors(kNeighborsValue)
    , strength(strengthValue)
    , luminanceStrength(luminanceStrengthValue)
    , radiusFactor(radiusFactorValue)
    , iterations(iterationsValue)
    , preserveLuminance(preserveLuminanceValue)
    , mode(modeValue)
    , outputFolder(outputFolderValue)
    , openFolderAfterExport(openFolderAfterExportValue)
{}

IMessage::MessageType ColorDenoiseFilterMessage::getType() const
{
    return MessageType::COLOR_DENOISE_FILTER_PARAMETERS;
}

IMessage* ColorDenoiseFilterMessage::copy() const
{
    return new ColorDenoiseFilterMessage(kNeighbors, strength, luminanceStrength, radiusFactor, iterations, preserveLuminance, mode, outputFolder, openFolderAfterExport);
}
