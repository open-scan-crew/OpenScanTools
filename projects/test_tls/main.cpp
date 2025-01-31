#include "tls_core.h"

#include "PointSamples.h"

#include <iostream>
#include <chrono>

std::string file_name = "test_sample.tls";

void write_sample()
{
    tls::ImageFile write_tls(file_name, tls::usage::write);

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

    // Add the points stored
    std::cout << "Writing " << some_points.size() << " points." << std::endl;
    write_tls.addPoints(some_points.data(), some_points.size());
    write_tls.finalizePointCloud();
    write_tls.close();
}

void read_sample()
{
    tls::ImageFile read_tls;
    // The path can be specified after the object declaration.
    read_tls.open(file_name, tls::usage::read);

    // The buffer to reveive the points must be allocated by the caller.
    // If the buffer is not large enought for the file, then the file will be read
    //   in multiple sections.
    std::vector<tls::Point> dst_buffer;
    dst_buffer.resize(8);
    size_t read_count = 0;

    if (!read_tls.is_valid_file())
        return;
    
    std::cout << "Reading " << read_tls.getTotalPoints() << " points." << std::endl;

    read_tls.setCurrentPointCloud(0);
    // The points are read and decoded in the specified buffer.
    // The function returns true until there is no more points to read.
    while (read_tls.readNextPoints(dst_buffer.data(), dst_buffer.size(), read_count))
    {
        for (int i = 0; i < read_count; ++i)
        {
            tls::Point& pt = dst_buffer[i];
            std::cout << pt.x << ", " << pt.y << ", " << pt.z << ", " << (int)pt.i << std::endl;
        }
        // Reseting the memory is optional, it is here for testing purposes.
        memset(dst_buffer.data(), 0, dst_buffer.size() * sizeof(tls::Point));
    }
}

void read_file_one_chunk(const std::filesystem::path& path)
{
    tls::ImageFile read_tls(path, tls::usage::read);

    // The buffer to reveive the points must be allocated by the caller.
    // If the buffer is not large enought for the file, then the file will be read
    //   in multiple sections.
    std::vector<tls::Point> dst_buffer;
    dst_buffer.resize(64 * 1024);
    size_t read_count = 0;
    size_t show_filter = 64 * 1024;

    if (!read_tls.is_valid_file())
        return;

    std::cout << "Reading " << read_tls.getTotalPoints() << " points." << std::endl;
    std::chrono::steady_clock::time_point tp_0 = std::chrono::steady_clock::now();

    read_tls.setCurrentPointCloud(0);
    //tls::ImagePointCloud impc = read_tls.getImagePointCloud(0);
    std::chrono::steady_clock::time_point tp_1 = std::chrono::steady_clock::now();
    // The points are read and decoded in the specified buffer.
    // The function returns true until there is no more points to read.
    while (read_tls.readNextPoints(dst_buffer.data(), dst_buffer.size(), read_count))
    //while (impc.readNextPoints(dst_buffer.data(), dst_buffer.size(), read_count))
    {
    //    for (size_t i = 0; i < read_count; i += show_filter)
    //    {
    //        tls::Point& pt = dst_buffer[i];
    //        std::cout << pt.x << ", " << pt.y << ", " << pt.z << ", " << (int)pt.i << std::endl;
    //    }
    }
    std::chrono::steady_clock::time_point tp_2 = std::chrono::steady_clock::now();
    float d01 = std::chrono::duration<float, std::milli>(tp_1 - tp_0).count();
    float d12 = std::chrono::duration<float, std::milli>(tp_2 - tp_1).count();
    float d02 = std::chrono::duration<float, std::milli>(tp_2 - tp_0).count();
    std::cout << "Initial read: " << read_tls.getTotalPoints() / d01 << " pts/ms" << std::endl;
    std::cout << "Decoding    : " << read_tls.getTotalPoints() / d12 << " pts/ms" << std::endl;
    std::cout << "Total read  : " << read_tls.getTotalPoints() / d02 << " pts/ms" << std::endl;

}

void read_file_low_mem(const std::filesystem::path& path)
{
    tls::ImageFile read_tls(path, tls::usage::read);

    // The buffer to reveive the points must be allocated by the caller.
    // If the buffer is not large enought for the file, then the file will be read
    //   in multiple sections.
    std::vector<tls::Point> dst_buffer;
    dst_buffer.resize(64 * 1024);
    size_t read_count = 0;
    size_t show_filter = 64 * 1024;

    if (!read_tls.is_valid_file())
        return;

    std::cout << "Reading " << read_tls.getTotalPoints() << " points." << std::endl;
    std::chrono::steady_clock::time_point tp_0 = std::chrono::steady_clock::now();

    tls::ImagePointCloud impc = read_tls.getImagePointCloud(0);
    std::chrono::steady_clock::time_point tp_1 = std::chrono::steady_clock::now();
    // The points are read and decoded in the specified buffer.
    // The function returns true until there is no more points to read.
    while (impc.readNextPoints(dst_buffer.data(), dst_buffer.size(), read_count))
    {
    }
    std::chrono::steady_clock::time_point tp_2 = std::chrono::steady_clock::now();
    float d01 = std::chrono::duration<float, std::milli>(tp_1 - tp_0).count();
    float d12 = std::chrono::duration<float, std::milli>(tp_2 - tp_1).count();
    float d02 = std::chrono::duration<float, std::milli>(tp_2 - tp_0).count();
    std::cout << "Initial read: " << read_tls.getTotalPoints() / d01 << " pts/ms" << std::endl;
    std::cout << "Decoding    : " << read_tls.getTotalPoints() / d12 << " pts/ms" << std::endl;
    std::cout << "Total read  : " << read_tls.getTotalPoints() / d02 << " pts/ms" << std::endl;

}

int main(int argc, char** argv)
{
    //write_sample();

    //read_sample();

    read_file_one_chunk("C:/Workspace/Point Clouds/Usine_Huot/Scans/Decolletage_002.tls");
    read_file_one_chunk("C:/Workspace/Point Clouds/Usine_Huot/Scans/Decolletage_003.tls");
    read_file_one_chunk("C:/Workspace/Point Clouds/Usine_Huot/Scans/Decolletage_004.tls");

    std::cout << "\nType anything to end the program..." << std::endl;
    int a;
    std::cin >> a;
    return 0;
}