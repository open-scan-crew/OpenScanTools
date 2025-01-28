#include "tls_core.h"

#include "tls_impl.h"

using namespace tls;

ImageFile::ImageFile()
{}

ImageFile::ImageFile(const std::filesystem::path& _filepath, usage _u)
{
    p_ = std::make_shared<ImageFile_p>(_filepath, _u);

    if (!p_->is_valid_file())
        p_.reset();
}

ImageFile::~ImageFile()
{
    p_.reset();
}

bool ImageFile::open(const std::filesystem::path& _filepath, usage _u)
{
    p_ = std::make_shared<ImageFile_p>(_filepath, _u);
    return p_->is_valid_file();
}

void ImageFile::close()
{
    p_.reset();
}

bool ImageFile::is_valid_file() const
{
    return (p_.get() != nullptr) && p_->is_valid_file();
}

std::filesystem::path ImageFile::getPath() const
{
    if (p_.get() == nullptr)
        return std::filesystem::path();

    return p_->filepath_;
}

uint32_t ImageFile::getScanCount() const
{
    if (p_.get() == nullptr)
        return 0u;

    return p_->getScanCount();
}

uint64_t ImageFile::getTotalPoints() const
{
    if (p_.get() == nullptr)
        return 0ull;

    return p_->getPointCount();
}

tls::FileHeader ImageFile::getFileHeader() const
{
    if (p_.get() == nullptr)
        return tls::FileHeader{};
    
    return p_->file_header_;
}

tls::ScanHeader ImageFile::getPointCloudHeader(uint32_t _pc_num) const
{
    if (p_.get() == nullptr || _pc_num >= p_->pcs_.size())
        return tls::ScanHeader{};

    return p_->pcs_[_pc_num].infos_;
}

bool ImageFile::setCurrentPointCloud(uint32_t _pc_num)
{
    if (p_.get() == nullptr)
        return false;

    return  p_->setCurrentPointCloud(_pc_num);
}

bool ImageFile::appendPointCloud(const tls::ScanHeader& info)
{
    if (p_.get() == nullptr)
        return false;

    return p_->appendPointCloud(info);
}

bool ImageFile::finalizePointCloud(double add_x, double add_y, double add_z)
{
    if (p_.get() == nullptr)
        return false;

    return p_->finalizePointCloud(add_x, add_y, add_z);
}

bool ImageFile::readPoints(Point* dstBuf, uint64_t bufSize, uint64_t& readCount)
{
    if (p_.get() == nullptr)
        return false;

    return p_->readPoints(dstBuf, bufSize, readCount);
}

bool ImageFile::addPoints(Point const* srcBuf, uint64_t srcSize)
{
    if (p_.get() == nullptr)
        return false;

    return p_->addPoints(srcBuf, srcSize);
}

bool ImageFile::mergePoints(Point const* srcBuf, uint64_t srcSize, const Transformation& srcTransfo, tls::PointFormat srcFormat)
{
    if (p_.get() == nullptr)
        return false;

    return p_->mergePoints(srcBuf, srcSize, srcTransfo, srcFormat);
}

bool ImageFile::getOctreeBase(uint32_t _pc_num, OctreeBase& _octree_base)
{
    if (p_.get() == nullptr)
        return false;

    return p_->getOctreeBase(_pc_num, _octree_base);
}

bool ImageFile::getData(uint32_t _pc_num, uint64_t _file_pos, void* _data_buf, uint64_t _data_size)
{
    if (p_.get() == nullptr)
        return false;

    return p_->getData(_pc_num, _file_pos, _data_buf, _data_size);
}

bool ImageFile::getCellRenderData(uint32_t _pc_num, void* data_buf, uint64_t& data_size)
{
    if (p_.get() == nullptr)
        return false;

    return p_->getCellRenderData(_pc_num, data_buf, data_size);
}
