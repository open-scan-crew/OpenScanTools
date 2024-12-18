#include "io/exports/dxfExport.h"
#include "utils/Utils.h"

#define DELTA_CM 0.005

dxfExport::dxfExport()
	: f_closed(true)
	, m_status(false)
{}

dxfExport::~dxfExport()
{
	end();
}

bool dxfExport::begin(const std::filesystem::path & fileName)
{
	if (f_closed == false)
		return (false);

	m_file.open(fileName);
	if (m_file.bad())
		return (false);
	f_closed = false;
	m_status = true;

	// === Header section of every dxf file. 
	m_file << 0 << std::endl;
	m_file << "SECTION" << std::endl;
	m_file << 2 << std::endl;
	m_file << "HEADER" << std::endl;
	m_file << 9 << std::endl;
	m_file << "$ACADVER" << std::endl;
	m_file << 1 << std::endl;
	m_file << "AC1009" << std::endl;

	m_file << 9 << std::endl;
	m_file << "$INSBASE" << std::endl;
	m_file << 10 << std::endl;
	m_file << 0.0 << std::endl;
	m_file << 20 << std::endl;
	m_file << 0.0 << std::endl;
	m_file << 30 << std::endl;
	m_file << 0.0 << std::endl;

	m_file << 0 << std::endl;
	m_file << "ENDSEC" << std::endl;

	m_file << 0 << std::endl;
	m_file << "SECTION" << std::endl;
	m_file << 2 << std::endl;
	m_file << "ENTITIES" << std::endl;

	return (m_file.bad());
}

bool dxfExport::end()
{
	if (f_closed == false)
	{
		// end of sequence objects of dxf file.
		m_file << 0 << std::endl;
		m_file << "ENDSEC" << std::endl;
		m_file << 0 << std::endl;
		m_file << "EOF";

		m_file.close();
		f_closed = true;
	}
	return m_status;
}

bool dxfExport::text(std::wstring name, int layer, const glm::vec3& center, float radius)
{
	m_file << "  " << 0 << std::endl; // entity type
	m_file << "TEXT" << std::endl;

	// l'espace avec le point est à l'espace juste avant le nom
	m_file << "  " << 1 << std::endl; // layer name
	m_file << " " << Utils::to_utf8(name) << std::endl;

	m_file << "  " << 8 << std::endl;
	m_file << layer << std::endl;

	m_file << " " << 10 << std::endl;
	m_file << center.x << std::endl;

	m_file << " " << 20 << std::endl;
	m_file << center.y << std::endl;

	m_file << " " << 30 << std::endl;
	m_file << center.z << std::endl;

	m_file << " " << 40 << std::endl;
	m_file << radius << std::endl;

	return (m_file.bad());
}

bool dxfExport::point(float x, float y, float z, int layer)
{
	m_file << "  " << 0 << std::endl; // entity type
	m_file << "POINT" << std::endl;

	m_file << 8 << std::endl;
	m_file << layer << std::endl;

	m_file << " " << 10 << std::endl;
	m_file << x << std::endl;

	m_file << " " << 20 << std::endl;
	m_file << y << std::endl;

	m_file << " " << 30 << std::endl;
	m_file << z << std::endl;

	return (m_file.bad());
}

bool dxfExport::drawFace(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, int layer)
{
	m_file << "  " << 0 << std::endl; // entity type
	m_file << "3DFACE" << std::endl;

	m_file << 8 << std::endl;
	m_file << layer << std::endl;
	
	m_file << " " << 10 << std::endl;
	m_file << p1.x << std::endl;

	m_file << " " << 20 << std::endl;
	m_file << p1.y << std::endl;

	m_file << " " << 30 << std::endl;
	m_file << p1.z << std::endl;

	m_file << " " << 11 << std::endl;
	m_file << p2.x << std::endl;

	m_file << " " << 21 << std::endl;
	m_file << p2.y << std::endl;

	m_file << " " << 31 << std::endl;
	m_file << p2.z << std::endl;

	m_file << " " << 12 << std::endl;
	m_file << p3.x << std::endl;

	m_file << " " << 22 << std::endl;
	m_file << p3.y << std::endl;

	m_file << " " << 32 << std::endl;
	m_file << p3.z << std::endl;

	return (m_file.bad());
}

bool dxfExport::drawLine(const glm::vec3& p1, const glm::vec3& p2, int layer)
{
	m_file << "  " << 0 << std::endl; // entity type
	m_file << "LINE" << std::endl;

	m_file << 8 << std::endl;
	m_file << layer << std::endl;

	m_file << " " << 10 << std::endl;
	m_file << p1.x << std::endl;

	m_file << " " << 20 << std::endl;
	m_file << p1.y << std::endl;

	m_file << " " << 30 << std::endl;
	m_file << p1.z << std::endl;

	m_file << " " << 11 << std::endl;
	m_file << p2.x << std::endl;

	m_file << " " << 21 << std::endl;
	m_file << p2.y << std::endl;

	m_file << " " << 31 << std::endl;
	m_file << p2.z << std::endl;

	return (m_file.bad());
}

bool dxfExport::drawTarget(const glm::vec3& center, int layer)
{
	drawLine(center - glm::vec3(DELTA_CM, 0, 0), center + glm::vec3(DELTA_CM, 0, 0), layer);
	drawLine(center - glm::vec3(0, DELTA_CM, 0), center + glm::vec3(0, DELTA_CM, 0), layer);
	drawLine(center - glm::vec3(0, 0, DELTA_CM), center + glm::vec3(0, 0, DELTA_CM), layer);
	return (m_file.bad());
}