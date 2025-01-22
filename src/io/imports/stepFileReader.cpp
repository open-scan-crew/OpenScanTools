#include "io/imports/stepFileReader.h"

#include <TopExp_Explorer.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS.hxx>
#include <BRep_Tool.hxx>
#include <BRepTools.hxx>
#include <BRepBndLib.hxx>
#include <Bnd_Box.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <Poly.hxx>
#include <TDocStd_Document.hxx>
#include <TDataStd_Name.hxx>
#include <XCAFDoc_DocumentTool.hxx>

#include <Standard_NumericError.hxx>

#include <StepData_StepModel.hxx>
#include <Interface_Static.hxx>

#include "magic_enum/magic_enum.hpp"
#include "utils/Logger.h"
#include "utils/Utils.h"

#include "controller/Controller.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/SplashScreenTexts.hpp"

#include <XCAFDoc_LengthUnit.hxx>

#include <time.h>




stepFileReader::stepFileReader(Controller* pController, const MeshObjInputData& inputInfo)
	: IMeshReader(pController, inputInfo)
{
	m_errLinCoef = 0.01;
	m_errAngCoef = 0.3;
}

stepFileReader::~stepFileReader()
{

}

bool stepFileReader::read()
{
	IFSelect_ReturnStatus status;

	try
	{
		STEPCAFControl_Reader cafReader;
		cafReader.SetNameMode(true);
		cafReader.SetColorMode(true);

		TCollection_AsciiString aFileName(m_inputInfo.path.wstring().c_str());
		status = cafReader.ReadFile(aFileName.ToCString());

		//float unit = UnitsMethods::GetCasCadeLengthUnit();
		//XSAlgo::AlgoContainer()->PrepareForTransfer(); // update unit info
		//cafReader.ChangeReader().SetSystemLengthUnit(UnitsMethods::GetCasCadeLengthUnit());

		if (!CheckReturnStatus(status))
			return false;

		Handle(TDocStd_Document) doc = new TDocStd_Document("MDTV-XCAF");

		/*TColStd_SequenceOfAsciiString unitLength;
		TColStd_SequenceOfAsciiString unitAngle;
		TColStd_SequenceOfAsciiString unitSolidAngle;

		cafReader.ChangeReader().FileUnits(unitLength, unitAngle, unitSolidAngle);
		std::string unit(unitLength.First().ToCString());*/

		/*std::string unit(Interface_Static::CVal("xstep.cascade.unit"));

		if (unit == "M")
			m_fileScale = 1.f;
		else
			m_fileScale = 0.001f;*/

		if (cafReader.Transfer(doc))
		{
			m_shapeTool = XCAFDoc_DocumentTool::ShapeTool(doc->Main());
			//m_colorTool = XCAFDoc_DocumentTool::ColorTool(doc->Main());

			Handle(XCAFDoc_LengthUnit) lengthUnit;
			if (doc->Main().Root().FindAttribute(XCAFDoc_LengthUnit::GetID(), lengthUnit))
				m_scale = lengthUnit->GetUnitValue();
			else
				m_scale = 0.001f;

			TDF_LabelSequence labels_shapes;
			m_shapeTool->GetFreeShapes(labels_shapes);

			for (int i = 1; i <= labels_shapes.Length(); i++)
			{
				StepShape newShape;
				const TDF_Label& label_shape = labels_shapes.Value(i);
				const TopoDS_Shape& shape = m_shapeTool->GetShape(label_shape);

				if (shape.IsNull())
					continue;

				newShape.name = getName(label_shape);
				newShape.label = label_shape;
				newShape.rootShape = shape;

				recGetSubShapes(newShape);
				
				if (shape.ShapeType() == TopAbs_ShapeEnum::TopAbs_COMPSOLID
					|| shape.ShapeType() == TopAbs_ShapeEnum::TopAbs_SOLID)
					m_solidShapes.push_back(newShape);
			}
		}
	}
	catch (Standard_NumericError&) {
		STEPControl_Reader reader;
		status = reader.ReadFile(m_inputInfo.path.string().c_str());

		if (!CheckReturnStatus(status))
			return false;

		int nbRoot;
		if (nbRoot = reader.TransferRoots())
		{
			for (int i = 1; i <= nbRoot; i++) {
				StepShape newShape;
				const TopoDS_Shape& shape = reader.Shape(i);

				newShape.name = m_inputInfo.path.filename().stem().wstring() + L'_' + std::to_wstring(i);
				newShape.rootShape = shape;
				newShape.label = TDF_Label();

				recGetSubShapes(newShape);

				m_solidShapes.push_back(newShape);
			}
		}
		else
			status = IFSelect_RetFail;

		std::string unit(Interface_Static::CVal("xstep.cascade.unit"));
		m_scale = getScaleFromUnitLabel(unit);
	}

	if (m_solidShapes.size() == 0)
		status = IFSelect_RetVoid;

	m_maxCount = (int)m_solidShapes.size();

	//Set LOD
	double linMax = 1;
	double linMin = 0.001;

	double angMax = M_PI / 2.;
	double angMin = 0.01;

	m_errLinCoef = linMax + (linMin - linMax) * (((double)m_inputInfo.lod - 1) / 99.);
	m_errAngCoef = angMax + (angMin - angMax) * (((double)m_inputInfo.lod - 1) / 99.);

	return CheckReturnStatus(status);
}

