#include "io/imports/ifcFileReader.h"
#include "ifcpp/reader/ReaderSTEP.h"
#include "ifcpp/model/UnitConverter.h"

#include "gui/texts/SplashScreenTexts.hpp"

#include "utils/math/basic_functions.h"
#include "utils/math/glm_extended.h"
#include "utils/Utils.h"


#include <BRepTools.hxx>
#include <BRepMesh_IncrementalMesh.hxx>

ifcFileReader::ifcFileReader(Controller* pController, const MeshObjInputData& inputInfo)
	: IMeshReader(pController, inputInfo)
	, m_ifcModel(std::make_shared<BuildingModel>())
	, m_geometryConverter(m_ifcModel)
{
	m_ifcModel->setMessageCallBack(this, [](void* a, std::shared_ptr<StatusCallback::Message> b) { IOLOG << "IFC info - " << QString::fromStdWString(b->m_message_text).toStdString().c_str() << Logger::endl; });
	m_geometryConverter.setMessageCallBack(this, [](void* a, std::shared_ptr<StatusCallback::Message> b) { IOLOG << "IFC info - " << QString::fromStdWString(b->m_message_text).toStdString().c_str() << Logger::endl; });
}

ifcFileReader::~ifcFileReader()
{
}

bool ifcFileReader::read()
{
	ReaderSTEP reader = ReaderSTEP();
	reader.setMessageCallBack(this, [](void* a, std::shared_ptr<StatusCallback::Message> b) { IOLOG << "IFC info - " << QString::fromStdWString(b->m_message_text).toStdString().c_str() << Logger::endl; });

	reader.loadModelFromFile(m_inputInfo.path.wstring(), m_ifcModel);

	bool isCancel = reader.isCanceled();

	m_scale = 1.f;

	//m_geometryConverter.setCsgEps(7.5e-06);

	double fact = 0.1 + ((m_inputInfo.lod - 1) / 99.) * 0.75;
	IOLOG << "IFC - fact " << fact << LOGENDL;
	IOLOG << LOGENDL;
	m_geometryConverter.getGeomSettings()->setNumVerticesPerCircle(4 + 12 * fact);
	IOLOG << "IFC - numVert per cercle " << m_geometryConverter.getGeomSettings()->getNumVerticesPerCircle() << LOGENDL;
	m_geometryConverter.getGeomSettings()->setMinNumVerticesPerArc(2 + 6 * fact);
	IOLOG << "IFC - minNumVert per arc " << m_geometryConverter.getGeomSettings()->getMinNumVerticesPerArc() << LOGENDL;
	m_geometryConverter.getGeomSettings()->setNumVerticesPerControlPoint(1);
	IOLOG << "IFC - numVert per controlpoint " << m_geometryConverter.getGeomSettings()->getNumVerticesPerControlPoint() << LOGENDL;
	IOLOG << LOGENDL;
	m_geometryConverter.getGeomSettings()->setCoplanarFacesMaxDeltaAngle(M_PI * 0.02 * (0.5/fact));
	IOLOG << "IFC - coplanar maxDelta " << m_geometryConverter.getGeomSettings()->getCoplanarFacesMaxDeltaAngle() << LOGENDL;
	m_geometryConverter.getGeomSettings()->setCreaseEdgesMaxDeltaAngle(M_PI * 0.05 * (0.5/fact));
	IOLOG << "IFC - creaseEdge maxDelta " << m_geometryConverter.getGeomSettings()->getCreaseEdgesMaxDeltaAngle() << LOGENDL;
	
	
	m_geometryConverter.convertGeometry();

	m_maxCount = m_geometryConverter.getShapeInputData().size();

	return true;
}

