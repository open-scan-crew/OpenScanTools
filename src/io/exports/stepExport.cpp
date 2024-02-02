#include "io/exports/stepExport.h"

#include <TDocStd_Application.hxx>
#include <TDocStd_Document.hxx>
#include <Interface_Static.hxx>
#include <NCollection_Vector.hxx>
#include <STEPCAFControl_Writer.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <XCAFDoc_ColorTool.hxx>
#include <XCAFDoc_LayerTool.hxx>
#include <XCAFDoc_DimTolTool.hxx>
#include <XCAFDoc_MaterialTool.hxx>
#include <XCAFDoc_NotesTool.hxx>
#include <XCAFDoc_Note.hxx>
#include <XCAFApp_Application.hxx>
#include <XCAFDoc_Location.hxx>
#include <XCAFDoc_Centroid.hxx>

#include <XCAFDoc_ColorType.hxx>

#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepPrimAPI_MakeTorus.hxx>
#include <BRepPrimAPI_MakeCone.hxx>

#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>

#include <BRepBuilderAPI_Transform.hxx>
#include <gp_Quaternion.hxx>
#include <gp_Circ.hxx>

#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

#include <TDataStd_Name.hxx>
#include <TDataStd_Comment.hxx>
#include <TDataStd_Integer.hxx>
#include <TDataStd_NamedData.hxx>

#include <TDF_Tool.hxx>
#include <TopoDS_Iterator.hxx>

#include <APIHeaderSection_MakeHeader.hxx>
#include <XCAFDoc_LengthUnit.hxx>
#include <StepData_StepModel.hxx>
#include <XSAlgo.hxx>
#include <XSAlgo_AlgoContainer.hxx>
#include <UnitsMethods.hxx>

#include "utils/math/trigo.h"
#include "utils/math/glm_extended.h"
#include "utils/Logger.h"
#include "utils/Utils.h"
#include "utils/OpenScanToolsVersion.h"

#include "utils/Color32.hpp"

#include "models/3d/GeometryGenerator.h"

#define IOLOG Logger::log(LoggerMode::IOLog)

struct _StepDocInternals {
	_StepDocInternals() : application(XCAFApp_Application::GetApplication())
	{
		// Create document
		application->NewDocument("MDTV-XCAF", document);

		// Get tools
		shapeTool = XCAFDoc_DocumentTool::ShapeTool(document->Main());
		colorTool = XCAFDoc_DocumentTool::ColorTool(document->Main());
		materialTool = XCAFDoc_DocumentTool::MaterialTool(document->Main());
		layerTool = XCAFDoc_DocumentTool::LayerTool(document->Main());
		dimTolTool = XCAFDoc_DocumentTool::DimTolTool(document->Main());
		notesTool = XCAFDoc_DocumentTool::NotesTool(document->Main());
	}
	// Base attributes
	Handle(TDocStd_Document) document;
	Handle(TDocStd_Application) application;

	// Tools
	Handle(XCAFDoc_ShapeTool) shapeTool;
	Handle(XCAFDoc_ColorTool) colorTool;
	Handle(XCAFDoc_MaterialTool) materialTool;
	Handle(XCAFDoc_LayerTool) layerTool;
	Handle(XCAFDoc_DimTolTool) dimTolTool;
	Handle(XCAFDoc_NotesTool) notesTool;

	std::vector<TDF_Label> rootChildrens;
};

stepExport::stepExport() : m_internals(new _StepDocInternals)
{
	//m_builder = BRep_Builder();
	//m_builder.MakeCompound(m_rootShape);
	//m_rootShape = TopoDS_Compound();
	m_rootShapeLabel = m_internals->shapeTool->NewShape();

	//m_internals->shapeTool->SetShape(m_rootShapeLabel, m_rootShape);
	//m_rootShapeLabel = TDF_TagSource::NewChild(m_internals->document->Main());
	//m_internals->shapeTool = XCAFDoc_DocumentTool::ShapeTool(m_rootShapeLabel);
	//m_rootShapeLabel = m_internals->shapeTool->AddShape(m_rootShape);
	TDataStd_Name::Set(m_rootShapeLabel, "Root");
	/*
	assert(m_internals->shapeTool->GetShape(m_rootShapeLabel, m_rootShape));*/
	//
	
}

stepExport::~stepExport()
{
	m_internals->application->Close(m_internals->document);
	delete m_internals;
}


// 3DOBjects converters
primitiveId stepExport::makeBox(const glm::dvec3& size, const glm::dvec3& center, const glm::quat& orientation)
{
	BRepPrimAPI_MakeBox box = BRepPrimAPI_MakeBox(gp_Pnt(-size.x / 2, -size.y / 2, -size.z / 2), size.x, size.y, size.z);
	box.Build();

	Primitive prim;
	if(box.IsDone())
		prim.shape = box.Solid();
	else
		assert(false);
	primitiveId id = addPrimitive(prim);
	applyRotate(id, center, orientation);

	

	return id;
}

