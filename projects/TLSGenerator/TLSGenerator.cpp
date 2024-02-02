#pragma once
#include <iostream>
#include <string>
#include "pointCloudEngine/OctreeCtor.h"
#include "pointCloudEngine/OctreeDecoder.h"
#include "io/exports/E57FileWriter.h"
#include "io/exports/TlsFileWriter.h"
#include "models/pointCloud/PointXYZIRGB.h"

constexpr uint64_t POINTS_PER_READ = 2 * 1048576;

void insertGrid(OctreeCtor& octree,float orgin[3], float step[3], int iteration[3])
{
	for (int xIt(0); xIt < iteration[0]; xIt++)
		for (int yIt(0); yIt < iteration[1]; yIt++)
			for (int zIt(0); zIt < iteration[2]; zIt++)
				octree.insertPoint({xIt*step[0] - orgin[0], yIt*step[1] - orgin[1], zIt*step[2] - orgin[2], 0, (uint8_t)xIt, (uint8_t)yIt, (uint8_t)zIt });
}

int main(int argc, char* argv[])
{
	std::wstring log;
	OctreeCtor octree1(tls::PrecisionType::TL_OCTREE_100UM, tls::PointFormat::TL_POINT_XYZ_I), octree2(tls::PrecisionType::TL_OCTREE_100UM, tls::PointFormat::TL_POINT_XYZ_I);
	float orgin[3] = {0.0f,0.0f,0.0f};
	float step[3] = {0.01f,0.1f,0.05f};
	int iteration[3] = {50,100,20};
	insertGrid(octree1, orgin, step, iteration);
	insertGrid(octree2, orgin, step, iteration);
	tls::ScanHeader header;
	header.format = tls::PointFormat::TL_POINT_XYZ_RGB;
	header.transfo.translation[0] = 0;
	header.transfo.translation[1] = 0;
	header.transfo.translation[2] = 0;
	header.transfo.quaternion[0] = 0;
	header.transfo.quaternion[1] = 0;
	header.transfo.quaternion[2] = 0;
	header.transfo.quaternion[0] = 1;

	IScanFileWriter* scanFileWriter(nullptr);
	// Create a Writer per scan or per project
	if (getScanFileWriter("./",L"testScans.tls", FileType::TLS, log, &scanFileWriter) == false)
		return 0;

	OctreeDecoder decode(octree1);
	for (uint32_t iterator(0); iterator < decode.getCellCount(); iterator++)
	{
		uint64_t size(0);
		const PointXYZIRGB*pointsBuf = decode.getCellPoints(iterator, size);
		scanFileWriter->addPoints(pointsBuf, size);
	}

	scanFileWriter->appendPointCloud(header);

	scanFileWriter->flushWrite();
	delete scanFileWriter;
    std::cout << "Hello World!\n";
	return 0;
}
