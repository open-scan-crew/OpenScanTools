#include "io/ImageWriter.h"
#include "gui/UnitConverter.h"
#include "utils/Logger.h"
#include "utils/Utils.h"
#include "utils/ColorConversion.h"
#include "vulkan/VulkanManager.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <limits>

#include <QtCore/qobject.h>
#include <QtGui/qimage.h>
#include <QtGui/qfontmetrics.h>
#include <QtGui/qpainter.h>

#include <fmt/format.h>

namespace
{
std::string formatDistance(double value, const UnitUsage& unitUsage)
{
    const std::string dist_unit = UnitConverter::getUnitText(unitUsage.distanceUnit).toStdString();
    const uint32_t digits = unitUsage.displayedDigits;
    const std::string format = fmt::format("{{:.{0}f}}{1}", digits, dist_unit);
    const double converted = UnitConverter::meterToX(value, unitUsage.distanceUnit);
    return fmt::format(fmt::runtime(format), converted);
}

std::string formatTemperature(double value, const UnitUsage& unitUsage)
{
    const uint32_t digits = unitUsage.displayedDigits;
    const std::string format = fmt::format("{{:.{0}f}}{{}}", digits);
    return fmt::format(fmt::runtime(format), value, UnitConverter::getTemperatureUnitText().toStdString());
}

std::vector<QColor> buildRampColors(int steps)
{
    std::vector<QColor> colors;
    if (steps <= 0)
        return colors;

    colors.reserve(static_cast<size_t>(steps));
    const int denom = std::max(1, steps - 1);
    for (int s = 0; s < steps; ++s)
    {
        const float q = static_cast<float>(s) / static_cast<float>(denom);
        const float hue = (q * 2.f) / 3.f;
        const glm::vec3 rgb = utils::color::hsl2rgb(glm::vec3(hue, 1.f, 0.5f));
        colors.emplace_back(static_cast<int>(rgb.x * 255.f),
                            static_cast<int>(rgb.y * 255.f),
                            static_cast<int>(rgb.z * 255.f),
                            255);
    }
    return colors;
}
} // namespace

ImageWriter::ImageWriter()
{}

ImageWriter::~ImageWriter()
{
    delete[] image_buffer_;
    image_buffer_ = nullptr;
    buffer_size_ = 0;
}

bool ImageWriter::saveScreenshot(const std::filesystem::path& filepath, ImageFormat format, ImageTransferEvent transfer, uint32_t width, uint32_t height, bool includeAlpha)
{
    width_ = width;
    height_ = height;
    format_ = format;
    byte_per_pixel_ = format == ImageFormat::PNG16 ? 8u : 4u;
    include_alpha_ = includeAlpha;

    if (!allocateBuffer())
        return false;

    VulkanManager::getInstance().doImageTransfer(transfer, width_, height_, image_buffer_, buffer_size_, 0u, 0u, 0u, format == ImageFormat::PNG16);
    saveImage(filepath);

    return true;
}

bool ImageWriter::startCapture(ImageFormat format, uint32_t width, uint32_t height, bool includeAlpha)
{
    width_ = width;
    height_ = height;
    format_ = format;
    byte_per_pixel_ = format == ImageFormat::PNG16 ? 8u : 4u;
    include_alpha_ = includeAlpha;
    return allocateBuffer();
}

void ImageWriter::transferImageTile(ImageTransferEvent transfer, uint32_t dstOffsetW, uint32_t dstOffsetH, uint32_t border)
{
    VulkanManager::getInstance().doImageTransfer(transfer, width_, height_, image_buffer_, buffer_size_, dstOffsetW, dstOffsetH, border, format_ == ImageFormat::PNG16);
}

void ImageWriter::writeTile(const void* tileBuffer, uint32_t tileW, uint32_t tileH, uint32_t dstOffsetW, uint32_t dstOffsetH)
{
    if (!tileBuffer || !image_buffer_ || tileW == 0 || tileH == 0)
        return;

    const size_t bytesPerPixel = byte_per_pixel_;
    const size_t dstRowStride = bytesPerPixel * width_;
    const size_t srcRowStride = bytesPerPixel * tileW;

    for (uint32_t h = 0; h < tileH; ++h)
    {
        const size_t dstOffset = bytesPerPixel * (dstOffsetW + static_cast<size_t>(dstOffsetH + h) * width_);
        const char* srcRow = static_cast<const char*>(tileBuffer) + srcRowStride * h;
        memcpy_s(image_buffer_ + dstOffset, buffer_size_ - dstOffset, srcRow, srcRowStride);
    }
}

