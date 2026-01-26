#ifndef COLOR_BALANCE_MESSAGE_H
#define COLOR_BALANCE_MESSAGE_H

#include "controller/messages/IMessage.h"

#include <string>

enum class ColorBalanceMode
{
    Separate,
    Global
};

enum class ColorBalanceStrength
{
    Low,
    Mid,
    High
};

class ColorBalanceMessage : public IMessage
{
public:
    ColorBalanceMessage(ColorBalanceStrength strength, ColorBalanceMode mode, bool applyOnRGBAndIntensity, const std::wstring& outputFolder, bool openFolderAfterExport);
    ~ColorBalanceMessage() {}

    MessageType getType() const override;
    IMessage* copy() const override;

    ColorBalanceStrength strength;
    ColorBalanceMode mode;
    bool applyOnRGBAndIntensity;
    std::wstring outputFolder;
    bool openFolderAfterExport;
};

#endif
