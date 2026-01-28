
#include <cstdint>
#include <cstdio>
//
#include "pico/stdio.h"
#include "pico/stdio_usb.h"
#include "pico/stdlib.h"
#include "pico/time.h"
//
#include "argv.h"
#include "sys_led.h"

// A command is a sequence of tokens, ending with newline.
//
// The "Argv" object takes a character at a time and builds a complete command
// (an array of tokens), which can then be examined for commands and parameters.



int main()
{
    stdio_init_all();

    SysLed::init();
    SysLed::pattern(50, 950);

    while (!stdio_usb_connected()) {
        tight_loop_contents();
        SysLed::loop();
    }

    sleep_ms(10);

    SysLed::off();

    printf("\n");
    printf("argv_test\n");
    printf("\n");

    Argv argv;

    argv.verbosity(1);

    while (true) {

        // Get console input if available.
        // This might result in a new command.
        int c = stdio_getchar_timeout_us(0);
        if (0 <= c && c <= 255) {
            if (argv.add_char(char(c))) {
                // newline received
                for (int i = 0; i < argv.argc(); i++)
                    printf("argv[%d]=\"%s\"\n", i, argv[i]);
                argv.reset();
            }
        }

    } // while (true)

    return 0;
}
