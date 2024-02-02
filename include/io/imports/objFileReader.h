#ifndef _OBJ_FILE_READER_H_
#define _OBJ_FILE_READER_H_

#include "io/imports/IMeshReader.h"
#include "tiny_obj_loader.h"

namespace ObjectAllocation {
	enum class ReturnCode;
}

class Controller;

class ObjFileReader : public IMeshReader
{
public:
	ObjFileReader(Controller* pController, const MeshObjInputData& inputInfo);
	~ObjFileReader();

	bool read() override;
	ObjectAllocation::ReturnCode generateGeometries() override;

private:
	void getVertices(const tinyobj::attrib_t& attrib, MeshGeometries& geom) const;
	/// <summary>
	///  Take a tinyobj::mesh_t and convert the mulitiple indexes description to a single 
	/// index description. In the process we may end with duplicates of the attributes
	/// as we need to identify with a unique index a pair of vertex & normal.
	/// '_vn' means that we only convert the vertices(v) and normals(n) attributes.
	/// 
	///  As the corresponding map for indexes is passed by ref, we use this function to
	/// merge multiple shapes.
	/// </summary>
	/// <param name="mesh"></param>
	/// <param name="attrib"></param>
	/// <param name="correspIndexes">The map use to store if we already have encountered a pair of indexes</param>
	/// <param name="geom"></param>
	void mapShapeToGeometry_vn(const tinyobj::mesh_t& mesh, const tinyobj::attrib_t& attrib, std::unordered_map<uint64_t, uint32_t>& correspIndexes, MeshGeometries& geom);

	void getVertexIndices(const std::vector<tinyobj::index_t>& indices, std::vector<uint32_t>& buffer) const;

private:
	tinyobj::ObjReader* m_reader;
};

#endif // !_STEP_FILE_READER_H_