void stepFileReader::recGetSubShapes(StepShape& stepShape)
{
	int i = 0;
	std::vector<std::pair<TDF_Label, TopoDS_Shape>> subShapes;

	/*if (!stepShape.label.IsNull())
	{
		TDF_LabelSequence subSequence;
		m_shapeTool->GetSubShapes(stepShape.label, subSequence);

		for (int i = 1; i <= subSequence.Length(); i++)
		{
			TDF_Label subLabel = subSequence.Value(i);
			TopoDS_Shape newSubShape = m_shapeTool->GetShape(subSequence.Value(i));
			subShapes.push_back({ TDF_Label(subSequence.Value(i)), newSubShape });
		}
	}*/

	/*if (!stepShape.label.IsNull())
	{
		TDF_ChildIterator childIter(stepShape.label);

		for (; childIter.More(); childIter.Next())
		{
			TDF_Label subLabel = childIter.Value();
			TopoDS_Shape newSubShape = m_shapeTool->GetShape(subLabel);
			subShapes.push_back({ subLabel, newSubShape });
		}
	}*/

	SafePtr<ClusterNode> cluster = make_safe<ClusterNode>();
	{
		WritePtr<ClusterNode> wClu = cluster.get();
		wClu->setName(stepShape.name);
		wClu->setTreeType(TreeType::MeshObjects);
	}

	if (stepShape.parentCluster)
		AGraphNode::addOwningLink(stepShape.parentCluster, cluster);

	for (TopoDS_Iterator exp(stepShape.rootShape); exp.More(); exp.Next())
	{
		TopoDS_Shape newSubShape = exp.Value();
		TDF_Label newSubLabel;
		m_shapeTool->Search(newSubShape, newSubLabel);
		subShapes.push_back({ newSubLabel, newSubShape });
	}


	for (auto subShape : subShapes) {
		i++;
		TopAbs_ShapeEnum shapeType = subShape.second.ShapeType();

		StepShape sub;
		sub.rootShape = subShape.second;
		sub.parentCluster = cluster;

		if (!subShape.first.IsNull()) {
			sub.label = subShape.first;
			std::wstring lname = getName(sub.label);
			sub.name = (lname.empty()) ? stepShape.name + L'_' + std::to_wstring(i) : lname;
		}
		else 
		{
			sub.label = TDF_Label();
			sub.name = stepShape.name + L'_' + std::to_wstring(i);
		}

		switch (shapeType) {
			case TopAbs_ShapeEnum::TopAbs_COMPOUND:
			{
				recGetSubShapes(sub);
			}
			break;
			case TopAbs_ShapeEnum::TopAbs_SOLID:
			case TopAbs_ShapeEnum::TopAbs_COMPSOLID:
			{
				m_solidShapes.push_back(sub);
			}
			break;
		}
	}
	
}

