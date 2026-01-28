
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include "argv.h"
#include "pico/stdio.h"
#include "pico/stdio_usb.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "str_ops.h"
#include "sys_led.h"

//////////////////////////////////////////////////////////////////////////////

int main()
{
    Argv argv;

    stdio_init_all();

    SysLed::init();
    SysLed::pattern(50, 950);

#if 1
    while (!stdio_usb_connected()) {
        tight_loop_contents();
        SysLed::loop();
    }
#endif

    sleep_ms(10);

    SysLed::off();

    printf("\n");
    printf("argv_test\n");
    sleep_ms(200);

    argv.verbosity(0);

    // after reset
    assert(argv.argc() == 0);

    // one arg
    argv.reset();
    assert(!argv.add_char('X'));
    assert(argv.check(1, "X", false));
    assert(argv.argc() == 0);

    assert(argv.add_char('\n'));
    assert(argv.check(2, "X" "\0", true));
    assert(argv.argc() == 1);

    assert(*argv[0] == 'X');
    assert(strlen(argv[0]) == 1);
    assert(*argv[1] == '\0');
    // one arg
    argv.reset();
    assert(!argv.add_char('X'));
    assert(!argv.add_char('Y'));
    assert(argv.check(2, "XY", false));
    assert(argv.argc() == 0);
    assert(argv.add_char('\n'));
    assert(argv.check(3, "XY" "\0", true));
    assert(argv.argc() == 1);
    assert(strlen(argv[0]) == 2);
    assert(strcmp(argv[0], "XY") == 0);
    assert(*argv[1] == '\0');

    // two args
    argv.reset();
    assert(!argv.add_str("XY ABC"));
    assert(argv.check(6, "XY" "\0" "ABC", false));
    assert(argv.add_char('\n'));
    assert(argv.check(7, "XY" "\0" "ABC" "\0", true));
    assert(argv.argc() == 2);
    assert(strlen(argv[0]) == 2);
    assert(strcmp(argv[0], "XY") == 0);
    assert(strlen(argv[1]) == 3);
    assert(strcmp(argv[1], "ABC") == 0);
    assert(*argv[2] == '\0');

    // one arg, leading whitespace
    argv.reset();
    assert(!argv.add_char('\n'));
    assert(argv.check(0, "", false));
    assert(!argv.add_char(' '));
    assert(argv.check(0, "", false));
    assert(argv.add_str("XY\n"));
    assert(argv.check(3, "XY" "\0", true));
    assert(argv.argc() == 1);
    assert(strlen(argv[0]) == 2);
    assert(strcmp(argv[0], "XY") == 0);
    assert(*argv[1] == '\0');

    // two args, multiple spaces
    argv.reset();
    assert(argv.add_str("XY \t ABC\r"));
    assert(argv.check(7, "XY" "\0" "ABC" "\0", true));
    assert(argv.argc() == 2);
    assert(strlen(argv[0]) == 2);
    assert(strcmp(argv[0], "XY") == 0);
    assert(strlen(argv[1]) == 3);
    assert(strcmp(argv[1], "ABC") == 0);
    assert(*argv[2] == '\0');

    if (argv.line_max() == 10) {

        // three args, fill line buffer
        argv.reset();
        assert(!argv.add_str("XY ABC 12"));
        assert(argv.check(9, "XY" "\0" "ABC" "\0" "12", false));
        assert(argv.add_char('\n'));
        assert(argv.check(10, "XY" "\0" "ABC" "\0" "12" "\0", true));
        assert(argv.argc() == 3);
        assert(strlen(argv[0]) == 2);
        assert(strcmp(argv[0], "XY") == 0);
        assert(strlen(argv[1]) == 3);
        assert(strcmp(argv[1], "ABC") == 0);
        assert(strlen(argv[2]) == 2);
        assert(strcmp(argv[2], "12") == 0);
        assert(*argv[3] == '\0');

        // three args, overflow line buffer
        argv.reset();
        assert(!argv.add_str("XY ABC 12"));
        assert(argv.check(9, "XY" "\0" "ABC" "\0" "12", false));
        assert(!argv.add_char('3')); // should cause reset
        assert(argv.check(0, "", false));

    } // if (argv.line_max() == 10)

    printf("\n");
    printf("ok\n");
    sleep_ms(200);

    return 0;
}