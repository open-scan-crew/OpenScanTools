#include "pointCloudEngine/OctreeDecoder.h"
#include <map>

OctreeDecoder::OctreeDecoder(OctreeBase const& _base, tls::BoundingBox const& _bbox)
    : OctreeBase(_base)
    , m_isEncodedBuffered(false)
    , m_isDecoded(false)
    , m_encodedBuffers(std::vector<char*>(m_cellCount, 0)) // Isn't it better with nullptr ?
    , m_decodedBuffers(std::vector<PointXYZIRGB*>(m_cellCount, 0))
	, m_bbox(_bbox)
{
}

OctreeDecoder::OctreeDecoder(OctreeBase const& _base)
	: OctreeBase(_base)
	, m_isEncodedBuffered(false)
	, m_isDecoded(false)
	, m_encodedBuffers(std::vector<char*>(m_cellCount, nullptr)) // NOTE - changed 0 to nullptr. Does it work?
	, m_decodedBuffers(std::vector<PointXYZIRGB*>(m_cellCount, nullptr)) // NOTE - changed 0 to nullptr
{
}

OctreeDecoder::~OctreeDecoder()
{
	cleanBuffers();
}

void OctreeDecoder::cleanBuffers()
{
	cleanEncodedBuffers();

	if (m_decodedBuffers.empty())
		return;
	for (PointXYZIRGB* dbuf : m_decodedBuffers)
		if (dbuf)
			delete dbuf;
	m_decodedBuffers.clear();
}

void OctreeDecoder::cleanEncodedBuffers()
{
	if (m_encodedBuffers.empty())
		return;
	for (char* ebuf : m_encodedBuffers)
		if (ebuf)
			delete ebuf;
	m_encodedBuffers.clear();
}

bool OctreeDecoder::readPointsFromFile(std::ifstream& _is, const uint64_t& pointDataOffset, const uint64_t& pointDataSize)
{
    // NOTE(robin) - Organize the leaves to read them in the file order (faster)
    std::map<uint64_t, uint32_t> orderedCellToRead;

    for (uint32_t cellId(0); cellId < m_vTreeCells.size(); cellId++)
    {
        if (!m_vTreeCells[cellId].m_isLeaf)
            continue;

        orderedCellToRead.insert({ m_vTreeCells[cellId].m_dataOffset, cellId });
    }

    for (std::pair<uint64_t, uint32_t> cellOffset : orderedCellToRead)
    {
        m_encodedBuffers[cellOffset.second] = new char[m_vTreeCells[cellOffset.second].m_dataSize];

        _is.seekg(pointDataOffset + cellOffset.first);
        _is.read(m_encodedBuffers[cellOffset.second], m_vTreeCells[cellOffset.second].m_dataSize);
    }
    m_isEncodedBuffered = true;

    return true;
}

// FIXME(robin) - Pas de vérification sur la taille de l'espace pointé par `optionalOutput`
void OctreeDecoder::decodeCell(uint32_t cellId, PointXYZIRGB* optionalOutput)
{
    assert(cellId == NO_CHILD || cellId >= m_vTreeCells.size());
    const TreeCell& cell = m_vTreeCells[cellId];
    uint64_t nbOfPoints = cell.m_layerIndexes[cell.m_depthSPT];

    // Select the source of the buffered encoded points 
    const char* srcPoints = m_encodedBuffers[cellId];

    // Select the destination
    PointXYZIRGB* points;
    if (optionalOutput != nullptr)
        points = optionalOutput;
    else
    {
        if (m_decodedBuffers[cellId] != nullptr)
            delete m_decodedBuffers[cellId];
        m_decodedBuffers[cellId] = new PointXYZIRGB[nbOfPoints];
        points = m_decodedBuffers[cellId];
    }

    for (uint64_t dataIterator(0); dataIterator < nbOfPoints; dataIterator++)
    {
        Coord16& coord = ((Coord16*)srcPoints)[dataIterator];
        points[dataIterator].x = coord.x * m_precisionValue + cell.m_position[0];
        points[dataIterator].y = coord.y * m_precisionValue + cell.m_position[1];
        points[dataIterator].z = coord.z * m_precisionValue + cell.m_position[2];
    }
    if (m_ptFormat != tls::PointFormat::TL_POINT_FORMAT_UNDEFINED)
    {
        if (m_ptFormat != tls::PointFormat::TL_POINT_XYZ_RGB)
        {
            uint8_t* packI = (uint8_t*)(srcPoints + cell.m_iOffset);
            for (uint64_t dataIterator(0); dataIterator < nbOfPoints; dataIterator++)
            {
                points[dataIterator].i = packI[dataIterator];
            }
        }

        if (m_ptFormat != tls::PointFormat::TL_POINT_XYZ_I)
        {
            Color24* packRGB = (Color24*)(srcPoints + cell.m_rgbOffset);
            for (uint64_t dataIterator(0); dataIterator < nbOfPoints; dataIterator++)
            {
                points[dataIterator].r = packRGB[dataIterator].r;
                points[dataIterator].g = packRGB[dataIterator].g;
                points[dataIterator].b = packRGB[dataIterator].b;
            }
        }
    }
}

