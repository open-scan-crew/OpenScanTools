#ifndef IMAGE_TYPES_H
#define IMAGE_TYPES_H

#include <map>
#include <glm/glm.hpp>
#include <string>

enum class ImageFormat {
    BMP = 0,
    JPG,
    JPEG,
    PNG,
	TIFF,
    MAX_ENUM
    /*NOTE Not working : GIF,*/
    /*PBM, PGM, PPM, XBM, XPM*/
};

const std::map<ImageFormat, std::string> ImageFormatDictio = {
	{ ImageFormat::BMP, "BMP" },
	//{ ImageFormat::GIF, "GIF" },
	{ ImageFormat::JPG, "JPG" },
	{ ImageFormat::JPEG, "JPEG" },
	{ ImageFormat::PNG, "PNG" },
	{ ImageFormat::TIFF, "TIFF" },
	/*{ ImageFormat::PBM, "PBM" },
	{ ImageFormat::PGM, "PGM" },
	{ ImageFormat::PPM, "PPM" },
	{ ImageFormat::XBM, "XBM" },
	{ ImageFormat::XPM, "XPM" } */
};

struct ImageHDMetadata
{
	bool saveTextFile;
	bool ortho;

	std::string date;
	std::string imageRatioLabel;

	//orthoInfo
	double scaleInv;
	double dpi;
	glm::dvec2 orthoSize;

	bool progressBar;

};

#endif
