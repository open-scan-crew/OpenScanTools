#ifndef COLOR_BALANCE_MESSAGE_H
#define COLOR_BALANCE_MESSAGE_H

#include "controller/messages/IMessage.h"

#include <filesystem>

class ColorBalanceMessage : public IMessage
{
public:
    ColorBalanceMessage(int photometricDepthValue,
                        int nMinValue,
                        int nGoodValue,
                        double betaValue,
                        int smoothingLevelsValue,
                        bool applyIntensityWhenAvailableValue,
                        const std::wstring& outputFolderValue,
                        bool openFolderAfterExportValue);
    ~ColorBalanceMessage() {}

    MessageType getType() const override;
    IMessage* copy() const override;

public:
    int photometricDepth;
    int nMin;
    int nGood;
    double beta;
    int smoothingLevels;
    bool applyIntensityWhenAvailable;
    std::wstring outputFolder;
    bool openFolderAfterExport;
};

#endif