ObjectAllocation::ReturnCode ifcFileReader::generateGeometries()
{
	MeshShape merge_meshShape;

	if (m_ifcModel->getIfcProject() && m_ifcModel->getIfcProject()->m_LongName)
		merge_meshShape.name = m_ifcModel->getIfcProject()->m_LongName->toString();

	if(merge_meshShape.name.empty())
		merge_meshShape.name = m_inputInfo.path.stem().wstring();

	int productDataSize = m_geometryConverter.getShapeInputData().size();
	int addCount = productDataSize / 20;
	int count = 0;

	uint64_t numVertices = 0;

	for (auto productShape : m_geometryConverter.getShapeInputData())
	{
		count++;

		if (!productShape.second || productShape.second->m_vec_representations.empty())
			continue;

		for (size_t i_item = 0; i_item < productShape.second->m_vec_representations.size(); i_item++)
		{
			std::shared_ptr<RepresentationData> representation  = productShape.second->m_vec_representations[i_item];

			representation->applyTransformToRepresentation(productShape.second->getTransform());
		}

		shared_ptr<IfcObjectDefinition> ifc_object_def(productShape.second->m_ifc_object_definition);
		shared_ptr<IfcSpace> space = dynamic_pointer_cast<IfcSpace>(ifc_object_def);
		if (space != nullptr)
			continue;

		MeshShape meshShape;

        if (ifc_object_def && ifc_object_def->m_Name)
			meshShape.name = ifc_object_def->m_Name->toString();
		
		if(meshShape.name.empty())
			meshShape.name = m_inputInfo.path.stem().wstring() + L"_" + Utils::wCompleteWithZeros(count);

		loadMesh(productShape.second, m_inputInfo.isMerge ? merge_meshShape.geometry : meshShape.geometry);

		numVertices += m_inputInfo.isMerge ? merge_meshShape.geometry.vertices.size() : meshShape.geometry.vertices.size();

		if (addCount != 0 && count % addCount == 0)
		{
			m_loadCount += addCount;
			if (!updateImportProcessUI(TEXT_SPLASH_SCREEN_IMPORT_GEOMETRIES.arg(count).arg(productDataSize), true))
				return ObjectAllocation::ReturnCode::Aborted;
		}
		
		if (!m_inputInfo.isMerge)
			m_meshesShapes.push_back(std::move(meshShape));
	}

	if (m_inputInfo.isMerge)
		m_meshesShapes.push_back(std::move(merge_meshShape));

	IOLOG << "IFC vertices size : " << numVertices << LOGENDL;

	m_loadCount += productDataSize % 20;
	if (!updateImportProcessUI(TEXT_SPLASH_SCREEN_IMPORT_GEOMETRIES.arg(count).arg(productDataSize), true))
		return ObjectAllocation::ReturnCode::Aborted;

	m_geometryConverter.clearInputCache();
	
	return ObjectAllocation::ReturnCode::Success;
}

void ifcFileReader::loadMesh(std::shared_ptr<ProductShapeData> product, MeshGeometries& geom)
{
	for (shared_ptr<RepresentationData> representationData : product->m_vec_representations)
	{
		if (representationData->m_ifc_representation.expired())
			continue;

		for (shared_ptr<ItemShapeData> item : representationData->m_vec_item_data)
		{
			meshSetFillGeom(item->m_meshsets, geom);

			meshSetFillGeom(item->m_meshsets_open, geom);

			/*for (auto polyligne : item->m_polylines)
			{
				for (auto ligne : polyligne->polylines)
				{
					std::vector<uint32_t> lig;
					for (int ind : ligne.second)
					{
						carve::geom3d::Vector p = polyligne->getVertex(ind);
						std::array<float, 3> p_ = { p.x, p.y, p.z };
						lig.push_back(addPoint(p_, geom.vertices, mapVertices));
					}

					geom.polyligneIndices.push_back(lig);
				}
			}*/
		}
	}

}

void ifcFileReader::loadMeshOcc(std::shared_ptr<ProductShapeDataOCC> product, MeshGeometries& geom)
{
	for (shared_ptr<RepresentationDataOCC> representationData : product->m_vec_representations)
	{
		if (representationData->m_ifc_representation.expired())
			continue;

		for (shared_ptr<ItemShapeDataOCC> item : representationData->m_vec_item_data)
		{
			shapeOCCFillGeom(item->getShapes(), geom);
		}
	}
}

