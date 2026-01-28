#include "controller/messages/ColorBalanceMessage.h"

ColorBalanceMessage::ColorBalanceMessage(int photometricDepthValue,
                                         int nMinValue,
                                         int nGoodValue,
                                         double betaValue,
                                         int smoothingLevelsValue,
                                         bool applyIntensityWhenAvailableValue,
                                         const std::wstring& outputFolderValue,
                                         bool openFolderAfterExportValue)
    : photometricDepth(photometricDepthValue)
    , nMin(nMinValue)
    , nGood(nGoodValue)
    , beta(betaValue)
    , smoothingLevels(smoothingLevelsValue)
    , applyIntensityWhenAvailable(applyIntensityWhenAvailableValue)
    , outputFolder(outputFolderValue)
    , openFolderAfterExport(openFolderAfterExportValue)
{}

IMessage::MessageType ColorBalanceMessage::getType() const
{
    return MessageType::COLOR_BALANCE_PARAMETERS;
}

IMessage* ColorBalanceMessage::copy() const
{
    return new ColorBalanceMessage(photometricDepth, nMin, nGood, beta, smoothingLevels, applyIntensityWhenAvailable, outputFolder, openFolderAfterExport);
}