primitiveId stepExport::makeCylinder(double radius, double length, const glm::dvec3& center, const glm::dvec3& axis, const glm::dquat& orientation)
{
	glm::dvec3 startPos = center - (axis * (length/2));

	BRepPrimAPI_MakeCylinder cylinder = BRepPrimAPI_MakeCylinder(radius, length);
	cylinder.Build();

	BRepBuilderAPI_MakeEdge centerLine = BRepBuilderAPI_MakeEdge(gp_Pnt(0., 0., 0.),
																gp_Pnt(0., 0., length));
	centerLine.Build();

	BRep_Builder builder = BRep_Builder();
	TopoDS_Compound comp;
	builder.MakeCompound(comp);

	Primitive prim;
	prim.shape = comp;

	Primitive cylinderPrim;
	cylinderPrim.name = L"Cylinder Shape";
	if(cylinder.IsDone())
		cylinderPrim.shape = cylinder.Solid();
	else
		assert(false);
	prim.subPrimes.push_back(cylinderPrim);

	Primitive centerLinePrim;
	centerLinePrim.name = L"Center Line";
	if(centerLine.IsDone())
		centerLinePrim.shape = centerLine.Edge();
	else
		assert(false);
	prim.subPrimes.push_back(centerLinePrim);
	
	primitiveId id = addPrimitive(prim);

	applyRotate(id, startPos, orientation);
	
	return id;
}

primitiveId stepExport::makePoint(const glm::dvec3& center)
{

	BRep_Builder builder = BRep_Builder();
	BRepBuilderAPI_MakeWire point = BRepBuilderAPI_MakeWire();
	double crossLength = 0.2;
	double diameter = 0.15;
	BRepBuilderAPI_MakeEdge xLine = BRepBuilderAPI_MakeEdge(gp_Pnt(-crossLength/2., 0., 0.),
															gp_Pnt(crossLength/2., 0., 0.));
	xLine.Build();
	BRepBuilderAPI_MakeEdge yLine = BRepBuilderAPI_MakeEdge(gp_Pnt(0., crossLength / 2., 0.),
															gp_Pnt(0., -crossLength / 2., 0.));
	yLine.Build();
	BRepBuilderAPI_MakeEdge circl = BRepBuilderAPI_MakeEdge(gp_Circ(gp_Ax2(
															gp_Pnt(0., 0., 0.), gp::DZ()), diameter/2.));
	circl.Build();
	point.Add(circl.Edge());

	assert(point.IsDone());
	point.Build();

	TopoDS_Wire wire = point.Wire();
	builder.Add(wire, xLine.Edge());
	builder.Add(wire, yLine.Edge());

	Primitive prim;
	prim.shape = wire;
	prim.location.SetTranslation(gp_Vec(center.x, center.y, center.z));

	primitiveId id = addPrimitive(prim);

	return id;
}

primitiveId stepExport::makeScan(const glm::dvec3 & center)
{
	double r = 0.1;
	double h = 0.15;
	BRepPrimAPI_MakeCone cone = BRepPrimAPI_MakeCone(gp_Ax2(gp_Pnt(0, 0, h), gp_Dir(0,0,1)),0, r, h);
	cone.Build();


	Primitive prim;
	if (cone.IsDone())
		prim.shape = cone.Solid();
	else
		assert(false);
	prim.location.SetTranslation(gp_Vec(center.x, center.y, center.z));

	primitiveId id = addPrimitive(prim);

	return id;
}

primitiveId stepExport::makeWire(const std::vector<glm::dvec3>& points, const glm::dvec3& center)
{
	BRepBuilderAPI_MakeWire wire = BRepBuilderAPI_MakeWire();
	/*glm::dvec3 centerPoint = glm::dvec3(0.,0.,0.);
	for (glm::dvec3 point : points) {
		centerPoint += point;
	}
	centerPoint = glm::dvec3(centerPoint.x / (double)points.size(), centerPoint.y / (double)points.size(), centerPoint.z / (double)points.size());*/
	for (int i = 1; i < points.size(); i++) {
		glm::dvec3 point1 = points[i - 1] - center;
		glm::dvec3 point2 = points[i] - center;
		BRepBuilderAPI_MakeEdge edge = BRepBuilderAPI_MakeEdge(gp_Pnt(point1.x, point1.y, point1.z),
																gp_Pnt(point2.x, point2.y, point2.z));

		edge.Build();
		if (edge.IsDone())
			wire.Add(edge.Edge());
		//else assert(false);
	}
	wire.Build();

	Primitive prim;
	if (wire.IsDone())
		prim.shape = wire.Wire();
	//else assert(false);

	prim.location.SetTranslation(gp_Vec(center.x, center.y, center.z));

	primitiveId id = addPrimitive(prim);

	return id;
}

