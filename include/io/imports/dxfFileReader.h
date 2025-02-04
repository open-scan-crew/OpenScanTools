#ifndef _DXF_IMPORT_H_
#define _DXF_IMPORT_H_

#include "io/imports/IMeshReader.h"
#include "dl_creationadapter.h"

class dxfFileReader : public IMeshReader, public DL_CreationAdapter
{
public:
	dxfFileReader(Controller* pController, const MeshObjInputData& inputInfo);
	~dxfFileReader();

	virtual bool read() override;
	virtual ObjectAllocation::ReturnCode generateGeometries() override;

    virtual void addLayer(const DL_LayerData& data) override;
    virtual void addPoint(const DL_PointData& data) override;
    virtual void addLine(const DL_LineData& data) override;
    virtual void addPolyline(const DL_PolylineData& data) override;
    virtual void addVertex(const DL_VertexData& data) override;
    virtual void add3dFace(const DL_3dFaceData& data) override;
    virtual void addXLine(const DL_XLineData& data) override;

    virtual void addArc(const DL_ArcData& data) override;
    virtual void addCircle(const DL_CircleData& data) override;
    virtual void addEllipse(const DL_EllipseData& data) override;

    virtual void addBlock(const DL_BlockData& data) override;
    virtual void endBlock() override;

    virtual void addInsert(const DL_InsertData& data) override;

    virtual void endSequence() override;
    virtual void endEntity() override;

    virtual void giveLineReadCount(int& lineCount) override;

    MeshShape& getMeshShapeToEdit();

private:
    uint32_t addVertice(std::array<float, 3> point);
    glm::vec3 convertPositionToWCS(glm::vec3 point);
    std::array<float, 3> convertPositionToWCS(std::array<float, 3> point);

private:
    enum class EntityToRead {CLOSED_POLYLINE, POLYLINE, NONE};

    std::unordered_map<std::string, MeshShape> m_blockShapes;
    std::unordered_map<std::string, MeshShape> m_layersShapes;

    MeshShape m_mergeShape;

    bool m_isMeshBlock = false;
    MeshShape m_blockShape;
    std::string m_blockShapeNameId;

    EntityToRead m_entityToRead = EntityToRead::NONE;
    std::vector<uint32_t> m_currentPolyline;

};

#endif // !_DXF_EXPORT_H_
