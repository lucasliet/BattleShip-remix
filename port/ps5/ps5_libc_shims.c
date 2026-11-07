/*
 * libc functions the PS5 title runtime (the boilerplate's sce_module/libc.prx) does not export but
 * spdlog, libzip and libultraship's FileDropMgr call. The console clock is used as UTC: these
 * times only stamp log lines and zip entries.
 */
#ifdef __PS5__

#include <dlfcn.h>
#include <stdint.h>
#include <time.h>

static const int64_t kSecondsPerDay = 86400;

/**
 * Converts days since 1970-01-01 to a civil date (Howard Hinnant's days_from_civil inverse).
 * @param days days since the Unix epoch
 * @param year receives the year
 * @param month receives the month, 1..12
 * @param day receives the day of the month, 1..31
 */
static void civil_from_days(int64_t days, int64_t *year, int *month, int *day) {
    days += 719468;
    const int64_t era = (days >= 0 ? days : days - 146096) / 146097;
    const int64_t day_of_era = days - era * 146097;
    const int64_t year_of_era = (day_of_era - day_of_era / 1460 + day_of_era / 36524 - day_of_era / 146096) / 365;
    const int64_t day_of_year = day_of_era - (365 * year_of_era + year_of_era / 4 - year_of_era / 100);
    const int64_t month_index = (5 * day_of_year + 2) / 153;
    *day = (int)(day_of_year - (153 * month_index + 2) / 5 + 1);
    *month = (int)(month_index < 10 ? month_index + 3 : month_index - 9);
    *year = year_of_era + era * 400 + (*month <= 2);
}

/**
 * Days elapsed in a year before the first day of a month.
 * @param month the month, 1..12
 * @param leap non-zero for a leap year
 * @return the day of the year, 0-based, of the month's first day
 */
static int days_before_month(int month, int leap) {
    static const int kCumulative[12] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
    return kCumulative[month - 1] + (leap && month > 2 ? 1 : 0);
}

struct tm *gmtime_r(const time_t *clock, struct tm *result) {
    int64_t seconds = (int64_t)*clock;
    int64_t days = seconds / kSecondsPerDay;
    int64_t remainder = seconds % kSecondsPerDay;
    if (remainder < 0) {
        remainder += kSecondsPerDay;
        days -= 1;
    }
    int64_t year;
    int month;
    int day;
    civil_from_days(days, &year, &month, &day);
    const int leap = (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;

    result->tm_sec = (int)(remainder % 60);
    result->tm_min = (int)(remainder / 60 % 60);
    result->tm_hour = (int)(remainder / 3600);
    result->tm_mday = day;
    result->tm_mon = month - 1;
    result->tm_year = (int)(year - 1900);
    result->tm_wday = (int)((days % 7 + 11) % 7);
    result->tm_yday = days_before_month(month, leap) + day - 1;
    result->tm_isdst = 0;
    return result;
}

struct tm *localtime_r(const time_t *clock, struct tm *result) {
    return gmtime_r(clock, result);
}

int dladdr(const void *address, Dl_info *info) {
    (void)address;
    (void)info;
    return 0;
}

#endif