void ImageWriter::applyOrthoGridOverlay(const ImageHDMetadata& metadata, const OrthoGridOverlay& overlay)
{
    if (!overlay.active || !metadata.ortho)
        return;
    if (width_ == 0 || height_ == 0 || overlay.step <= 0.f)
        return;
    if (metadata.orthoSize.x <= 0.0 || metadata.orthoSize.y <= 0.0)
        return;

    QImage qImage;
    if (format_ == ImageFormat::PNG16)
        qImage = QImage(reinterpret_cast<uchar*>(image_buffer_), width_, height_, QImage::Format::Format_RGBA64);
    else
        qImage = QImage(reinterpret_cast<uchar*>(image_buffer_), width_, height_, QImage::Format::Format_ARGB32);
    if (qImage.isNull())
        return;

    QPainter painter(&qImage);
    painter.setRenderHint(QPainter::Antialiasing, false);
    QPen pen(QColor(overlay.color.r, overlay.color.g, overlay.color.b, overlay.color.a));
    pen.setWidthF(static_cast<qreal>(overlay.lineWidth));
    painter.setPen(pen);

    const float gridW = static_cast<float>(overlay.step * static_cast<double>(width_) / metadata.orthoSize.x);
    const float gridH = static_cast<float>(overlay.step * static_cast<double>(height_) / metadata.orthoSize.y);
    const float limitGridW = 5.f;
    const float totalW = static_cast<float>(width_);
    const float totalH = static_cast<float>(height_);

    if (gridW > limitGridW && gridH > 0.f)
    {
        const float numLineH = std::trunc(totalH / gridH);
        const float decalH = totalH - gridH * numLineH;

        for (float w = 0.f; w < totalW; w += gridW)
        {
            const float x = std::roundf(w);
            painter.drawLine(QPointF(x, 0.f), QPointF(x, totalH));
        }

        for (float h = 0.f; h < totalH; h += gridH)
        {
            const float y = std::roundf(h + decalH);
            painter.drawLine(QPointF(0.f, y), QPointF(totalW, y));
        }
    }

    const QString unitText = UnitConverter::getUnitText(overlay.distanceUnit);
    const double gridSizeUI = UnitConverter::meterToX(overlay.step, overlay.distanceUnit);
    const QString label = QObject::tr("Grid cell size : %1 %2").arg(gridSizeUI).arg(unitText);

    QFont font = painter.font();
    font.setPixelSize(12);
    painter.setFont(font);
    painter.setPen(QColor(overlay.color.r, overlay.color.g, overlay.color.b, overlay.color.a));

    const QFontMetrics metrics(font);
    const int textWidth = metrics.horizontalAdvance(label);
    const int textHeight = metrics.height();
    const int margin = 4;
    const QRect textRect(margin, static_cast<int>(height_) - textHeight - margin, textWidth, textHeight);
    painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, label);
}

