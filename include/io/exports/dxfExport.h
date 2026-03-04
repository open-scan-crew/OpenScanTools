#ifndef _DXF_EXPORT_H_
#define _DXF_EXPORT_H_

#include <string>
#include <filesystem>
#include <fstream>

#include <glm/glm.hpp>

class dxfExport
{
public:
	dxfExport();
	~dxfExport();

	bool begin(const std::filesystem::path& fileName);
	bool end();

	bool text(std::wstring name, int layer, const glm::dvec3& center, double radius);
	bool point(double x, double y, double z, int layer);

	bool drawFace(const glm::dvec3& p1, const glm::dvec3& p2, const glm::dvec3& p3, int layer);
	bool drawLine(const glm::dvec3& p1, const glm::dvec3& p2, int layer);
	bool drawTarget(const glm::dvec3& center, int layer);

protected:

	std::ofstream m_file;
	bool f_closed;
	bool m_status;
};

#endif // !_DXF_EXPORT_H_
