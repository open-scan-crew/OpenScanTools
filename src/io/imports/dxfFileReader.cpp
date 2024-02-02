#include "io/imports/dxfFileReader.h"

#include <iostream>
#include <stdlib.h>
#include <stdio.h>

#include "utils/Utils.h"
#include "gui/texts/SplashScreenTexts.hpp"

#include "dl_dxf.h"

#define SECTOR_COUNT 32

dxfFileReader::dxfFileReader(Controller* pController, const MeshObjInputData& inputInfo)
    : IMeshReader(pController, inputInfo)
    , DL_CreationAdapter()
{
}

dxfFileReader::~dxfFileReader()
{}

bool dxfFileReader::read()
{
    m_maxCount = 0;
    std::ifstream in(m_inputInfo.path);
    std::string unused;
    while (std::getline(in, unused))
        ++m_maxCount;
    //La lecture se fait dans le generateGeometries
    return true;
}

ObjectAllocation::ReturnCode dxfFileReader::generateGeometries()
{
    std::filesystem::path file = m_inputInfo.path;
    std::cout << "Reading file " << file << "...\n";

    updateImportProcessUI(TEXT_SPLASH_SCREEN_IMPORT_LINES.arg(0).arg(m_maxCount), false);

    m_mergeShape.name = m_inputInfo.path.filename().stem().wstring();
    DL_Dxf dxf;

    bool readSucces = false;

    std::ifstream istr(file);
    istr.imbue(std::locale(istr.getloc()));
    if (dxf.in(istr, this))
        readSucces = true;

    if(!readSucces)
    {
        std::cerr << file << " could not be opened.\n";
        return ObjectAllocation::ReturnCode::Load_File_Error;
    }


    updateImportProcessUI(TEXT_SPLASH_SCREEN_IMPORT_LINES.arg(m_maxCount).arg(m_maxCount), false);

    m_meshesShapes.push_back(m_mergeShape);

    if (!m_inputInfo.isMerge)
    {
        for (auto& pair : m_layersShapes)
            m_meshesShapes.push_back(std::move(pair.second));
    }

    m_blockShapes.clear();
    m_layersShapes.clear();

    return ObjectAllocation::ReturnCode::Success;
}

void dxfFileReader::addLayer(const DL_LayerData& data)
{
    if (attributes.isInPaperSpace())
        return;
    if (m_inputInfo.isMerge || data.name == "0")
        return;

    std::wstring name = Utils::decode(data.name);
    SafePtr<ClusterNode> cluster = make_safe<ClusterNode>();
    {
        WritePtr<ClusterNode> wClu = cluster.get();
        wClu->setName(name);
        wClu->setTreeType(TreeType::MeshObjects);
    }

    std::string clusterName = attributes.getLayer();
    if (m_layersShapes.find(clusterName) != m_layersShapes.end())
        AGraphNode::addOwningLink(m_layersShapes.at(clusterName).parentCluster, cluster);

    MeshShape layerShape;
    layerShape.parentCluster = cluster;
    layerShape.name = name;

    m_layersShapes[data.name] = layerShape;
}

void dxfFileReader::addPoint(const DL_PointData& data)
{
    if (attributes.isInPaperSpace())
        return;

    const float dAngle((float)M_PI * 2.0f / (float)SECTOR_COUNT);
    constexpr float circlesize = 0.04f;

    std::vector<uint32_t> indices;
    for (uint32_t i = 0; i <= SECTOR_COUNT; ++i)
    {
        float x = circlesize * cosf(dAngle * i);
        float y = circlesize * sinf(dAngle * i);

        std::array<float, 3> point1 = { x + (float)data.x, y + (float)data.y, (float)data.z };

        uint32_t ind = addVertice(point1);

        indices.push_back(ind);
    }
    getMeshShapeToEdit().geometry.polyligneIndices.push_back(indices);

    constexpr float csize = 0.06f;

    std::array<float, 3> ur = {(float)data.x + csize, (float)data.y + csize, (float)data.z };
    std::array<float, 3> dl = {(float)data.x - csize, (float)data.y - csize, (float)data.z };
    getMeshShapeToEdit().geometry.edgesIndices.push_back(addVertice(ur));
    getMeshShapeToEdit().geometry.edgesIndices.push_back(addVertice(dl));


    std::array<float, 3> ul = {(float)data.x - csize, (float)data.y + csize, (float)data.z };
    std::array<float, 3> dr = {(float)data.x + csize, (float)data.y - csize, (float)data.z };
    getMeshShapeToEdit().geometry.edgesIndices.push_back(addVertice(ul));
    getMeshShapeToEdit().geometry.edgesIndices.push_back(addVertice(dr));


}

void dxfFileReader::addLine(const DL_LineData& data)
{
    if (attributes.isInPaperSpace())
        return;

    MeshShape& shapeToEdit = getMeshShapeToEdit();

    std::array<float, 3> point1 = { (float)data.x1, (float)data.y1, (float)data.z1 };
    std::array<float, 3> point2 = { (float)data.x2, (float)data.y2, (float)data.z2 };

    uint32_t ind1 = addVertice(point1);
    uint32_t ind2 = addVertice(point2);

    shapeToEdit.geometry.edgesIndices.push_back(ind1);
    shapeToEdit.geometry.edgesIndices.push_back(ind2);
}

