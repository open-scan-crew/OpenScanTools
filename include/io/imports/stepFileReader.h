#ifndef _STEP_FILE_READER_H_
#define _STEP_FILE_READER_H_

#include <STEPCAFControl_Reader.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <TopoDS_Shape.hxx>

#include <glm/glm.hpp>

#include "io/imports/IMeshReader.h"

namespace ObjectAllocation {
	enum class ReturnCode;
}

class Controller;

struct StepBoundingBox
{
	glm::vec3 min = glm::vec3(std::numeric_limits<double>::min());
	glm::vec3 max = glm::vec3(std::numeric_limits<double>::max());
};


struct StepShape {
	std::wstring name;

	TDF_Label label;
	TopoDS_Shape rootShape;

	SafePtr<ClusterNode> parentCluster;
	//std::vector<StepShape> subShapes;
	//StepBoundingBox bbox;
};

class stepFileReader : public IMeshReader
{
public:
	stepFileReader(Controller* pController, const MeshObjInputData& inputInfo);
	~stepFileReader();

	bool read() override;
	ObjectAllocation::ReturnCode generateGeometries() override;

	float getScaleFromUnitLabel(std::string unitLabel);

private:
	void recGetSubShapes(StepShape& rootShape);
	std::wstring getName(const TDF_Label& label);
	bool CheckReturnStatus(const IFSelect_ReturnStatus& status) const;
	
private:
	Handle(XCAFDoc_ShapeTool) m_shapeTool;

	std::vector<StepShape> m_solidShapes;

	double m_errLinCoef;
	double m_errAngCoef;
};

#endif // !_STEP_FILE_READER_H_
