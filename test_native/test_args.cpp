#include "args.h"
#include "test.h"

#define CHECK(expr) if (!(expr)) return false

static bool test_empty()
{
    Args a("");
    CHECK(a.argc() == 0);
    return true;
}

static bool test_whitespace_only()
{
    Args a("   \t  ");
    CHECK(a.argc() == 0);
    return true;
}

static bool test_single_char()
{
    Args a("L");
    CHECK(a.argc() == 1);
    CHECK(a[0].t == Args::Type::CHAR);
    CHECK(a[0].c == 'L');
    return true;
}

static bool test_single_int()
{
    Args a("3");
    CHECK(a.argc() == 1);
    CHECK(a[0].t == Args::Type::INT);
    CHECK(a[0].i == 3);
    return true;
}

static bool test_negative_int()
{
    Args a("-5");
    CHECK(a.argc() == 1);
    CHECK(a[0].t == Args::Type::INT);
    CHECK(a[0].i == -5);
    return true;
}

static bool test_positive_sign()
{
    Args a("+7");
    CHECK(a.argc() == 1);
    CHECK(a[0].t == Args::Type::INT);
    CHECK(a[0].i == 7);
    return true;
}

static bool test_multidigit_int()
{
    Args a("123");
    CHECK(a.argc() == 1);
    CHECK(a[0].t == Args::Type::INT);
    CHECK(a[0].i == 123);
    return true;
}

static bool test_hex_int()
{
    Args a("0x1F");
    CHECK(a.argc() == 1);
    CHECK(a[0].t == Args::Type::INT);
    CHECK(a[0].i == 31);
    return true;
}

static bool test_multiple_chars()
{
    // Multiple chars in a row become multiple CHAR args
    Args a("ABC");
    CHECK(a.argc() == 3);
    CHECK(a[0].t == Args::Type::CHAR);
    CHECK(a[0].c == 'A');
    CHECK(a[1].t == Args::Type::CHAR);
    CHECK(a[1].c == 'B');
    CHECK(a[2].t == Args::Type::CHAR);
    CHECK(a[2].c == 'C');
    return true;
}

static bool test_char_int_space()
{
    // "L 3" and "L3" are both two arguments per the spec
    Args a("L 3");
    CHECK(a.argc() == 2);
    CHECK(a[0].t == Args::Type::CHAR);
    CHECK(a[0].c == 'L');
    CHECK(a[1].t == Args::Type::INT);
    CHECK(a[1].i == 3);
    return true;
}

static bool test_char_int_nospace()
{
    // "L3" — boundary is clear, so no whitespace needed
    Args a("L3");
    CHECK(a.argc() == 2);
    CHECK(a[0].t == Args::Type::CHAR);
    CHECK(a[0].c == 'L');
    CHECK(a[1].t == Args::Type::INT);
    CHECK(a[1].i == 3);
    return true;
}

static bool test_char_int_char_nospace()
{
    // "L3C" — three args, no spaces needed
    Args a("L3C");
    CHECK(a.argc() == 3);
    CHECK(a[0].t == Args::Type::CHAR);
    CHECK(a[0].c == 'L');
    CHECK(a[1].t == Args::Type::INT);
    CHECK(a[1].i == 3);
    CHECK(a[2].t == Args::Type::CHAR);
    CHECK(a[2].c == 'C');
    return true;
}

static bool test_char_int_char_spaces()
{
    // "L 3 C" — same result as "L3C"
    Args a("L 3 C");
    CHECK(a.argc() == 3);
    CHECK(a[0].t == Args::Type::CHAR);
    CHECK(a[0].c == 'L');
    CHECK(a[1].t == Args::Type::INT);
    CHECK(a[1].i == 3);
    CHECK(a[2].t == Args::Type::CHAR);
    CHECK(a[2].c == 'C');
    return true;
}

static bool test_int_then_char_nospace()
{
    // "-5C" — int boundary is clear at alpha
    Args a("-5C");
    CHECK(a.argc() == 2);
    CHECK(a[0].t == Args::Type::INT);
    CHECK(a[0].i == -5);
    CHECK(a[1].t == Args::Type::CHAR);
    CHECK(a[1].c == 'C');
    return true;
}

static bool test_char_multidigit_int()
{
    Args a("L123");
    CHECK(a.argc() == 2);
    CHECK(a[0].t == Args::Type::CHAR);
    CHECK(a[0].c == 'L');
    CHECK(a[1].t == Args::Type::INT);
    CHECK(a[1].i == 123);
    return true;
}

static bool test_leading_trailing_whitespace()
{
    Args a("  L  3  ");
    CHECK(a.argc() == 2);
    CHECK(a[0].t == Args::Type::CHAR);
    CHECK(a[0].c == 'L');
    CHECK(a[1].t == Args::Type::INT);
    CHECK(a[1].i == 3);
    return true;
}

static bool test_unrecognized_stops_mid()
{
    // Unrecognized token stops parsing; prior args are kept
    Args a("L.3");
    CHECK(a.argc() == 1);
    CHECK(a[0].t == Args::Type::CHAR);
    CHECK(a[0].c == 'L');
    return true;
}

static bool test_unrecognized_at_start()
{
    // Unrecognized token at start: nothing parsed
    Args a(".L3");
    CHECK(a.argc() == 0);
    return true;
}

extern const Test tests_args[] = {
    {"empty",                    test_empty},
    {"whitespace_only",          test_whitespace_only},
    {"single_char",              test_single_char},
    {"single_int",               test_single_int},
    {"negative_int",             test_negative_int},
    {"positive_sign",            test_positive_sign},
    {"multidigit_int",           test_multidigit_int},
    {"hex_int",                  test_hex_int},
    {"multiple_chars",           test_multiple_chars},
    {"char_int_space",           test_char_int_space},
    {"char_int_nospace",         test_char_int_nospace},
    {"char_int_char_nospace",    test_char_int_char_nospace},
    {"char_int_char_spaces",     test_char_int_char_spaces},
    {"int_then_char_nospace",    test_int_then_char_nospace},
    {"char_multidigit_int",      test_char_multidigit_int},
    {"leading_trailing_ws",      test_leading_trailing_whitespace},
    {"unrecognized_stops_mid",   test_unrecognized_stops_mid},
    {"unrecognized_at_start",    test_unrecognized_at_start},
};

extern const int tests_args_cnt = sizeof(tests_args) / sizeof(tests_args[0]);