uint32_t OctreeDecoder::getCellCount()
{
    return (uint32_t)m_vTreeCells.size();
}

uint32_t OctreeDecoder::getCellPointCount(uint32_t cellId)
{
    return m_vTreeCells[cellId].m_layerIndexes[m_vTreeCells[cellId].m_depthSPT];
}

bool OctreeDecoder::isLeaf(uint32_t cellId) const
{
    return (m_vTreeCells[cellId].m_isLeaf);
}

const PointXYZIRGB* OctreeDecoder::getCellPoints(uint32_t cellId, uint64_t& pointCount)
{
    if (cellId >= m_vTreeCells.size())
    {
#ifdef _DEBUG_
        std::cout << "Error: OctreeDecoder::getCellPoints() -> Wrong cell ID: " << cellId << std::endl;
#endif
        pointCount = 0;
        return nullptr;
    }

    if (m_decodedBuffers[cellId] != nullptr)
    {
        pointCount = getCellPointCount(cellId);
        return m_decodedBuffers[cellId];
    }
    else if (m_isEncodedBuffered)
    {
        if (!m_vTreeCells[cellId].m_isLeaf)
        {
#ifdef _DEBUG_
            std::cout << "Error: OctreeDecoder::getCellPoints() -> The cell " << cellId << " is not a leaf." << std::endl;
#endif
            pointCount = 0;
            return nullptr;
        }

        decodeCell(cellId);

        // Free the buffered memory
        delete[] m_encodedBuffers[cellId];
        m_encodedBuffers[cellId] = nullptr;

        // Result
        pointCount = m_vTreeCells[cellId].m_layerIndexes[m_vTreeCells[cellId].m_depthSPT];
        return m_decodedBuffers[cellId];
    }
    else
    {
#ifdef _DEBUG_
        std::cout << "Error: OctreeDecoder::getCellPoints() -> Cell points not buffered." << std::endl;
#endif
        pointCount = 0;
        return nullptr;
    }
}

bool OctreeDecoder::copyCellPoints(uint32_t cellId, PointXYZIRGB* dstPoints, uint64_t dstSize, uint64_t& dstOffset)
{
    if (cellId >= m_vTreeCells.size())
    {
#ifdef _DEBUG_
        std::cout << "Error: OctreeDecoder::getCellPoints() -> Wrong cell ID: " << cellId << std::endl;
#endif
        return false;
    }

    if (!m_vTreeCells[cellId].m_isLeaf)
    {
#ifdef _DEBUG_
        std::cout << "Error: OctreeDecoder::getCellPoints() -> The cell " << cellId << " is not a leaf." << std::endl;
#endif
        return false;
    }

    const uint64_t& nbOfPoints = getCellPointCount(cellId);

    // Check that there is enough space in the destination buffer
    if (dstOffset + nbOfPoints > dstSize)
        return false;

    if (m_decodedBuffers[cellId] != nullptr)
    {
        memcpy(dstPoints + dstOffset, m_decodedBuffers[cellId], nbOfPoints * sizeof(PointXYZIRGB));
        dstOffset += nbOfPoints;
        return true;
    }
    else if (m_isEncodedBuffered)
    {
        // shorthand pointer
        PointXYZIRGB* points = dstPoints + dstOffset;
        dstOffset += nbOfPoints;

        // decode the cell directly in the destination buffer
        decodeCell(cellId, points);

        return true;
    }
    else
    {
#ifdef _DEBUG_
        std::cout << "Error: OctreeDecoder::getCellPoints() -> Cell points not buffered." << std::endl;
#endif
        return false;
    }
}

const tls::BoundingBox&  OctreeDecoder::getBBox() const
{
	return m_bbox;
}