void ImageWriter::applyRampScaleOverlay(const RampScaleOverlay& overlay)
{
    if (!overlay.active)
        return;
    if (width_ == 0 || height_ == 0)
        return;
    if (overlay.steps <= 0 || overlay.graduation < 0)
        return;

    QImage qImage;
    if (format_ == ImageFormat::PNG16)
        qImage = QImage(reinterpret_cast<uchar*>(image_buffer_), width_, height_, QImage::Format::Format_RGBA64);
    else
        qImage = QImage(reinterpret_cast<uchar*>(image_buffer_), width_, height_, QImage::Format::Format_ARGB32);
    if (qImage.isNull())
        return;

    const float marginY = static_cast<float>(height_) * 0.05f;
    const float marginRight = static_cast<float>(width_) * 0.02f;
    const float scaleHeight = static_cast<float>(height_) - marginY * 2.f;
    if (scaleHeight <= 0.f)
        return;

    const float internMarginX = std::max(4.f, static_cast<float>(width_) * 0.005f);
    const float internMarginY = std::max(4.f, static_cast<float>(height_) * 0.01f);
    const float scaleWidth = std::max(6.f, static_cast<float>(width_) * 0.015f);
    const float blank = std::max(2.f, static_cast<float>(width_) * 0.0025f);
    const float bigDashWidth = scaleWidth * 0.4f;
    const float smallDashWidth = scaleWidth * 0.24f;
    const float bigDashHeight = std::max(2.f, static_cast<float>(height_) * 0.002f);
    const float smallDashHeight = std::max(1.f, static_cast<float>(height_) * 0.001f);
    const int graduation = std::max(1, overlay.graduation);

    QPainter painter(&qImage);
    painter.setRenderHint(QPainter::Antialiasing, false);

    QFont font = painter.font();
    const float fontSize = std::max(5.f, static_cast<float>(height_) * 0.01f);
    font.setPixelSize(static_cast<int>(std::round(fontSize)));
    painter.setFont(font);
    const QFontMetrics metrics(font);

    const std::string minText = overlay.isTemperature
        ? formatTemperature(overlay.vmin, overlay.unitUsage)
        : formatDistance(overlay.vmin, overlay.unitUsage);
    const std::string maxText = overlay.isTemperature
        ? formatTemperature(overlay.vmax, overlay.unitUsage)
        : formatDistance(overlay.vmax, overlay.unitUsage);
    const int textWidth = std::max(metrics.horizontalAdvance(QString::fromStdString(minText)),
                                   metrics.horizontalAdvance(QString::fromStdString(maxText)));
    const float textHeight = static_cast<float>(metrics.height());

    const float totalWidth = internMarginX * 2.f + static_cast<float>(textWidth) + blank + bigDashWidth + scaleWidth;
    if (totalWidth + marginRight > static_cast<float>(width_))
        return;

    const float x = static_cast<float>(width_) - marginRight - totalWidth;
    const float y = marginY;
    const float internHeight = scaleHeight - internMarginY * 2.f;
    if (internHeight <= textHeight)
        return;

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(48, 48, 48, 192));
    painter.drawRoundedRect(QRectF(x, y, totalWidth, scaleHeight), 5.f, 5.f);

    painter.setPen(QColor(255, 255, 255));

    const float textX = x + internMarginX;
    const float bigDashX = textX + static_cast<float>(textWidth) + blank;
    const float smallDashX = bigDashX + (bigDashWidth - smallDashWidth);
    const float scaleX = bigDashX + bigDashWidth;

    const float dyGrad = (internHeight - textHeight) / static_cast<float>(graduation);
    float lastTextY = -std::numeric_limits<float>::infinity();
    for (int i = 0; i < graduation + 1; ++i)
    {
        const float textY = y + internMarginY + dyGrad * static_cast<float>(i);
        const float dashY = textY + textHeight / 2.f;
        if (textY < lastTextY + textHeight * 1.5f)
        {
            painter.fillRect(QRectF(smallDashX, dashY - smallDashHeight / 2.f, smallDashWidth, smallDashHeight),
                             QColor(255, 255, 255));
            continue;
        }

        std::string text;
        if (overlay.isTemperature)
        {
            int entryIndex = static_cast<int>(std::round(static_cast<double>(i) * (overlay.steps - 1) / graduation));
            entryIndex = std::clamp(entryIndex, 0, overlay.steps - 1);
            if (overlay.temperatureAscending)
                entryIndex = overlay.steps - 1 - entryIndex;
            const double value = overlay.temperatureEntries[static_cast<size_t>(entryIndex)].temperature;
            text = formatTemperature(value, overlay.unitUsage);
        }
        else
        {
            const double value = overlay.vmax + (overlay.vmin - overlay.vmax) * static_cast<double>(i) / graduation;
            text = formatDistance(value, overlay.unitUsage);
        }

        painter.drawText(QPointF(textX, textY + textHeight), QString::fromStdString(text));
        lastTextY = textY;
        painter.fillRect(QRectF(bigDashX, dashY - bigDashHeight / 2.f, bigDashWidth, bigDashHeight),
                         QColor(255, 255, 255));
    }

    const float dY = (internHeight - textHeight) / static_cast<float>(overlay.steps);
    if (dY <= 0.f)
        return;

    if (overlay.isTemperature)
    {
        for (int i = 0; i < overlay.steps; ++i)
        {
            const int entryIndex = overlay.temperatureAscending ? (overlay.steps - 1 - i) : i;
            const TemperatureScaleEntry& entry = overlay.temperatureEntries[static_cast<size_t>(entryIndex)];
            const QColor color(entry.r, entry.g, entry.b, 255);
            const float barY = y + internMarginY + textHeight / 2.f + dY * static_cast<float>(i);
            painter.fillRect(QRectF(scaleX, barY, scaleWidth, dY), color);
        }
    }
    else
    {
        const std::vector<QColor> colors = buildRampColors(overlay.steps);
        for (int i = 0; i < overlay.steps; ++i)
        {
            const float barY = y + internMarginY + textHeight / 2.f + dY * static_cast<float>(i);
            painter.fillRect(QRectF(scaleX, barY, scaleWidth, dY), colors[static_cast<size_t>(i)]);
        }
    }
}

bool ImageWriter::save(const std::filesystem::path& file_path, ImageHDMetadata metadata)
{
    bool result = true;
    result &= saveImage(file_path);
    result &= saveMetadata(file_path, metadata);
    //m_dataDispatcher.updateInformation(new GuiDataProcessingSplashScreenLogUpdate(TEXT_SCREENSHOT_TEXT_FAILED));
    return result;
}

