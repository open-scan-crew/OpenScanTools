#ifndef _GRID_H_
#define _GRID_H_

#include <glm/glm.hpp>

class Grid 
{
public:
    Grid();
    Grid(uint32_t _nbXLines, uint32_t _nbYLines);
    ~Grid();
    
    const void* constData();
    uint32_t getByteSize();
    uint32_t getVertexCount();
    glm::mat4 getModelMatrix() const;

    void setModelMatrix(glm::mat4 _model);

private:
    uint32_t m_xLines;
    uint32_t m_yLines;
    uint32_t m_nbVertices = 0;
    float* m_vertices;
    glm::mat4 m_model;
};

#endif
