struct VirtualBuffer
{
    VkDeviceSize memSize;
    VkDeviceSize offSet;
    VkDeviceSize memAlign;
};

struct BufferPartition // BufferManager
{
    VkDeviceSize memSize;
    VkDeviceSize offSet;
    VkDeviceSize memAlign;
};

// 1 Memory = 1 Buffer = N Virtual Buffers
class MemoryManager
{
public:
    
private:
    VkDeviceMamory memory;
    VkBuffer buffer;
    std::vector<VirtualBuffer> v_Buffers;
    // Memory heap (one MenoryManager per memory heap)
};