bool ImageWriter::saveMetadata(const std::filesystem::path& file_path, ImageHDMetadata metadata)
{
    if (!metadata.ortho)
        return true;

    std::filesystem::path metadata_path = file_path;

    std::ofstream os = std::ofstream(metadata_path.replace_extension(".txt"));
    if (os.is_open() == false)
    {
        return false;
    }

    os << "Image resolution" << std::endl;
    os << width_ << " x " << height_ << std::endl;
    os << std::endl;

    os << "Image date" << std::endl;
    os << metadata.date << std::endl;
    os << std::endl;

    os << "Image ratio" << std::endl;
    os << metadata.imageRatioLabel << std::endl;
    os << std::endl;

    os << "Image dimension L x H (m)" << std::endl;
    os << Utils::roundFloat(metadata.orthoSize.x, 3) << " x " << Utils::roundFloat(metadata.orthoSize.y, 3) << std::endl;
    os << std::endl;

    os << "Image scale" << std::endl;
    os << "1/" << (long)round(metadata.scaleInv) << std::endl;
    os << std::endl;

    os << "Image DPI" << std::endl;
    os << (long)round(metadata.dpi) << std::endl;
    os << std::endl;

    if (metadata.hasBottomZ)
    {
        os << "Z coordinate (m) - image bottom" << std::endl;
        os << Utils::roundFloat(metadata.imageBottomZ, 3) << std::endl;
        if (std::abs(metadata.importScanTranslation.z) > std::numeric_limits<double>::epsilon())
        {
            os << "Z original coordinate (m) - image bottom" << std::endl;
            os << Utils::roundFloat(metadata.imageBottomZ - metadata.importScanTranslation.z, 3) << std::endl;
        }
        os << std::endl;
    }

    if (metadata.hasVerticalCorners)
    {
        os << "XY coordinates (m), image bottom left" << std::endl;
        os << Utils::roundFloat(metadata.imageBottomLeft.x, 3) << " " << Utils::roundFloat(metadata.imageBottomLeft.y, 3) << std::endl;
        os << "XY coordinates (m), image top right" << std::endl;
        os << Utils::roundFloat(metadata.imageTopRight.x, 3) << " " << Utils::roundFloat(metadata.imageTopRight.y, 3) << std::endl;
        if (std::abs(metadata.importScanTranslation.x) > std::numeric_limits<double>::epsilon()
            || std::abs(metadata.importScanTranslation.y) > std::numeric_limits<double>::epsilon())
        {
            os << "XY original coordinates (m), image bottom left" << std::endl;
            os << Utils::roundFloat(metadata.imageBottomLeft.x - metadata.importScanTranslation.x, 3) << " "
               << Utils::roundFloat(metadata.imageBottomLeft.y - metadata.importScanTranslation.y, 3) << std::endl;
            os << "XY original coordinates (m), image top right" << std::endl;
            os << Utils::roundFloat(metadata.imageTopRight.x - metadata.importScanTranslation.x, 3) << " "
               << Utils::roundFloat(metadata.imageTopRight.y - metadata.importScanTranslation.y, 3) << std::endl;
        }
        os << std::endl;
    }

    os.close();
    return true;
}

bool ImageWriter::saveImage(const std::filesystem::path& file_path)
{
    assert(image_buffer_);    
    QImage qImage;
    if (format_ == ImageFormat::PNG16)
        qImage = QImage(reinterpret_cast<uchar*>(image_buffer_), width_, height_, QImage::Format::Format_RGBA64);
    else
        qImage = QImage(reinterpret_cast<uchar*>(image_buffer_), width_, height_, QImage::Format::Format_ARGB32);
    if (include_alpha_ == false)
    {
        if (format_ == ImageFormat::PNG16 || format_ == ImageFormat::TIFF)
            qImage = qImage.convertToFormat(QImage::Format::Format_RGBX64);
        else
            qImage = qImage.convertToFormat(QImage::Format::Format_RGB32);
    }
    std::filesystem::path ext_path = file_path;
    ext_path.replace_extension("." + getImageExtension(format_));
    bool ret(qImage.save(QString::fromStdWString(ext_path.generic_wstring())));

    return ret;
}

bool ImageWriter::allocateBuffer()
{
    const size_t required_size = static_cast<size_t>(width_) * height_ * byte_per_pixel_;
    if (required_size == 0)
        return false;
    if (image_buffer_ != nullptr)
    {
        // Reuse the existing buffer if it is already large enough
        if (buffer_size_ >= required_size)
            return true;

        delete[] image_buffer_;
        image_buffer_ = nullptr;
        buffer_size_ = 0;
    }

    buffer_size_ = required_size;
    try
    {
        image_buffer_ = new char[buffer_size_];
    }
    catch (std::exception& e)
    {
        IOLOG << "Failed to allocate memory for the image buffer. Size needed =  " << buffer_size_ << "\n Exception details :" << e.what() << Logger::endl;
        buffer_size_ = 0;
        return false;
    }
    return true;
}
