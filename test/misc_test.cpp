#include <cassert>
#include <cstdio>
#include "pico/stdlib.h"
#include "sys_led.h"
#include "bit_ops.h"
#include "dbg_gpio.h"

// connect these together
static const int dbg_gpio_out = 3;
static const int dbg_gpio_in = 4;


static void bit_ops_test()
{
    // 01101001, 3, 3
    //   ^^^ -> 101
    assert(extract<uint8_t>(0x69, 3, 3) == 5);

    // 01101001, 3, 0
    //      ^^^ -> 001
    assert(extract<uint8_t>(0x69, 3, 0) == 1);

    // 0001_0010_0011_0100_0101_0110_0111_1000, 9, 23
    // ^^^^_^^^^_^ -> 100
    assert(extract<uint32_t>(0x12345678, 9, 23) == 0x024);

    // 1111_1111    0xff
    //   01_0       0x2, 3, 3
    // 1101_0111    0xd7
    uint8_t v1 = 0xff;
    insert<uint8_t>(v1, 0x2, 3, 3);
    assert(v1 == 0xd7);

    // 10101010_10101010_10101010_10101010  0xaaaaaaaa
    //      110_0110011                     0x333, 10, 17
    // 10101110_01100110_10101010_10101010  0xae66aaaa
    uint32_t v2 = 0xaaaaaaaa;
    insert<uint32_t>(v2, 0x333, 10, 17);
    assert(v2 == 0xae66aaaa);
}


static void dbg_gpio_test()
{
    DbgGpio::init({dbg_gpio_out});

    assert(!gpio_get(dbg_gpio_in));
    {
        DbgGpio d(dbg_gpio_out);
        assert(gpio_get(dbg_gpio_in));
    }
    assert(!gpio_get(dbg_gpio_in));
}


int main()
{
    stdio_init_all();

    SysLed::init();
    SysLed::pattern(50, 950);

    while (!stdio_usb_connected()) {
        tight_loop_contents();
        SysLed::loop();
    }

    // With no delay here, we lose the first few lines of output.
    // Delaying 1 msec has been observed to work with a debug build.
    sleep_ms(10);

    bit_ops_test();
    dbg_gpio_test();

    printf("all tests passed\n");

    SysLed::pattern(950, 50);
    while (true)
        SysLed::loop();

    return 0;
}