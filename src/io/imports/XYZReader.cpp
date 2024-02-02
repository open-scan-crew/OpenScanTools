/*** PointCloud.cpp ***********************************************************
** Author           | Robin Kervadec                                         **
**                                                                           **
** TODO insert description                                                   ** 
**                                                                           **
******* Copyright (C) 2018 TagLabs *******************************************/

#include "XYZReader.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstring>

PointCloud::PointCloud() :
    m_nbPoints(0),
    m_nbChunks(0),
    m_readyToLoad(false),
    m_readyToDraw(false)
{ }

PointCloud::PointCloud(const std::string& _filePath)
{
    tryOpenFile(_filePath);
}

PointCloud::~PointCloud()
{
    // nothing special
}

void PointCloud::tryOpenFile(const std::string& _filePath)
{
    std::ifstream fileXYZ(_filePath, std::ios::in);

    if(!fileXYZ.is_open())
        throw std::runtime_error("unable to open point cloud file.");
    else {
        m_filePath = _filePath;
    }

    fileXYZ.seekg(0);
    m_nbPoints = std::count(std::istreambuf_iterator<char>(fileXYZ), std::istreambuf_iterator<char>(), '\n');
    std::cout << "debug_pc: Nombre de points : " << m_nbPoints << std::endl;

    m_nbChunks = (m_nbPoints / POINTS_PER_CHUNK); // whole chunks
    std::cout << "debug_pc: Nombre de chunks : " << m_nbChunks << std::endl;

    fileXYZ.close();
    m_readyToLoad = true;
}

void PointCloud::loadToMem(void* _p)
{
    std::cout << "debug_pc: Starting the loading of the point cloud" << std::endl;
    // Verify if the file name is initialized
    if (m_filePath.size() == 0)
    {
        throw std::runtime_error("Point cloud: no file path.");
    }

    // Verify if the point cloud has been initialized
    if (!m_readyToLoad || m_nbPoints == 0)
    {
        throw std::runtime_error("Point cloud: nothing to load.");
    }

    std::ifstream fileXYZ(m_filePath, std::ios::in);

    if (!fileXYZ.is_open())
    {
        throw std::runtime_error("Point cloud: unable to open cloud point file during loading.");
    }

    float points[POINTS_PER_CHUNK * 4] = {0.0};
    fileXYZ.seekg(0);
    size_t bytesPerChunk = POINTS_PER_CHUNK * 4 * sizeof(float);

    std::cout << "Loading Point Cloud";
    std::cout.flush();
    uint32_t j_ = 0;
    for (uint32_t j = 0; j < m_nbChunks; j++) {
        for (uint32_t i = 0; i < POINTS_PER_CHUNK; i++) {
            fileXYZ >> points[i * 4];     // X
            fileXYZ >> points[i * 4 + 1]; // Y
            fileXYZ >> points[i * 4 + 2]; // Z
            fileXYZ >> points[i * 4 + 3]; // Intensity
        }
        memcpy((uint8_t*)_p + j * bytesPerChunk, points, bytesPerChunk);
        if (j_ < (j * 60) / m_nbChunks) {
            j_++;        
            std::cout << ".";
            std::cout.flush();
        }
    }
    std::cout << std::endl;

    size_t nbPtsLastChunk = m_nbPoints - POINTS_PER_CHUNK * m_nbChunks;
    for (uint32_t i = 0; i < nbPtsLastChunk; i++) {
            fileXYZ >> points[i * 4];
            fileXYZ >> points[i * 4 + 1];
            fileXYZ >> points[i * 4 + 2];
            fileXYZ >> points[i * 4 + 3];
    }
    memcpy((uint8_t*)_p + m_nbChunks * bytesPerChunk, points, (size_t)(nbPtsLastChunk * 4 * sizeof(float)));

    fileXYZ.close();
    m_readyToDraw = true;
}

uint32_t PointCloud::getByteSize()
{
    // (x, y, z, I) for each point
    return m_nbPoints * 4 * sizeof(float);
}

uint32_t PointCloud::getVertexCount()
{
    return m_nbPoints;
}

bool PointCloud::isReadyToLoad()
{
    return m_readyToLoad;
}

bool PointCloud::isReadyToDraw()
{
    return m_readyToDraw;
}
