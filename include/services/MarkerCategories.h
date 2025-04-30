#ifndef MARKER_CATEGORIES_H
#define MARKER_CATEGORIES_H

#include "models/data/Marker.h"

#include <cstdint>
#include <unordered_map>

enum class MarkerCategory
{
    Tag,
    Epi,
    Danger_2,
    Danger_1,
    Security,
    Rescue,
    Max_Enum
};

struct MarkerCategoryDefinition
{
    scs::MarkerIcon firstIcon;
    uint32_t iconCount;
};

const static std::unordered_map<MarkerCategory, MarkerCategoryDefinition> markerCategoryDefinitions =
{
    { MarkerCategory::Tag, { scs::MarkerIcon::Tag_Base, 12} },
    { MarkerCategory::Epi, { scs::MarkerIcon::Picto_Epi_Ari, 12} },
    { MarkerCategory::Danger_2, { scs::MarkerIcon::Picto_Danger_Attention, 8} },
    { MarkerCategory::Danger_1, { scs::MarkerIcon::Picto_Danger_EX, 9} },
    { MarkerCategory::Security, { scs::MarkerIcon::Picto_Security_Alarm, 4} },
    { MarkerCategory::Rescue, { scs::MarkerIcon::Picto_Rescue_Ari, 9} }
};

#endif