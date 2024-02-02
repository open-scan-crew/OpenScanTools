#ifndef IMPORT_TYPES_H
#define IMPORT_TYPES_H


namespace AsciiImport
{
	enum class ValueRole {X, Y, Z, R, Rf, G, Gf, B, Bf, I, Ignore};

	struct Info
	{
		std::vector<ValueRole> columnsRole = {};
		char sep = ' ';
		bool useCommaAsDecimal = false;
	};

}

#endif
