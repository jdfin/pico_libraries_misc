
#include <cassert>
#include <cstdint>
#include <cstdio>
// pico
#include "hardware/i2c.h"
#include "pico/stdio.h"
#include "pico/stdio_usb.h"
#include "pico/stdlib.h"
// touchscreen
#include "gt911.h"
// misc
#include "dbg_gpio.h"
#include "i2c_extra.h"

static constexpr bool show_timing = false;

static const int tp_sda_pin = 4;
static const int tp_scl_pin = 5;
static const int tp_rst_pin = 6;
static const int tp_int_pin = 7;
static const int tp_i2c_baud = 400'000;
static const uint8_t tp_i2c_addr = 0x14; // 0x14 or 0x5d

// set to -1 to disable debug gpio
static const int dbg_gpio_num = 28;

static uint8_t read_status(i2c_inst_t *i2c);
static void write_status(i2c_inst_t *i2c, uint8_t status);
static void read_touch(i2c_inst_t *i2c, int &x, int &y);


int main()
{
    DbgGpio::init(dbg_gpio_num);

    stdio_init_all();

    while (!stdio_usb_connected())
        tight_loop_contents();

    sleep_ms(10);

    printf("\n");
    printf("i2c_extra_test\n");
    printf("\n");

    I2cDev i2c_dev(i2c0, tp_scl_pin, tp_sda_pin, tp_i2c_baud);

    printf("i2c_extra_test: i2c running at %u Hz\n", i2c_dev.baud());

    Gt911 gt911(i2c_dev, tp_i2c_addr, tp_rst_pin, tp_int_pin);

    constexpr int verbosity = 2;
    assert(gt911.init(verbosity));

    printf("i2c_extra_test: ready\n");

    gt911.set_rotation(Gt911::Rotation::landscape);

    while (true) {

        sleep_ms(100);

        uint8_t status = read_status(i2c0);
        if (status != 0) {
            if ((status & 0x80) != 0 && (status & 0x0f) > 0) {
                int x, y;
                read_touch(i2c0, x, y);
                write_status(i2c0, 0);
                printf("i2c_extra_test: touch at (%d,%d)\n", x, y);
            } else {
                write_status(i2c0, 0);
            }
        }
    }

    printf("i2c_extra_test: bye\n");

    sleep_ms(10);

    return 0;
}


// Timing:
//     2.2 usec to initiate the read (i2c_wr_rd_start)
//   135.5 usec waiting for i2c to finish (polling i2c_running)
//     0.5 usec to read status byte from fifo (i2c_wr_rd_check)
static uint8_t read_status(i2c_inst_t *i2c)
{

    uint8_t wr_buf[] = {0x81, 0x4e}; // TOUCH_STAT
    int wr_len = sizeof(wr_buf);
    int rd_len = 1;
    i2c_wr_rd_start(i2c, tp_i2c_addr, wr_buf, wr_len, rd_len);

    uint32_t t0_us;
    uint32_t t1_us;

    {
        DbgGpio d(dbg_gpio_num);

        t0_us = time_us_32();

        while (i2c_running(i2c))
            tight_loop_contents();

        t1_us = time_us_32();
    }

    uint8_t rd_buf[8];
    int rd_max = sizeof(rd_buf);
    int rd_cnt = i2c_wr_rd_check(i2c, rd_buf, rd_max);

    if constexpr (show_timing) {
        printf("i2c_extra_test: read in %lu usec; %d bytes:", //
               t1_us - t0_us, rd_cnt);
        for (int i = 0; i < rd_cnt; i++)
            printf(" %02x", rd_buf[i]);
        printf("\n");
    }

    return rd_buf[0];
}


static void write_status(i2c_inst_t *i2c, uint8_t status)
{
    uint8_t wr_buf[] = {0x81, 0x4e, status}; // TOUCH_STAT
    int wr_len = sizeof(wr_buf);
    int rd_len = 0;

    i2c_wr_rd_start(i2c, tp_i2c_addr, wr_buf, wr_len, rd_len);

    uint32_t t0_us = time_us_32();

    while (i2c_running(i2c))
        tight_loop_contents();

    uint32_t t1_us = time_us_32();

    if constexpr (show_timing)
        printf("i2c_extra_test: write in %lu usec\n", t1_us - t0_us);
}


static void read_touch(i2c_inst_t *i2c, int &x, int &y)
{
    uint8_t wr_buf[] = {0x81, 0x50}; // TOUCH_1
    int wr_len = sizeof(wr_buf);
    int rd_len = 4;
    i2c_wr_rd_start(i2c, tp_i2c_addr, wr_buf, wr_len, rd_len);

    uint32_t t0_us = time_us_32();

    while (i2c_running(i2c))
        tight_loop_contents();

    uint32_t t1_us = time_us_32();

    uint8_t rd_buf[8];
    int rd_max = sizeof(rd_buf);
    int rd_cnt = i2c_wr_rd_check(i2c, rd_buf, rd_max);

    if constexpr (show_timing) {
        printf("i2c_extra_test: read in %lu usec; %d bytes:", //
               t1_us - t0_us, rd_cnt);
        for (int i = 0; i < rd_cnt; i++)
            printf(" %02x", rd_buf[i]);
        printf("\n");
    }

    x = (int(rd_buf[1]) << 8) | rd_buf[0];
    y = (int(rd_buf[3]) << 8) | rd_buf[2];
}
