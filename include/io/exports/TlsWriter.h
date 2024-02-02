#ifndef TLS_WRITER_H_
#define TLS_WRITER_H_

#include "models/pointCloud/TLS.h"

#include <fstream>


#define TLS_WRITER_DECLARE(klass) \
friend bool tls::writer::write##klass(std::ofstream& _os, uint32_t _scanNumber, klass const* _pOctree, ScanHeader _header);

class OctreeCtor;
class OctreeShredder;

namespace tls
{
    namespace writer
    {
        bool writeFileHeader(std::ofstream& _os, uint32_t _scanCount);

        bool writeOctreeCtor(std::ofstream& _os, uint32_t _scanNumber, OctreeCtor const* _pOctree, ScanHeader _header);
        void overwriteTransformation(std::ofstream& _os, double _translation[3], double _quaternion[4]);

        bool writeOctreeShredder(std::ofstream& _os, uint32_t _scanNumber, OctreeShredder const* _pOctree, ScanHeader _header);
    }
}

#endif