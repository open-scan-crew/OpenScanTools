#ifndef _IFC_EXPORT_H_
#define _IFC_EXPORT_H_

class ifcExport
{
public:
	/*
	stepExport();
	~stepExport();

	// 3D Objects converters
	primitiveId makeBox(const glm::dvec3&  size, const glm::dvec3&  center, const glm::quat& orientation);
	primitiveId makeCylinder(double radius, double length, const glm::dvec3& center, const glm::dvec3& axis, const glm::dquat& orientation);

	primitiveId makePoint(const glm::dvec3&  center);
	primitiveId makeScan(const glm::dvec3&  center);
	primitiveId makeWire(const std::vector<glm::dvec3>& points, const glm::dvec3& center);
	primitiveId makeSphere(double radius, const glm::dvec3&  center);

	primitiveId makeTorus(double R, double r, double angle, const glm::dvec3& center, const glm::dquat& orientation);


	void setName(primitiveId primId, const std::string& name);
	void setDiameter(primitiveId primId, double tubeRadius);
	void setRootName(const std::string& name);
	void setColor(primitiveId primId, const Color32& color);

	TDF_Label addNewParent(const std::string& name, const TDF_Label& parent = TDF_Label());

	// add shape to the export document
	void addShape(primitiveId primId, const TDF_Label& parent = TDF_Label());

	// save in file
	bool write(const std::filesystem::path& filename, const std::string& author, const std::string& company);

	void printLabel(TDF_Label label) const;

private:

	void addShape(Primitive& prim, const TDF_Label& parent = TDF_Label());
	primitiveId addPrimitive(Primitive prim);
	void applyRotate(primitiveId primId, const glm::dvec3& center, const glm::quat& orientation);*/

};

#endif // !_IFC_EXPORT_H_