std::wstring stepFileReader::getName(const TDF_Label& label)
{
	if (label.IsNull())
		return L"";
	Handle(TDataStd_Name) nameData;
	TCollection_ExtendedString occString;
	if (label.FindAttribute(TDataStd_Name::GetID(), nameData))
		occString = nameData->Get();

	return occString.ToWideString();
}


ObjectAllocation::ReturnCode stepFileReader::generateGeometries()
{
	// Traverse faces
	time_t timeNow;
	time(&timeNow);

	//std::unordered_map<std::array<float, 3>, uint32_t, HashVec3> gloNormalsMap;

	MeshShape merge_meshShape;
	merge_meshShape.name = m_inputInfo.path.stem().wstring();

	int addCount = (int)m_solidShapes.size() / 20;
	int count = 0;

	for (StepShape& stepShape : m_solidShapes) {

		count++;

		//std::unordered_map<std::array<float, 3>, uint32_t, HashVec3> normalsMap;

		MeshShape meshShape;
		meshShape.name = stepShape.name.empty() ? m_inputInfo.path.stem().wstring() + L"_" + Utils::wCompleteWithZeros(count) : stepShape.name;
		meshShape.parentCluster = stepShape.parentCluster;

		MeshGeometries& geom = m_inputInfo.isMerge ? merge_meshShape.geometry : meshShape.geometry;

		double xMin, yMin, zMin, xMax, yMax, zMax;
		Bnd_Box box;
		BRepBndLib::Add(stepShape.rootShape, box);
		box.Get(xMin, yMin, zMin, xMax, yMax, zMax);
		glm::vec3 bbMin = glm::vec3(xMin, yMin, zMin);
		glm::vec3 bbMax = glm::vec3(xMax, yMax, zMax);

		double diagLength = glm::distance(bbMin, bbMax);

		TopoDS_Shape shape = stepShape.rootShape;

		BRepTools::Clean(shape);
			

		BRepMesh_IncrementalMesh bMesh(shape, m_errLinCoef * diagLength, false, 0.5, true);
			
		if (!bMesh.IsDone())
			continue;
			
		TopExp_Explorer ExpFace;
		for (ExpFace.Init(shape, TopAbs_FACE); ExpFace.More(); ExpFace.Next())
		{
			const TopoDS_Face& face = TopoDS::Face(ExpFace.Current());

			TopLoc_Location loc;

			const Handle(Poly_Triangulation)& myT = BRep_Tool::Triangulation(face, loc);

			if (!myT
				|| myT.IsNull())
				continue;

			const TopAbs_Orientation& orientation = face.Orientation();

			// Ajout des vertices + index
			for (int i = 1; i <= myT->NbTriangles(); i++)
			{
				int n1, n2, n3;
				myT->Triangle(i).Get(n1, n2, n3);

				if (orientation == TopAbs_REVERSED)
					std::swap(n1, n2);

				std::vector<gp_Pnt> pnts = std::vector<gp_Pnt>(3);

				pnts[0] = myT->Node(n1).Transformed(loc.Transformation());
				pnts[1] = myT->Node(n2).Transformed(loc.Transformation());
				pnts[2] = myT->Node(n3).Transformed(loc.Transformation());

				std::vector<uint32_t> inds = std::vector<uint32_t>(3);

				for (int j = 0; j < 3; j++) 
				{
					std::array<float,3> point = { (float)pnts[j].X(), (float)pnts[j].Y(), (float)pnts[j].Z() };
					inds[j] = geom.addVertice(point, false);
				}

				geom.indices.push_back(inds[0]);
				geom.indices.push_back(inds[1]);
				geom.indices.push_back(inds[2]);

				/*IOLOG << "Normals  : " << norm.x << ", " << norm.y << ", " << norm.z << LOGENDL;
				IOLOG << "Triangle : " << pnts[0].X() << ", " << pnts[0].Y() << ", " << pnts[0].Z() << LOGENDL;
				IOLOG << "           " << pnts[1].X() << ", " << pnts[1].Y() << ", " << pnts[1].Z() << LOGENDL;
				IOLOG << "           " << pnts[2].X() << ", " << pnts[2].Y() << ", " << pnts[2].Z() << LOGENDL;
				IOLOG << LOGENDL;*/
			}

			//Index des edges
			TopExp_Explorer ExpEdge;
			for (ExpEdge.Init(face, TopAbs_EDGE); ExpEdge.More(); ExpEdge.Next())
			{
				const TopoDS_Edge& edge = TopoDS::Edge(ExpEdge.Current());
				const Handle(Poly_PolygonOnTriangulation)& polygon = BRep_Tool::PolygonOnTriangulation(edge, myT, loc);
				if (!polygon || polygon.IsNull())
					continue;
				const TColStd_Array1OfInteger& edgeNodes = polygon->Nodes();

				std::vector<uint32_t> indList;

				for (int i = edgeNodes.Lower(); i <= edgeNodes.Upper(); ++i)
				{
					gp_Pnt pnt = myT->Node(edgeNodes(i)).Transformed(loc.Transformation());
					std::array<float, 3> point = { (float)pnt.X(), (float)pnt.Y(), (float)pnt.Z() };

					uint32_t ind = geom.addVertice(point, true);

					indList.push_back(ind);
				}

				geom.polyligneIndices.push_back(indList);
			}

			if (myT->HasNormals())
			{
				NCollection_Array1<gp_Vec3f> Normals = myT->InternalNormals();
				IOLOG << "Normals size " << Normals.Size() << LOGENDL;
			}
		}


		if (addCount != 0 && count % addCount == 0)
		{
			m_loadCount += addCount;
			if (!updateImportProcessUI(TEXT_SPLASH_SCREEN_IMPORT_GEOMETRIES.arg(count).arg(m_solidShapes.size()), true))
				return ObjectAllocation::ReturnCode::Aborted;
		}

		if (!m_inputInfo.isMerge)
			m_meshesShapes.push_back(std::move(meshShape));
	}
	if (m_inputInfo.isMerge)
		m_meshesShapes.push_back(std::move(merge_meshShape));

	m_loadCount += m_solidShapes.size() % 20;
	if (!updateImportProcessUI(TEXT_SPLASH_SCREEN_IMPORT_GEOMETRIES.arg(count).arg(m_solidShapes.size()), true))
		return ObjectAllocation::ReturnCode::Aborted;

	m_solidShapes.clear();

	time_t endTime;
	time(&endTime);

	IOLOG << "Elapsed time : " << difftime(endTime, timeNow) << " seconds" << LOGENDL;

	return ObjectAllocation::ReturnCode::Success;
}

