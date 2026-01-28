#ifndef COLOR_DENOISE_FILTER_MESSAGE_H
#define COLOR_DENOISE_FILTER_MESSAGE_H

#include "controller/messages/IMessage.h"

#include <string>

enum class DenoiseFilterMode
{
    Separate,
    Global
};

class ColorDenoiseFilterMessage : public IMessage
{
public:
    ColorDenoiseFilterMessage(int kNeighborsValue, int strengthValue, int luminanceStrengthValue, double radiusFactorValue, int iterationsValue, bool preserveLuminanceValue, DenoiseFilterMode modeValue, const std::wstring& outputFolderValue, bool openFolderAfterExportValue);
    ~ColorDenoiseFilterMessage() {}

    MessageType getType() const override;
    IMessage* copy() const override;

    int kNeighbors;
    int strength;
    int luminanceStrength;
    double radiusFactor;
    int iterations;
    bool preserveLuminance;
    DenoiseFilterMode mode;
    std::wstring outputFolder;
    bool openFolderAfterExport;
};

#endif
