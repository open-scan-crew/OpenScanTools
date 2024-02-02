#include "models/3d/Grid.h"
#include <cstring>

// Constructeurs
Grid::Grid()
{
    m_xLines = 2;
    m_yLines = 2;
    m_nbVertices = 8; // 2 * 2 + 2 * 2
    m_vertices = new float[m_nbVertices * 6] {0.0f, 0.0f, 0.0f, 0.7f, 1.0f, 0.7f, // x, y, z, r, g, b
                  0.0f, 5.0f, 0.0f, 0.7f, 1.0f, 0.7f,
        // line x = 5
                  5.0f, 0.0f, 0.0f, 0.7f, 1.0f, 0.7f,
                  5.0f, 5.0f, 0.0f, 0.7f, 1.0f, 0.7f,
        // line y = 0
                  0.0f, 0.0f, 0.0f, 0.7f, 0.7f, 1.0f,
                  5.0f, 0.0f, 0.0f, 0.7f, 0.7f, 1.0f,
        // line y = 5
                  0.0f, 5.0f, 0.0f, 0.7f, 0.7f, 1.0f,
                  5.0f, 5.0f, 0.0f, 0.7f, 0.7f, 1.0f};
    m_model = glm::mat4(1.f);
}

Grid::Grid(uint32_t _nbXLines, uint32_t _nbYLines) :
    m_xLines(_nbXLines), m_yLines(_nbYLines)
{
    m_nbVertices = 2 * m_xLines + 2 * m_yLines;
    m_vertices = new float[m_nbVertices * 6];

    // Lines parallel to (Ox)
    for (unsigned int y = 0; y < m_xLines; y++)
    {
        float firstVertice[] = {0.0, (float)y, 0.0, 1.0, 0.5, 0.5};
        float secondVertice[] = {(float)m_yLines - 1, (float)y, 0.0, 1.0, 0.5, 0.5};
        memcpy(&m_vertices[(y * 12)], firstVertice, 6 * sizeof(float));
        memcpy(&m_vertices[(y * 12 + 6)], secondVertice, 6 * sizeof(float));
    }

    //Lines parallel to (Oy)
    size_t yOffset = m_xLines * 12;
    for (unsigned int x = 0; x < m_yLines; x++)
    {
        float firstVertice[] = {(float)x, 0.0, 0.0, 0.5, 1.0, 0.5};
        float secondVertice[] = {(float)x, (float)m_xLines - 1, 0.0, 0.5, 1.0, 0.5};
        memcpy(&m_vertices[yOffset + x * 12], firstVertice, 6 * sizeof(float));
        memcpy(&m_vertices[yOffset + x * 12 + 6], secondVertice, 6 * sizeof(float));
    }

}

Grid::~Grid()
{
    delete[] m_vertices;
}

const void* Grid::constData()
{
    return (void*)m_vertices;
}

uint32_t Grid::getByteSize()
{
    return m_nbVertices * 6 * sizeof(float);
}

uint32_t Grid::getVertexCount()
{
    return m_nbVertices;
}

glm::mat4 Grid::getModelMatrix() const
{
    return m_model;
}

void Grid::setModelMatrix(glm::mat4 _model)
{
    m_model = _model;
}