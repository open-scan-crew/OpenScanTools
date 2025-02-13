#include "tls_core.h"

#include "PointSample.h"

#include <iostream>
#include <chrono>

void write_sample(const std::filesystem::path& path)
{
    tls::ImageFile write_tls(path, tls::usage::write);

    // We cannot rewrite an existing file
    if (!write_tls.is_valid_file())
        return;

    tls::ScanHeader header;
    // Guid will be automatically generated if not provided
    header.name = L"Random Point Cloud";
    header.sensorModel = L"undefined";
    header.sensorSerialNumber = L"undefined";
    header.transfo.translation[0] = 1.0;
    header.transfo.translation[1] = 2.0;
    header.transfo.translation[2] = 3.0;
    header.precision = tls::PrecisionType::TL_OCTREE_100UM;
    header.format = tls::PointFormat::TL_POINT_XYZ_I;

    write_tls.appendPointCloud(header);

    // Add the points stored
    std::vector<tls::Point> some_points = PointSample::Brick(glm::vec3(1.f, 2.f, 3.f), glm::vec3(0.f, 5.f, -1.f), 10.f);
    std::cout << "Writing " << some_points.size() << " points." << std::endl;
    write_tls.addPoints(some_points.data(), some_points.size());
    write_tls.finalizePointCloud();
    write_tls.close();
}

void rewrite_transfo(const std::filesystem::path& path)
{
    tls::ImageFile update_tls(path, tls::usage::update);

    assert(update_tls.is_valid_file());

    tls::Transformation transfo;
    transfo.translation[0] = 6.0;
    transfo.translation[1] = -2.0;
    transfo.translation[2] = 3.5;
    transfo.quaternion[0] = 0.5;
    transfo.quaternion[1] = 0.5;
    transfo.quaternion[2] = 0.5;
    transfo.quaternion[3] = 0.5;
    update_tls.overwriteTransformation(0, transfo);
}

void read_file_low_mem(const std::filesystem::path& path, uint32_t _sample_count)
{
    tls::ImageFile read_tls;
    if (!read_tls.open(path, tls::usage::read))
        return;

    assert(read_tls.is_valid_file());

    // The buffer to reveive the points must be allocated by the caller.
    // If the buffer is not large enought for the file, then the file will be read
    //   in multiple sections.
    std::vector<tls::Point> dst_buffer;
    dst_buffer.resize(64 * 1024);
    size_t read_count = 0;
    size_t total_read = 0;

    if (!read_tls.is_valid_file())
        return;

    size_t point_count = read_tls.getPointCount();
    size_t next_pt_cout = point_count / _sample_count;
    std::cout << "*** LOW MEM *** " << path.filename() << " [" << point_count << " points]" << std::endl;
    std::chrono::steady_clock::time_point tp_0 = std::chrono::steady_clock::now();

    tls::ImagePointCloud impc = read_tls.getImagePointCloud(0);
    // The points are read and decoded in the specified buffer.
    // The function returns true until there is no more points to read.
    while (impc.getNextPoints(dst_buffer.data(), dst_buffer.size(), read_count))
    {
        total_read += read_count;
        while (next_pt_cout < total_read)
        {
            tls::Point& pt = dst_buffer[read_count - (total_read - next_pt_cout)];
            std::cout << pt.x << ", " << pt.y << ", " << pt.z << ", " << (int)pt.i << std::endl;
            next_pt_cout += point_count / _sample_count;
        }
    }
    std::chrono::steady_clock::time_point tp_1 = std::chrono::steady_clock::now();
    float d01 = std::chrono::duration<float, std::milli>(tp_1 - tp_0).count();
    std::cout << "Speed     (pts/ms): " << total_read / d01 << std::endl;
    std::cout << "Total read        : " << total_read << std::endl;
}

#include <map>

std::pair<uint32_t, uint32_t> flip_pair(const std::pair<uint32_t, uint32_t>& p)
{
    return std::pair<uint32_t, uint32_t>(p.second, p.first);
}

void flip_map()
{
    std::vector<uint32_t> pixels = { 42, 42, 17, 113, 17, 42, 17, 42, 19, 42 };

    std::map<uint32_t, uint32_t> counter;
    for (uint32_t p : pixels)
        counter[p]++;

    std::multimap<uint32_t, uint32_t> dst;
    std::transform(counter.begin(), counter.end(), std::inserter(dst, dst.begin()), flip_pair);
}

#include <fstream>

int main(int argc, char** argv)
{
    {
        float b = powf(2.f, -13);
        float d = b / 256;
        float a = 8.f;
        float e = 2.0000033f;
        float f = 1.9999967f;
        float g = 2.000123f;
        float ab = a / b;
        float eb = e / b;
        float fb = f / b;
        float gb = g / b;
        float edb = (e - d) / b;
        float fdb = (f - d) / b;
        float gdb = (g - d) / b;

        uint16_t au = (uint16_t)(a / b); // 0 (65536 overflow)
        uint16_t eu = (uint16_t)(eb); // 16384
        uint16_t fu = (uint16_t)(fb); // 16383
        uint16_t gu = (uint16_t)(gb); // 16385
        uint16_t adu = (uint16_t)((a - d) / b);
        uint16_t edu = (uint16_t)(edb);
        uint16_t fdu = (uint16_t)(fdb);
        uint16_t gdu = (uint16_t)(gdb);
    }

    {
        float s = 8.f;
        float o = 8.f;
        float p = -8.f;
        float q = 0.f;

        float b = 15.999999f;
        float c = -1.4901161e-8f;
        float d = 7.9999995f;

        bool rc = c < p + s; // true
        bool rp = c - p < s; // false

        bool rd = d < q + s; // true
        bool rq = d - q < s; // true

        bool rb = b < o + s; // true
        bool ro = b - o < s; // true
        std::cout << std::endl;
    }

    //flip_map();

    write_sample("C:/Workspace/test/test_brick.tls");
    //rewrite_transfo("C:/Workspace/test/test_brick.tls");
    read_file_low_mem("C:/Workspace/test/test_brick.tls", 20u);

    // Files for test:
    // Decolletage : {3, 4}, {1, 7}, {29, 27}, {37, 38}

    //read_file_low_mem("C:/Workspace/Point Clouds/Usine_Huot/Scans/Decolletage_001.tls", 20u);
    //read_file_low_mem("C:/Workspace/Point Clouds/Usine_Huot/Scans/Decolletage_004.tls", 0.f);
    //read_file_low_mem("C:/Workspace/Point Clouds/Usine_Huot/Scans/Decolletage_037.tls");
    //read_file_low_mem("C:/Workspace/Point Clouds/Usine_Huot/Scans/Decolletage_038.tls");
    //read_file_low_mem("D:/Point Clouds/Decolletage_014.tls");
    //read_file_low_mem("D:/Point Clouds/Decolletage_013.tls");

    return 0;
}