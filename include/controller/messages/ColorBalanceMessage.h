#ifndef COLOR_BALANCE_MESSAGE_H
#define COLOR_BALANCE_MESSAGE_H

#include "controller/messages/IMessage.h"
#include "io/FileUtils.h"
#include "pointCloudEngine/ColorBalanceSettings.h"

#include <string>

class ColorBalanceMessage : public IMessage
{
public:
    ColorBalanceMessage(const ColorBalanceSettings& settings, FileType outputFileType, const std::wstring& outputFolder, bool openFolderAfterExport);
    ~ColorBalanceMessage() {}

    MessageType getType() const override;
    IMessage* copy() const override;

    ColorBalanceSettings settings;
    FileType outputFileType;
    std::wstring outputFolder;
    bool openFolderAfterExport;
};

#endif
