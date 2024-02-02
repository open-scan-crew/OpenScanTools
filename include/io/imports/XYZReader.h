/*** PointCloud.h *************************************************************
** Author           | Robin Kervadec                                         **
**                                                                           **
** TODO insert description                                                   ** 
**                                                                           **
******* Copyright (C) 2018 TagLabs *******************************************/

#ifndef _POINT_CLOUD_H_
#define _POINT_CLOUD_H_

#include <cstdint>
#include <string>

const uint32_t POINTS_PER_CHUNK = 65536;

class PointCloud
{
public:
    PointCloud();
    PointCloud(const std::string& _filePath);

    ~PointCloud();

    void tryOpenFile(const std::string& _filePath);
    void loadToMem(void* _p);
    uint32_t getByteSize();
    uint32_t getVertexCount();

    bool isReadyToLoad();
    bool isReadyToDraw();

private:
    std::string m_filePath;
    uint32_t m_nbPoints;
    uint32_t m_nbChunks;

    bool m_readyToLoad;
    bool m_readyToDraw;
};

#endif // _POINT_CLOUD_H_
