#include "controller/messages/StatisticalOutlierFilterMessage.h"

StatisticalOutlierFilterMessage::StatisticalOutlierFilterMessage(int kNeighborsValue, double nSigmaValue, int samplingPercentValue, double betaValue, OutlierFilterMode modeValue, OutlierFilterExecutionMode executionModeValue, FileType outputFileTypeValue, const std::wstring& outputFolderValue, bool openFolderAfterExportValue)
    : kNeighbors(kNeighborsValue)
    , nSigma(nSigmaValue)
    , samplingPercent(samplingPercentValue)
    , beta(betaValue)
    , mode(modeValue)
    , executionMode(executionModeValue)
    , outputFileType(outputFileTypeValue)
    , outputFolder(outputFolderValue)
    , openFolderAfterExport(openFolderAfterExportValue)
{}

IMessage::MessageType StatisticalOutlierFilterMessage::getType() const
{
    return MessageType::STAT_OUTLIER_FILTER_PARAMETERS;
}

IMessage* StatisticalOutlierFilterMessage::copy() const
{
    return new StatisticalOutlierFilterMessage(kNeighbors, nSigma, samplingPercent, beta, mode, executionMode, outputFileType, outputFolder, openFolderAfterExport);
}
