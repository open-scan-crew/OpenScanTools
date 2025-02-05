#ifndef PCE_STREAM_H_
#define PCE_STREAM_H_

#include <cstdint>

class SmartBuffer;

struct TlStagedTransferInfo
{
    uint64_t stageOffset;
    uint64_t dataSize;
    SmartBuffer& sbuf;
};

#endif