primitiveId stepExport::makeSphere(double radius, const glm::dvec3& center)
{
	BRepPrimAPI_MakeSphere sphere = BRepPrimAPI_MakeSphere(radius);
	sphere.Build();


	Primitive prim;
	if (sphere.IsDone())
		prim.shape = sphere.Solid();
	else
		assert(false);
	prim.location.SetTranslation(gp_Vec(center.x, center.y, center.z));

	primitiveId id = addPrimitive(prim);

	return id;
}

primitiveId stepExport::makeTorus(double R, double r, double angle, const glm::dvec3& center, const glm::dquat& orientation)
{
	BRepPrimAPI_MakeTorus torus = BRepPrimAPI_MakeTorus(R, r, angle);
	torus.Build();

	uint32_t centerSectorCount = 24;

	BRepBuilderAPI_MakeWire centerWire = BRepBuilderAPI_MakeWire();
	for (uint32_t i = 1; i <= centerSectorCount; i++)
	{
		double angPart = (angle / centerSectorCount) * (i - 1);
		gp_Pnt point1 = gp_Pnt(R * cos(angPart), R * sin(angPart), 0.);

		angPart = (angle / centerSectorCount) * i;
		gp_Pnt point2 = gp_Pnt(R * cos(angPart), R * sin(angPart), 0.);
		BRepBuilderAPI_MakeEdge edge = BRepBuilderAPI_MakeEdge(point1, point2);
		edge.Build();
		if (edge.IsDone())
			centerWire.Add(edge.Edge());
		else
			assert(false);
	}
	centerWire.Build();


	std::vector<glm::vec3> centerEdgeVert;

	BRep_Builder builder = BRep_Builder();
	TopoDS_Compound comp;
	builder.MakeCompound(comp);

	Primitive prim;
	prim.shape = comp;

	Primitive torusPrim;
	torusPrim.name = L"Elbow Shape";
	if (torus.IsDone())
		torusPrim.shape = torus.Solid();
	else
		assert(false);
	prim.subPrimes.push_back(torusPrim);

	
	Primitive centerLinePrim;
	centerLinePrim.name = L"Center Line";
	if (centerWire.IsDone())
		centerLinePrim.shape = centerWire.Wire();
	else
		assert(false);
	prim.subPrimes.push_back(centerLinePrim);

	primitiveId id = addPrimitive(prim);

	applyRotate(id, center, orientation);

	return id;
}


primitiveId stepExport::addPrimitive(Primitive prim)
{
	primitiveId id = (uint32_t)m_primitives.size();
	m_primitives[id] = prim;
	return id;
}

void stepExport::applyRotate(primitiveId primId, const glm::dvec3& center, const glm::quat& orientation)
{
	m_primitives[primId].location.SetTransformation(gp_Quaternion(orientation.x, orientation.y, orientation.z, orientation.w), gp_Vec(center.x, center.y, center.z));

	//m_primitives[primId].location.SetTranslationPart(gp_Vec(center.x, center.y, center.z));
	//m_primitives[primId].location.SetRotation(gp_Quaternion(orientation.x, orientation.y, orientation.z, orientation.w));
	
	return;
}

void stepExport::setName(primitiveId primId, const std::wstring& name)
{
	m_primitives[primId].name = name;
}

void stepExport::setDiameter(primitiveId primId, double diameter)
{
	m_primitives[primId].diameter = diameter;
}

void stepExport::setRootName(const std::string & name)
{
	TDataStd_Name::Set(m_rootShapeLabel, name.c_str());
}

void stepExport::setColor(primitiveId primId, const Color32 & color)
{
	Quantity_Color qColor = Quantity_Color(color.r / 255.f, color.g / 255.f, color.b / 255.f, Quantity_TypeOfColor::Quantity_TOC_RGB);
	m_primitives[primId].color = qColor;
	for (Primitive& subprim : m_primitives[primId].subPrimes)
		subprim.color = qColor;
}

TDF_Label stepExport::addNewParent(const std::wstring& name, const TDF_Label &parent)
{
	TDF_Label newParent = m_internals->shapeTool->NewShape();
	if (!parent.IsNull())
		XCAFDoc_DocumentTool::ShapeTool(m_rootShapeLabel)->AddComponent(parent, newParent, TopLoc_Location());
	else
		XCAFDoc_DocumentTool::ShapeTool(m_rootShapeLabel)->AddComponent(m_rootShapeLabel, newParent, TopLoc_Location());

	TDataStd_Name::Set(newParent, name.c_str());
	return newParent;
}


