#ifndef TLS_READER_H
#define TLS_READER_H

#include "models/pointCloud/TLS.h"

#include <fstream>

#define TLS_READER_DECLARE(klass) \
friend bool tls::reader::get##klass(std::ifstream& _is, FileVersion version, klass& object);

class OctreeBase;
class EmbeddedScan;
class OctreeDecoder;

namespace tls::reader
{
    bool checkFile(std::ifstream& _is, FileHeader& _header);

    bool getScanInfo(std::ifstream& _is, FileVersion version, ScanHeader& infos, uint32_t scanNumber);

    bool getOctreeBase(std::ifstream& _is, FileVersion _version, OctreeBase& _octreeBase);
    bool getEmbeddedScan(std::ifstream& _is, FileVersion _version, EmbeddedScan& _scan);

    OctreeDecoder* getNewOctreeDecoder(std::ifstream& _is, const FileVersion& version, const uint32_t& scanNumber, bool loadPoints);

    bool copyRawData(std::ifstream& _is, FileVersion _version, char** pointBuffer, uint64_t& pointBufferSize, char** instanceBuffer, uint64_t& instanceBufferSize);

}

#endif