#ifndef E57_UTILS_H
#define E57_UTILS_H

#include "openE57.h"
#include "io/StagingBuffers.h"

namespace e57Utils
{
    bool check_tl_result(TlResult, std::string& msg);

    TlResult detectFormat(e57::StructureNode& _dataSet, E57AttribFormat& format);
    TlResult initDestBuffers(e57::ImageFile _imf, e57::StructureNode& _dataSet, std::vector<e57::SourceDestBuffer>& _destBuffers, StagingBuffers& _stagingBuffers, const E57AttribFormat& _format, Limits& _limits);
    TlResult getRigidBodyTransform(e57::StructureNode _dataSet, double translation[3], double quaternion[4]);

    bool printFileStructure(e57::ImageFile imf, std::string& outText);
    void printNode(e57::Node& childNode, uint32_t depth, std::ostream& os);
}

#endif
