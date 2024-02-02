#include "models/OpenScanToolsModelEssentials.h"

#include <cstdint>
#include <vector>

typedef uint32_t TLID;

namespace tl::tests::data
{
    void upcastTest();

    void integrityTest(size_t count, int seed);

    void massTest_1(uint64_t objectCount);
    void massTest_2(uint64_t objectCount);

    void allocateObjects(uint64_t count, std::vector<TLID>& result_ids);
    void freeObjects(const std::vector<TLID>& ids);
    void modifyObjects(const std::vector<TLID>& ids);
    void readObjects(const std::vector<TLID>& ids);

    void allocateObjects_direct(uint64_t count, std::vector<void*>& datas);
    void freeObjects_direct(std::vector<void*>& datas);
    void modifyObjects_direct(std::vector<void*>& datas);
    void readObjects_direct(const std::vector<void*>& datas);
}