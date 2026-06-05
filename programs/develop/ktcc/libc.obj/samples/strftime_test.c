#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>


#define ASSERT_STRFTIME_EQ(test_name, expected, actual)    \
    do {                                                   \
        if (strcmp(expected, actual) == 0) {               \
            printf("[PASS] %s\n", test_name);              \
        } else {                                           \
            printf("[FAIL] %s\n", test_name);              \
            printf("       Expected: \"%s\"\n", expected); \
            printf("       Got:      \"%s\"\n", actual);   \
        }                                                  \
    } while (0)

void run_test(const char* test_name, const struct tm* timeptr, const char* format, const char* expected)
{
    char buffer[256];
    size_t maxsize = sizeof(buffer);

    // Выполнение strftime
    size_t actual_len = strftime(buffer, maxsize, format, timeptr);

    if (actual_len > 0 || (actual_len == 0 && expected[0] == '\0')) {
        ASSERT_STRFTIME_EQ(test_name, expected, buffer);
    } else {
        printf("[FAIL] %s (Error: strftime returned 0, but expected \"%s\")\n", test_name, expected);
    }
}

struct TestData {
    struct tm* t;
    const char* format;
    const char* expected;
};

//  04.06.2026, 23:52:24
struct tm a = {
    .tm_sec = 24,
    .tm_min = 52,
    .tm_hour = 23,
    .tm_mday = 4,
    .tm_mon = 5,
    .tm_year = 2026 - 1900,
    .tm_wday = 4,
    .tm_yday = 154,
    .tm_isdst = -1
};

//  29.02.2024, 12:00:00
struct tm date_leap = {
    .tm_year = 2024 - 1900,
    .tm_mon = 1,
    .tm_mday = 29,
    .tm_hour = 12,
    .tm_min = 0,
    .tm_sec = 0,
    .tm_wday = 4,
    .tm_yday = 59,
    .tm_isdst = -1
};

// 31.12.1999, 23:59:59
struct tm date_end_99 = {
    .tm_year = 1999 - 1900,
    .tm_mon = 11,
    .tm_mday = 31,
    .tm_hour = 23,
    .tm_min = 59,
    .tm_sec = 59,
    .tm_wday = 5,
    .tm_yday = 364,
    .tm_isdst = -1
};

// 01.01.2000, 00:00:00
struct tm date_start_00 = {
    .tm_year = 2000 - 1900,
    .tm_mon = 0,
    .tm_mday = 1,
    .tm_hour = 0,
    .tm_min = 0,
    .tm_sec = 0,
    .tm_wday = 6,
    .tm_yday = 0,
    .tm_isdst = -1
};

// 12.04.1961, 09:07:00
struct tm date_gagarin = {
    .tm_year = 1961 - 1900,
    .tm_mon = 3,
    .tm_mday = 12,
    .tm_hour = 9,
    .tm_min = 7,
    .tm_sec = 0,
    .tm_wday = 3,
    .tm_yday = 101,
    .tm_isdst = -1
};

//  01.01.1970 00:00:00
struct tm date_epoch = {
    .tm_year = 1970 - 1900,
    .tm_mon = 0,
    .tm_mday = 1,
    .tm_hour = 0,
    .tm_min = 0,
    .tm_sec = 0,
    .tm_wday = 4,
    .tm_yday = 0,
    .tm_isdst = -1
};

// 04.06.2026, 12:00:00
struct tm date_noon = {
    .tm_year = 2026 - 1900,
    .tm_mon = 5,
    .tm_mday = 4,
    .tm_hour = 12,
    .tm_min = 0,
    .tm_sec = 0,
    .tm_wday = 4,
    .tm_yday = 154,
    .tm_isdst = -1
};

struct TestData tests[] = {
    { &a, "%Y", "2026" },
    { &a, "%y", "26" },
    { &a, "%m", "06" },
    { &a, "%d", "04" },
    { &a, "%e", " 4" },
    { &a, "%a", "Thu" },
    { &a, "%A", "Thursday" },
    { &a, "%w", "4" },
    { &a, "%b", "Jun" },
    { &a, "%B", "June" },
    { &a, "%h", "Jun" },
    { &a, "%H", "23" },
    { &a, "%I", "11" },
    { &a, "%p", "PM" },
    { &a, "%M", "52" },
    { &a, "%S", "24" },
    { &a, "%R", "23:52" },
    { &a, "%T", "23:52:24" },
    { &a, "%D", "06/04/26" },
    { &a, "%F", "2026-06-04" },
    { &a, "%%", "%" },
    { &a, "%j", "155" },

    { &date_leap, "%Y-%m-%d", "2024-02-29" },
    { &date_leap, "%j", "060" },
    { &date_leap, "%A", "Thursday" },

    { &date_end_99, "%x", "12/31/99" },
    { &date_end_99, "%j", "365" },
    { &date_end_99, "%H:%M:%S", "23:59:59" },

    { &date_start_00, "%Y-%m-%d", "2000-01-01" },
    { &date_start_00, "%A", "Saturday" },
    { &date_start_00, "%p", "AM" },

    { &date_gagarin, "%H:%M", "09:07" }, 
    { &date_gagarin, "%y", "61" },
    { &date_gagarin, "%A", "Wednesday" }, 
    { &date_gagarin, "%j", "102" },

    { &date_epoch, "%Y-%m-%d", "1970-01-01" },
    { &date_epoch, "%A", "Thursday" },
    { &date_epoch, "%j", "001" },

    { &date_noon, "%I:%M %p", "12:00 PM" },     // 12:00 дня — это PM
    { &date_start_00, "%I:%M %p", "12:00 AM" }, // 00:00 ночи — это AM

    { &a, "%% %%", "% %" },

    // Composite specifiers
    { &a, "%c", "Thu Jun  4 23:52:24 2026" },
    { &a, "%r", "11:52:24 PM" },

    // ISO 8601 week number and week-based year
    { &a, "%V", "23" },
    { &a, "%G", "2026" },
    { &date_start_00, "%V", "52" }, // 2000-01-01 belongs to ISO week 1999-W52
    { &date_start_00, "%G", "1999" },

    // Time zones are disabled in this build, so %z/%Z are emitted literally
    { &a, "%z", "%z" },
    { &a, "%Z", "%Z" },
    // %s (seconds since the Epoch) is disabled as well
    { &a, "%s", "%s" },
};

void run_tests()
{
    // NOTE: tm_wday/tm_yday are set explicitly in every test struct above,
    // because this libc's mktime() neither normalizes the struct nor fills
    // those fields. strftime reads them as given, so no mktime() call here.
    size_t num_tests = sizeof(tests) / sizeof(tests[0]);
    for (size_t i = 0; i < num_tests; i++) {
        char test_label[256];
        snprintf(test_label, sizeof(test_label), "Test #%u (format: %s)", i + 1, tests[i].format);
        run_test(test_label, tests[i].t, tests[i].format, tests[i].expected);
    }
}

// strftime must return 0 (and not overflow) when the result does not fit.
void run_truncation_test()
{
    char buffer[5];
    // "2026-06-04" needs 11 bytes but only 5 are available
    size_t r = strftime(buffer, sizeof(buffer), "%Y-%m-%d", &a);
    if (r == 0)
        printf("[PASS] truncation returns 0\n");
    else
        printf("[FAIL] truncation: expected 0, got %u (\"%s\")\n", (unsigned)r, buffer);
}

int main(int argc, char** argv)
{
    run_tests();
    run_truncation_test();

    return 0;
}
