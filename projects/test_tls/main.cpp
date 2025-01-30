#include "tls_core.h"

#include "PointSamples.h"

#include <iostream>

int main(int argc, char** argv)
{
    tls::ImageFile write_tls("a_point_cloud.tls", tls::usage::write);

    tls::ScanHeader header;
    // Guid will be automatically generated if not provided
    header.name = L"Random Point Cloud";
    header.sensorModel = L"undefined";
    header.sensorSerialNumber = L"undefined";
    header.transfo.translation[0] = 5.0;
    header.transfo.translation[1] = 0.0;
    header.transfo.translation[2] = 2.0;
    header.precision = tls::PrecisionType::TL_OCTREE_100UM;
    header.format = tls::PointFormat::TL_POINT_XYZ_I;

    write_tls.appendPointCloud(header);

    write_tls.addPoints(some_points.data(), some_points.size());
    write_tls.finalizePointCloud();
    write_tls.close();

    tls::ImageFile read_tls("a_point_cloud.tls", tls::usage::read);
    std::vector<tls::Point> dst_buffer;
    dst_buffer.resize(5);
    size_t read_count = 0;
    if (read_tls.is_valid_file())
    {
        //dst_buffer.resize(read_tls.getTotalPoints());
        read_tls.setCurrentPointCloud(0);
        while (read_tls.readNextPoints(dst_buffer.data(), dst_buffer.size(), read_count))
        {
            for (int i = 0; i < read_count; ++i)
            {
                tls::Point& pt = dst_buffer[i];
                std::cout << pt.x << ", " << pt.y << ", " << pt.z << ", " << (int)pt.i << std::endl;
            }
            dst_buffer.clear();
            dst_buffer.resize(5);
        }
    }

    int a;
    std::cin >> a;

    return 0;
}