void dxfFileReader::addArc(const DL_ArcData& data)
{
    if (attributes.isInPaperSpace())
        return;

    bool invert = extrusion->getDirection()[2] < 0;

    double startAngle = invert ? data.angle2 : data.angle1;
    double endAngle = invert ? data.angle1 : data.angle2;

    double angle = (endAngle - startAngle) * M_PI / 180.;
    while (angle < 0)
        angle += M_PI * 2.0;

    uint32_t recalcSectorCount = (angle / (2.0 * M_PI)) * SECTOR_COUNT;
    const float dAngle(angle / (float)recalcSectorCount);

    std::vector<uint32_t> indices;
    for (uint32_t i = 0; i <= recalcSectorCount; ++i)
    {
        float x = data.radius * cosf(dAngle * i + startAngle * M_PI / 180.);
        float y = data.radius * sinf(dAngle * i + startAngle * M_PI / 180.);

        std::array<float, 3> point1 = { x + (float)data.cx, y + (float)data.cy, (float)data.cz };

        indices.push_back(addVertice(point1));
    }

    getMeshShapeToEdit().geometry.polyligneIndices.push_back(indices);
}

void dxfFileReader::addCircle(const DL_CircleData& data)
{
    if (attributes.isInPaperSpace())
        return;

    const float dAngle((float)M_PI * 2.0f / (float)SECTOR_COUNT);

    std::vector<uint32_t> indices;
    for (uint32_t i = 0; i <= SECTOR_COUNT; ++i)
    {
        float x = data.radius * cosf(dAngle * i);
        float y = data.radius * sinf(dAngle * i);

        std::array<float, 3> point1 = { x + (float)data.cx, y + (float)data.cy, (float)data.cz };

        uint32_t ind = addVertice(point1);

        indices.push_back(ind);
    }

    getMeshShapeToEdit().geometry.polyligneIndices.push_back(indices);
}

void dxfFileReader::addEllipse(const DL_EllipseData& data)
{
}

void dxfFileReader::addPolyline(const DL_PolylineData& data)
{
    if (attributes.isInPaperSpace())
        return;

    if (data.flags == 1)
        m_entityToRead = EntityToRead::CLOSED_POLYLINE;
    else
        m_entityToRead = EntityToRead::POLYLINE;
}

void dxfFileReader::addVertex(const DL_VertexData& data)
{
    if (attributes.isInPaperSpace())
        return;

    std::array<float, 3> point = { (float)data.x, (float)data.y, (float)data.z };

    switch (m_entityToRead)
    {
        case EntityToRead::CLOSED_POLYLINE:
        case EntityToRead::POLYLINE:
        {
            m_currentPolyline.push_back(addVertice(point));
            break;
        }
    }
}

void dxfFileReader::add3dFace(const DL_3dFaceData& data)
{
}

void dxfFileReader::addXLine(const DL_XLineData& data)
{
}

void dxfFileReader::addBlock(const DL_BlockData& data)
{
    m_blockShape = MeshShape();
    m_blockShapeNameId = data.name;
    m_blockShape.name = Utils::decode(data.name);
    m_blockShape.center = glm::vec3((float)data.bpx, (float)data.bpy, (float)data.bpz);

    m_isMeshBlock = true;

    if (m_inputInfo.isMerge)
        return;

    std::string clusterName = attributes.getLayer();
    if (m_layersShapes.find(clusterName) != m_layersShapes.end())
        m_blockShape.parentCluster = m_layersShapes.at(clusterName).parentCluster;
}

void dxfFileReader::endBlock()
{
    m_blockShapes[m_blockShapeNameId] = m_blockShape;
    m_blockShapeNameId = "";

    m_isMeshBlock = false;
}

