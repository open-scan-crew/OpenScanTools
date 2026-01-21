#ifndef STATISTICAL_OUTLIER_FILTER_MESSAGE_H
#define STATISTICAL_OUTLIER_FILTER_MESSAGE_H

#include "controller/messages/IMessage.h"

enum class OutlierFilterMode
{
    Separate,
    Global
};

class StatisticalOutlierFilterMessage : public IMessage
{
public:
    StatisticalOutlierFilterMessage(int kNeighbors, double nSigma, OutlierFilterMode mode);
    ~StatisticalOutlierFilterMessage() {}

    MessageType getType() const override;
    IMessage* copy() const override;

    int kNeighbors;
    double nSigma;
    OutlierFilterMode mode;
};

#endif