//addShape to the document functions 
void stepExport::addShape(primitiveId primId, const TDF_Label& parent) {

	assert(!m_primitives[primId].shape.IsNull());
	if (m_primitives[primId].shape.IsNull())
		return;

	addShape(m_primitives[primId], parent);
	
	for (Primitive& subprim : m_primitives[primId].subPrimes)
		if(!subprim.shape.IsNull())
			addShape(subprim, m_primitives[primId].label);



	return;
}

void stepExport::addShape(Primitive& prim, const TDF_Label& parent)
{
	TopLoc_Location locShape = prim.location;
	TDF_Label label;

	/*if (!prim.subPrimes.empty())
	{
		BRep_Builder builder = BRep_Builder();
		for (Primitive subprim : prim.subPrimes)
			if(!subprim.shape.IsNull())
				builder.Add(prim.shape, subprim.shape);
	}*/
	if (prim.label.IsNull())
		prim.label = m_internals->shapeTool->AddShape(prim.shape, false);
	//printLabel(prim.label);

	assert(!prim.label.IsNull());

	if (parent.IsNull())
		m_internals->shapeTool->AddComponent(m_rootShapeLabel, prim.label, locShape);
	else
		m_internals->shapeTool->AddComponent(parent, prim.label, locShape);

	std::wstring primName = prim.name;
	if (prim.diameter > 0.)
		primName += L"_" + Utils::wRoundFloat(prim.diameter * 1000.) + L"mm";

	TDataStd_Name::Set(prim.label, primName.c_str());

	m_internals->colorTool->SetColor(prim.label, prim.color, XCAFDoc_ColorType::XCAFDoc_ColorGen);
}

//write in file
bool stepExport::write(const std::filesystem::path& filename, const std::wstring& author, const std::wstring& company) {

	m_internals->shapeTool->UpdateAssemblies();

	STEPCAFControl_Writer writer;

	if (!Interface_Static::SetCVal("write.step.schema", "AP242DIS"))
		assert(false);

	Interface_Static::SetIVal("write.step.vertex.mode", 1);

	if (!Interface_Static::SetIVal("write.step.assembly", 2))
		assert(false && "failed to set assembly mode for step data");


	//writer.SetMaterialMode(true);
	writer.SetDimTolMode(true);
	writer.SetLayerMode(true);
	//writer.SetPropsMode(true);
	writer.SetColorMode(true);
	writer.SetNameMode(true);
	//writer.SetSHUOMode(true);

	XCAFDoc_LengthUnit::Set(m_internals->document->Main().Root(), "m", 1.0);

	if (!writer.Transfer(m_internals->document, STEPControl_StepModelType::STEPControl_AsIs)) {
		assert(false);
		return false;
	}

	STEPControl_Writer a = writer.Writer();

	APIHeaderSection_MakeHeader makeHeader(a.Model());

	makeHeader.SetName(new TCollection_HAsciiString(filename.filename().c_str()));
	makeHeader.SetAuthorValue(1, new TCollection_HAsciiString(author.c_str()));
	makeHeader.SetOrganizationValue(1, new TCollection_HAsciiString(company.c_str()));
	std::string ostVer = std::string("OpenScanTools - ") + std::string(OPENSCANTOOLS_VERSION);
	makeHeader.SetOriginatingSystem(new TCollection_HAsciiString(ostVer.c_str()));
	makeHeader.SetDescriptionValue(1, new TCollection_HAsciiString(""));

	IFSelect_ReturnStatus ret = writer.Write(filename.string().c_str());
	if (ret == IFSelect_RetError || ret == IFSelect_RetFail || ret == IFSelect_RetStop || ret == IFSelect_RetVoid) {
		assert(false);
		return false;
	}

	return true;
}


void stepExport::printLabel(TDF_Label label) const
{
	if (label.IsNull())
		return;
	Handle(TDataStd_Name) name;
	label.FindAttribute(TDataStd_Name::GetID(), name);

	Handle(XCAFDoc_ShapeTool) shapeTool = m_internals->shapeTool;

	IOLOG << "Label: , " << name->Get()
		<< (shapeTool->IsShape(label) ? ", shape" : "")
		<< (shapeTool->IsTopLevel(label) ? ", topLevel" : "")
		<< (shapeTool->IsFree(label) ? ", free" : "")
		<< (shapeTool->IsAssembly(label) ? ", assembly" : "")
		<< (shapeTool->IsSimpleShape(label) ? ", simple" : "")
		<< (shapeTool->IsCompound(label) ? ", compound" : "")
		<< (shapeTool->IsReference(label) ? ", reference" : "")
		<< (shapeTool->IsComponent(label) ? ", component" : "")
		<< (shapeTool->IsSubShape(label) ? ", subshape" : "") << LOGENDL;
}