void dxfFileReader::addInsert(const DL_InsertData& data)
{
    if (attributes.isInPaperSpace())
        return;

    if (m_blockShapes.find(data.name) == m_blockShapes.end())
        return;

    MeshShape shapeToAdd = m_blockShapes.at(data.name);
    MeshGeometries& geomToAdd = shapeToAdd.geometry;
    if (geomToAdd.vertices.empty())
        return;

    if (data.cols > 1 || data.rows > 1)
        int i = 1;

    std::vector<float> shapeVertsCopy = std::move(geomToAdd.vertices);

    glm::vec3 dim;
    glm::vec3 dumpCenter;
    std::array<glm::vec3, 2> dump;

    if (data.cols > 1 || data.rows > 1)
        IMeshReader::getBoundingBox(shapeVertsCopy, dim, dumpCenter, dump);

    glm::vec3 translation = glm::vec3((float)data.ipx, (float)data.ipy, (float)data.ipz) - shapeToAdd.center;
    glm::vec3 scale = glm::vec3((float)data.sx, (float)data.sy, (float)data.sz);

    float angle = (float)(data.angle * M_PI / 180.);
    glm::quat quat = glm::angleAxis(angle, glm::vec3(0,0,1));

    assert((shapeVertsCopy.size() % 3) == 0);
    for (int i = 0; i < data.rows; i++)
        for (int j = 0; i < data.cols; i++)
        {
            glm::vec3 realTranslation = translation + glm::vec3(i * (data.rowSp + dim[0]), 0, 0) + glm::vec3(0, j * (data.colSp + dim[1]), 0);
            for (uint64_t iterator(0); iterator < shapeVertsCopy.size() - 2; iterator += 3)
            {
                glm::vec3 vertex(shapeVertsCopy[iterator],
                    shapeVertsCopy[iterator + 1],
                    shapeVertsCopy[iterator + 2]);

                vertex *= scale;
                vertex = quat * vertex;
                vertex += realTranslation;

                vertex = convertPositionToWCS(vertex);

                std::array<float, 3> point = { vertex.x, vertex.y, vertex.z };
                geomToAdd.vertices.push_back(point[0]);
                geomToAdd.vertices.push_back(point[1]);
                geomToAdd.vertices.push_back(point[2]);
            }
        }

    std::string clusterName = attributes.getLayer();
    if (m_layersShapes.find(clusterName) != m_layersShapes.end())
        shapeToAdd.parentCluster = m_layersShapes.at(clusterName).parentCluster;

    if (!m_inputInfo.isMerge)
        m_meshesShapes.push_back(shapeToAdd);
    else
        m_mergeShape.geometry.merge(geomToAdd);
}

void dxfFileReader::endSequence()
{
}

void dxfFileReader::endEntity()
{
    if (attributes.isInPaperSpace())
        return;

    switch (m_entityToRead)
    {
        case EntityToRead::CLOSED_POLYLINE:
        {
            if (!m_currentPolyline.empty())
                m_currentPolyline.push_back(m_currentPolyline.front());
        }
        case EntityToRead::POLYLINE:
        {
            getMeshShapeToEdit().geometry.polyligneIndices.push_back(m_currentPolyline);
            m_currentPolyline.clear();
            break;
        }
    }

    m_entityToRead = EntityToRead::NONE;
}

void dxfFileReader::giveLineReadCount(int& lineCount)
{
    int count = (lineCount - 1) / 2;
    int maxCount = m_maxCount / 25;
    if ((count % maxCount) == 0)
    {
        editLoadCountUI() = lineCount;
        updateImportProcessUI(TEXT_SPLASH_SCREEN_IMPORT_LINES.arg(lineCount).arg(m_maxCount), false);
    }
}

MeshShape& dxfFileReader::getMeshShapeToEdit()
{
    std::string layer = attributes.getLayer();

    if (m_isMeshBlock)
        return m_blockShape;

    if (m_layersShapes.find(layer) != m_layersShapes.end())
        return m_layersShapes.at(layer);

    return m_mergeShape;
}

uint32_t dxfFileReader::addVertice(std::array<float, 3> point)
{
    point = convertPositionToWCS(point);
    return getMeshShapeToEdit().geometry.addVertice(point, false);
}

glm::vec3 dxfFileReader::convertPositionToWCS(glm::vec3 point)
{
    std::array<float, 3> arrayPoint = { point.x, point.y, point.z };
    std::array<float, 3> convertPoint = convertPositionToWCS(arrayPoint);
    return glm::vec3(convertPoint[0], convertPoint[1], convertPoint[2]);
}

std::array<float, 3> dxfFileReader::convertPositionToWCS(std::array<float, 3> point)
{
    glm::vec3 Az = glm::vec3(extrusion->getDirection()[0], extrusion->getDirection()[1], extrusion->getDirection()[2]);
    Az = glm::normalize(Az);
    if (Az == glm::vec3(0, 0, 1))
        return point;

    glm::vec3 Ax;
    if ((abs(Az.x) < 1 / 64.) && (abs(Az.y) < 1 / 64.))
        Ax = glm::normalize(glm::cross(glm::vec3(0, 1, 0), Az));
    else
        Ax = glm::normalize(glm::cross(glm::vec3(0, 0, 1), Az));
    glm::vec3 Ay = glm::normalize(glm::cross(Az, Ax));

    auto wcs_to_ocs = [Ax, Ay, Az](glm::vec3 p)
    {
        float x = p.x * Ax.x + p.y * Ax.y + p.z * Ax.z;
        float y = p.x * Ay.x + p.y * Ay.y + p.z * Ay.z;
        float z = p.x * Az.x + p.y * Az.y + p.z * Az.z;
        return glm::vec3(x, y, z);
    };
    glm::vec3 Wx = wcs_to_ocs(glm::vec3(1, 0, 0));
    glm::vec3 Wy = wcs_to_ocs(glm::vec3(0, 1, 0));
    glm::vec3 Wz = wcs_to_ocs(glm::vec3(0, 0, 1));

    float x, y, z;
    x = point[0] * Wx.x + point[1] * Wx.y + point[2] * Wx.z;
    y = point[0] * Wy.x + point[1] * Wy.y + point[2] * Wy.z;
    z = point[0] * Wz.x + point[1] * Wz.y + point[2] * Wz.z;

    return { x, y, z };
}