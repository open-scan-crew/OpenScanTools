#include "tls_core.h"

#include "tls_impl.h"

//#include "utils.h"


tls::ImageFile::ImageFile(const std::filesystem::path& filepath, tls::usage_options options)
{
    p_ = std::make_shared<ImageFile_p>(filepath, options);

    if (!p_->is_valid_file())
        p_.reset();
}

uint32_t tls::ImageFile::getScanCount() const
{
    if (p_.get() == nullptr)
        return 0u;

    return p_->getScanCount();
}

uint64_t tls::ImageFile::getTotalPoints() const
{
    if (p_.get() == nullptr)
        return 0ull;

    return p_->getPointCount();
}

tls::FileHeader tls::ImageFile::getFileHeader() const
{
    if (p_.get() == nullptr)
        return tls::FileHeader{};
    
    return p_->file_header_;
}

tls::ScanHeader tls::ImageFile::getScanHeader(uint32_t n) const
{
    if (p_.get() == nullptr || n >= p_->pc_headers_.size())
        return tls::ScanHeader{};

    return p_->pc_headers_[n];
}

bool tls::ImageFile::setCurrentPointCloud(uint32_t n)
{
    if (p_.get() == nullptr)
        return false;

    return  p_->setCurrentPointCloud(n);
}

bool tls::ImageFile::appendPointCloud(const tls::ScanHeader& info, const Transformation& transfo)
{
    if (p_.get() == nullptr)
        return false;

    return p_->appendPointCloud(info, transfo);
}

bool tls::ImageFile::finalizePointCloud(uint32_t n)
{
    if (p_.get() == nullptr)
        return false;

    return p_->finalizePointCloud(n);
}

bool tls::ImageFile::readPoints(PointXYZIRGB* dstBuf, uint64_t bufSize, uint64_t& readCount)
{
    if (p_.get() == nullptr)
        return false;

    return p_->readPoints(dstBuf, bufSize, readCount);
}

bool tls::ImageFile::addPoints(PointXYZIRGB const* srcBuf, uint64_t srcSize)
{
    if (p_.get() == nullptr)
        return false;

    return p_->addPoints(srcBuf, srcSize);
}

bool tls::ImageFile::mergePoints(PointXYZIRGB const* srcBuf, uint64_t srcSize, const Transformation& srcTransfo, tls::PointFormat srcFormat)
{
    if (p_.get() == nullptr)
        return false;

    return p_->mergePoints(srcBuf, srcSize, srcTransfo, srcFormat);
}
