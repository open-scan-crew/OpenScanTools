#ifndef _IFC_FILE_READER_H_
#define _IFC_FILE_READER_H_

#include "io/imports/IMeshReader.h"

#include <ifcpp/model/BuildingModel.h>
#include "ifcpp/geometry/Carve/GeometryConverter.h"
#include "ifcpp/geometry/OCC/GeometryConverterOCC.h"

class ProductShapeData;

class ifcFileReader : public IMeshReader
{
public:
	ifcFileReader(Controller* pController, const MeshObjInputData& inputInfo);
	~ifcFileReader();

	bool read() override;
	ObjectAllocation::ReturnCode generateGeometries() override;

	void loadMesh(std::shared_ptr<ProductShapeData> product, MeshGeometries& geom);

	void loadMeshOcc(std::shared_ptr<ProductShapeDataOCC> product, MeshGeometries& geom);

	void meshSetFillGeom(const std::vector<shared_ptr<carve::mesh::MeshSet<3>>>& meshSet, MeshGeometries& geom);

	void shapeOCCFillGeom(const std::vector<TopoDS_Shape>& shapeSet, MeshGeometries& geom);

private:
	std::shared_ptr<BuildingModel> m_ifcModel;
	GeometryConverter m_geometryConverter;

	
};

#endif // !_STEP_FILE_READER_H_
