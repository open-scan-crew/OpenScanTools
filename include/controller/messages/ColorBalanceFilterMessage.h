#ifndef COLOR_BALANCE_FILTER_MESSAGE_H
#define COLOR_BALANCE_FILTER_MESSAGE_H

#include "controller/messages/IMessage.h"
#include "controller/messages/FilterExecutionMode.h"
#include "io/FileUtils.h"

#include <string>

enum class ColorBalanceMode
{
    Separate,
    Global
};

class ColorBalanceFilterMessage : public IMessage
{
public:
    ColorBalanceFilterMessage(int kMinValue, int kMaxValue, double trimPercentValue, double sharpnessBlendValue, ColorBalanceMode modeValue, bool applyOnIntensityAndRgbValue, FilterExecutionMode executionModeValue, FileType outputFileTypeValue, const std::wstring& outputFolderValue, bool openFolderAfterExportValue);
    ~ColorBalanceFilterMessage() {}

    MessageType getType() const override;
    IMessage* copy() const override;

    int kMin;
    int kMax;
    double trimPercent;
    double sharpnessBlend;
    ColorBalanceMode mode;
    bool applyOnIntensityAndRgb;
    FilterExecutionMode executionMode;
    FileType outputFileType;
    std::wstring outputFolder;
    bool openFolderAfterExport;
};

#endif
