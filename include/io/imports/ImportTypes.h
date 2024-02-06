#ifndef IMPORT_TYPES_H
#define IMPORT_TYPES_H

#include <glm/glm.hpp>
#include <map>
#include <filesystem>

enum class PositionOptions { KeepModel, ClickPosition, GivenCoordinates };

namespace Import
{
	enum class AsciiValueRole {X, Y, Z, R, Rf, G, Gf, B, Bf, I, Ignore};

	struct AsciiInfo
	{
		std::vector<AsciiValueRole> columnsRole = {};
		char sep = ' ';
		bool useCommaAsDecimal = false;
	};

	struct ScanInfo
	{
		std::vector<std::filesystem::path> paths;
		std::map<std::filesystem::path, Import::AsciiInfo> mapAsciiInfo;
		bool asObject;

		glm::dvec3 positionAsObject;
		PositionOptions positionOption;
	};

}

#endif
