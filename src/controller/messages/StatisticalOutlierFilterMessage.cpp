#include "controller/messages/StatisticalOutlierFilterMessage.h"

StatisticalOutlierFilterMessage::StatisticalOutlierFilterMessage(int kNeighborsValue, double nSigmaValue, OutlierFilterMode modeValue)
    : kNeighbors(kNeighborsValue)
    , nSigma(nSigmaValue)
    , mode(modeValue)
{}

IMessage::MessageType StatisticalOutlierFilterMessage::getType() const
{
    return MessageType::STAT_OUTLIER_FILTER_PARAMETERS;
}

IMessage* StatisticalOutlierFilterMessage::copy() const
{
    return new StatisticalOutlierFilterMessage(kNeighbors, nSigma, mode);
}
