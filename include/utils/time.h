#ifndef SCS_TIME_H
#define SCS_TIME_H

#include <cstdint>

#define DISPLAY_TIME_FORMAT "%a %d %b %Y %T"
#define DISPLAY_WIDE_TIME_FORMAT L"%a %d %b %Y %T"
#define SERIALIZE_TIME_FORMAT "%F_%Hh%Mm%Ss"
#define WIDE_SERIALIZE_TIME_FORMAT L"%F_%Hh%Mm%Ss"

namespace scs
{
    uint64_t time_gps_to_utc(uint64_t gpsDateTime);
    uint64_t dtime_gps_to_utc(double gpsDateTime);
    double time_utc_to_gps(uint64_t utcDateTime);
};

#endif // SCS_TIME_H