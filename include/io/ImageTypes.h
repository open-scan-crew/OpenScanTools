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
	PNG16,
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
	{ ImageFormat::PNG16, "PNG16" },
	{ ImageFormat::TIFF, "TIFF" },
	/*{ ImageFormat::PBM, "PBM" },
	{ ImageFormat::PGM, "PGM" },
	{ ImageFormat::PPM, "PPM" },
	{ ImageFormat::XBM, "XBM" },
	{ ImageFormat::XPM, "XPM" } */
};

inline std::string getImageExtension(const ImageFormat format)
{
	switch (format)
	{
	case ImageFormat::PNG:
	case ImageFormat::PNG16:
		return "PNG";
	case ImageFormat::JPG:
	case ImageFormat::JPEG:
		return "JPG";
	case ImageFormat::TIFF:
		return "TIFF";
	case ImageFormat::BMP:
		return "BMP";
	default:
		return "PNG";
	}
}

struct ImageHDMetadata
{
	bool saveTextFile;
	bool ortho;
	bool includeAlpha = true;

	std::string date;
	std::string imageRatioLabel;

	//orthoInfo
	double scaleInv;
	double dpi;
	glm::dvec2 orthoSize;

	bool progressBar;

	enum class CameraOrientation
	{
		None,
		Vertical,
		Horizontal
	};

	CameraOrientation cameraOrientation = CameraOrientation::None;
	bool hasBottomZ = false;
	double bottomZ = 0.0;
	bool hasBottomLeftXY = false;
	glm::dvec2 bottomLeftXY = glm::dvec2(0.0);
	bool hasTopRightXY = false;
	glm::dvec2 topRightXY = glm::dvec2(0.0);

};

#endif
