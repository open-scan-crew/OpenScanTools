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

	bool text(std::wstring name, int layer, const glm::vec3& center, float radius);
	bool point(float x, float y, float z, int layer);

	bool drawFace(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, int layer);
	bool drawLine(const glm::vec3& p1, const glm::vec3& p2, int layer);
	bool drawTarget(const glm::vec3& center, int layer);

protected:

	std::ofstream m_file;
	bool f_closed;
	bool m_status;
};

#endif // !_DXF_EXPORT_H_
