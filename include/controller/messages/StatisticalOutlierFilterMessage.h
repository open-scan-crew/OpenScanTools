#ifndef STATISTICAL_OUTLIER_FILTER_MESSAGE_H
#define STATISTICAL_OUTLIER_FILTER_MESSAGE_H

#include "controller/messages/IMessage.h"

#include <string>

enum class OutlierFilterMode
{
    Separate,
    Global
};

class StatisticalOutlierFilterMessage : public IMessage
{
public:
    StatisticalOutlierFilterMessage(int kNeighbors, double nSigma, OutlierFilterMode mode, const std::wstring& outputFolder, bool openFolderAfterExport);
    ~StatisticalOutlierFilterMessage() {}

    MessageType getType() const override;
    IMessage* copy() const override;

    int kNeighbors;
    double nSigma;
    OutlierFilterMode mode;
    std::wstring outputFolder;
    bool openFolderAfterExport;
};

#endif
