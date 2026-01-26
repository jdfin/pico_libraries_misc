#include <cassert>
#include <cstdio>

#include "sys_led.h"


void __assert_func(const char *file, int line, const char *func,
                   const char *failedexpr)
{
    printf("assertion \"%s\" failed: file \"%s\", line %d%s%s\n", failedexpr,
           file, line, func ? ", function: " : "", func ? func : "");

    SysLed::init();            // ok if this has already been called
    SysLed::pattern(250, 250); // blink at 2 Hz
    while (true)
        SysLed::loop(); // blink
}
