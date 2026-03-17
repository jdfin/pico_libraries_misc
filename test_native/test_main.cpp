#include <cstdio>

#include "test.h"

// Defined in test_args.cpp
extern const Test tests_args[];
extern const int tests_args_cnt;

static int run_suite(const char *suite_name, const Test *tests, int count)
{
    int fail = 0;
    for (int i = 0; i < count; i++) {
        bool ok = tests[i].func();
        printf("  %s %s\n", ok ? "PASS" : "FAIL", tests[i].name);
        if (!ok)
            fail++;
    }
    printf("%s: %d/%d passed\n\n", suite_name, count - fail, count);
    return fail;
}

int main()
{
    printf("=== Misc Native Tests ===\n\n");

    int fail = 0;
    fail += run_suite("args", tests_args, tests_args_cnt);

    printf("=== %s ===\n", fail == 0 ? "ALL PASSED" : "FAILURES");
    return fail == 0 ? 0 : 1;
}
