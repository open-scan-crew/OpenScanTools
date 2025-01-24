#ifndef TLS_TEXTS_HPP
#define TLS_TEXTS_HPP

#include <QObject>
#include "tls_def.h"

#define TEXT_TLS_PRECISION_1_MILLIMETER QObject::tr("1 millimeter")
#define TEXT_TLS_PRECISION_100_MICROMETER QObject::tr("100 micrometer")
#define TEXT_TLS_PRECISION_10_MICROMETER QObject::tr("10 micrometer")

const static std::unordered_map<tls::PrecisionType, QString> PrecisionTexts = {
    { tls::PrecisionType::TL_OCTREE_1MM, TEXT_TLS_PRECISION_1_MILLIMETER },
    { tls::PrecisionType::TL_OCTREE_100UM, TEXT_TLS_PRECISION_100_MICROMETER },
    { tls::PrecisionType::TL_OCTREE_10UM, TEXT_TLS_PRECISION_10_MICROMETER }
};

#endif