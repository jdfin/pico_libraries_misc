
#include <cassert>
#include <cstdint>
#include <cstdio>
// pico
#include "pico/stdio.h"
#include "pico/stdio_usb.h"
#include "pico/stdlib.h"
// misc
#include "dbg_gpio.h"
#include "i2c_dev.h"
// touchscreen
#include "gt911.h"

static const int tp_sda_pin = 4;
static const int tp_scl_pin = 5;
static const int tp_rst_pin = 6;
static const int tp_int_pin = 7;
static const int tp_i2c_baud = 400'000;
static const uint8_t tp_i2c_addr = 0x14; // 0x14 or 0x5d

static uint8_t read_status(I2cDev &i2c);
static void write_status(I2cDev &i2c, uint8_t status);
static void read_touch(I2cDev &i2c, int &x, int &y);


int main()
{
    stdio_init_all();

    while (!stdio_usb_connected())
        tight_loop_contents();

    sleep_ms(10);

    printf("\n");
    printf("i2c_dev_test\n");
    printf("\n");

    I2cDev i2c_dev(i2c0, tp_scl_pin, tp_sda_pin, tp_i2c_baud);

    printf("i2c_dev_test: i2c running at %u Hz\n", i2c_dev.baud());

    Gt911 gt911(i2c_dev, tp_i2c_addr, tp_rst_pin, tp_int_pin);

    constexpr int verbosity = 2;
    assert(gt911.init(verbosity));

    printf("i2c_dev_test: ready\n");

    gt911.set_rotation(Gt911::Rotation::landscape);

    while (true) {

        sleep_ms(100);

        uint8_t status = read_status(i2c_dev);
        if (status != 0) {
            if ((status & 0x80) != 0 && (status & 0x0f) > 0) {
                int x, y;
                read_touch(i2c_dev, x, y);
                write_status(i2c_dev, 0);
                printf("i2c_dev_test: touch at (%d,%d)\n", x, y);
            } else {
                write_status(i2c_dev, 0);
            }
        }
    }

    printf("i2c_dev_test: bye\n");

    sleep_ms(10);

    return 0;
}


static uint8_t read_status(I2cDev &i2c)
{
    const int wr_len = 2;
    const uint8_t wr_buf[wr_len] = {0x81, 0x4e}; // TOUCH_STAT

    const int rd_len = 1;
    uint8_t rd_buf[rd_len];

    i2c.write_read_async_start(tp_i2c_addr, wr_buf, wr_len, rd_buf, rd_len);

    while (i2c.busy())
        tight_loop_contents();

    int rd_cnt = i2c.write_read_async_check();
    assert(rd_cnt == rd_len);

    return rd_buf[0];
}


static void write_status(I2cDev &i2c, uint8_t status)
{
    const int wr_len = 3;
    const uint8_t wr_buf[wr_len] = {0x81, 0x4e, status}; // TOUCH_STAT
    i2c.write_read_async_start(tp_i2c_addr, wr_buf, wr_len, nullptr, 0);

    while (i2c.busy())
        tight_loop_contents();
}


static void read_touch(I2cDev &i2c, int &x, int &y)
{
    const int wr_len = 2;
    const uint8_t wr_buf[wr_len] = {0x81, 0x50}; // TOUCH_1
    const int rd_len = 4;
    uint8_t rd_buf[rd_len];
    i2c.write_read_async_start(tp_i2c_addr, wr_buf, wr_len, rd_buf, rd_len);

    while (i2c.busy())
        tight_loop_contents();

    int rd_cnt = i2c.write_read_async_check();
    assert(rd_cnt == 4);

    x = (int(rd_buf[1]) << 8) | rd_buf[0];
    y = (int(rd_buf[3]) << 8) | rd_buf[2];
}