void ifcFileReader::meshSetFillGeom(const std::vector<shared_ptr<carve::mesh::MeshSet<3>>>& meshSets, MeshGeometries& geom)
{
	float edgeLimitAngle = (float)M_PI / 6.f;

	for (auto meshSet : meshSets)
	{
		CSG_Adapter::retriangulateMeshSet(meshSet);
		for (auto mesh : meshSet->meshes)
		{
			for (auto face : mesh->faces)
			{
				assert(face->nVertices() == 3);
				assert(face->nEdges() == 3);

				std::vector<carve::mesh::Vertex<3U>*> vertices;
				face->getVertices(vertices);

				for (auto vert : vertices)
				{
					std::array<float, 3> point = { (float)vert->v.x, (float)vert->v.y, (float)vert->v.z };
					geom.indices.push_back(geom.addVertice(point, false));
				}
				/*
				carve::mesh::Edge<3>* edg = face->edge;

				for (int i = 0; i < 3; i++)
				{
					carve::geom3d::Vector vertex = edg->vert->v;

					std::array<float, 3> point = { vertex.x, vertex.y, vertex.z };
					geom.indices.push_back(addPoint(point, geom.vertices, mapVertices));

					edg = edg->next;
				}*/
			}

			for (auto edge : mesh->open_edges)
			{
				carve::geom3d::Vector v1 = edge->v1()->v;
				carve::geom3d::Vector v2 = edge->v2()->v;

				std::array<float, 3> p1 = { (float)v1.x, (float)v1.y, (float)v1.z };
				std::array<float, 3> p2 = { (float)v2.x, (float)v2.y, (float)v2.z };

				geom.edgesIndices.push_back(geom.addVertice(p1, true));
				geom.edgesIndices.push_back(geom.addVertice(p2, true));
			}

			for (auto edge : mesh->closed_edges)
			{
				carve::geom3d::Vector n1 = edge->rev->face->plane.N;
				carve::geom3d::Vector n2 = edge->face->plane.N;

				float angle = glm_extended::angleBetweenV3(glm::vec3(n1.x, n1.y, n1.z), glm::vec3(n2.x, n2.y, n2.z));
				if (angle < edgeLimitAngle)
					continue;

				carve::geom3d::Vector v1 = edge->v1()->v;
				carve::geom3d::Vector v2 = edge->v2()->v;

				std::array<float, 3> p1 = { (float)v1.x, (float)v1.y, (float)v1.z };
				std::array<float, 3> p2 = { (float)v2.x, (float)v2.y, (float)v2.z };

				geom.edgesIndices.push_back(geom.addVertice(p1, true));
				geom.edgesIndices.push_back(geom.addVertice(p2, true));
			}
		}
	}
}

void ifcFileReader::shapeOCCFillGeom(const std::vector<TopoDS_Shape>& shapeSet, MeshGeometries& geom)
{

	for (const TopoDS_Shape& shape : shapeSet)
	{
		double xMin, yMin, zMin, xMax, yMax, zMax;
		Bnd_Box box;
		BRepBndLib::Add(shape, box);
		box.Get(xMin, yMin, zMin, xMax, yMax, zMax);
		glm::vec3 bbMin = glm::vec3(xMin, yMin, zMin);
		glm::vec3 bbMax = glm::vec3(xMax, yMax, zMax);

		double diagLength = glm::distance(bbMin, bbMax);

		BRepTools::Clean(shape);


		BRepMesh_IncrementalMesh bMesh(shape, 0.01 * diagLength, false, 0.5, true);

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
					std::array<float, 3> point = { (float)pnts[j].X(), (float)pnts[j].Y(), (float)pnts[j].Z() };
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
	}

	
}
