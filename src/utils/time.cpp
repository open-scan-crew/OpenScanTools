#include "utils/time.h"


// Substract the number of seconds between january 1st 1970 and january 6th 1980
// between 1970 and 1979 there are 2 bissextile year (1972, 1976)
// days = 8 * 365 + 2 * 366 + 5
// seconds = days * 24 * 3600
// NOTE - This does not take into account the leap seconds which varies over time between GPS and UTC
const uint64_t secondsOffset = 315964800u;

uint64_t scs::time_gps_to_utc(uint64_t gpsDateTime)
{
    return (gpsDateTime + secondsOffset);
}

uint64_t scs::dtime_gps_to_utc(double gpsDateTime)
{
    uint64_t gpsSecond = (uint64_t)gpsDateTime;

    return (gpsSecond + secondsOffset);
}

double scs::time_utc_to_gps(uint64_t utcDateTime)
{
    return (double)(utcDateTime - secondsOffset);
}