float stepFileReader::getScaleFromUnitLabel(std::string unitLabel)
{
	std::transform(unitLabel.begin(), unitLabel.end(), unitLabel.begin(), [](unsigned char c) { return std::tolower(c); });

	if (unitLabel == "m")
		return 1.0f;
	if (unitLabel == "mm")
		return 0.001f;
	if (unitLabel == "cm")
		return 0.01f;
	if (unitLabel == "um")
		return 0.000001f;
	if (unitLabel == "km")
		return 1000.0f;
	if (unitLabel == "inch" || unitLabel == "in")
		return 0.0254f;
	if (unitLabel == "mil")
		return 0.0000254f;
	if (unitLabel == "uin")
		return 0.0000000254f;
	if (unitLabel == "ft")
		return 0.3048f;
	if (unitLabel == "mi")
		return 1609.344f;

	return 0.001f;
}

bool stepFileReader::CheckReturnStatus(const IFSelect_ReturnStatus& status) const
{
	bool isDone = false;

	if (status == IFSelect_RetDone)
		isDone = true;
	else
	{
		switch (status)
		{
		case IFSelect_RetError:
			IOLOG << "STEP Import : Not a valid STEP file." << LOGENDL;
			break;
		case IFSelect_RetFail:
			IOLOG << "STEP Import : Reading has failed." << LOGENDL;
			break;
		case IFSelect_RetVoid:
			IOLOG << "STEP Import : Nothing to translate." << LOGENDL;
			break;
		case IFSelect_RetStop:
			IOLOG << "STEP Import : Reading has stopped." << LOGENDL;
			break;
		}
	}

	return isDone;
}