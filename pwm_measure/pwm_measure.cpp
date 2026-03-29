
#include <cassert>
#include <cstdint>
#include <cstdio>
// pico
#include "pico/stdio.h"
#include "pico/stdio_usb.h"
#include "pico/stdlib.h"
#include "pico/time.h" // sleep_ms
//
#include "hardware/clocks.h" // clock_get_hz
#include "hardware/pwm.h"
//
#include "gpio_extra.h"


//  PWM            +-----| USB |-----+            PWM
//   0A         D0 | 1            40 | VBUS_OUT
//   0B         D1 | 2            39 | VSYS_IO
//             GND | 3            38 | GND
//   1A         D2 | 4            37 | 3V3_EN
//   1B         D3 | 5            36 | 3V3_OUT
//   2A         D4 | 6            35 | AREF
//   2B         D5 | 7            34 | A2/D28     6A
//             GND | 8            33 | GND
//   3A         D6 | 9            32 | A1/D27     5B
//   3B         D7 | 10           31 | A0/D26     5A
//   4A         D8 | 11           30 | RUN
//   4B         D9 | 12           29 | D22        3A
//             GND | 13           28 | GND
//   5A        D10 | 14           27 | D21        2B
//   5B        D11 | 15           26 | D20        2A
//   6A        D12 | 16           25 | D19        1B
//   6B        D13 | 17           24 | D18        1A
//             GND | 18           23 | GND
//   7A        D14 | 19           22 | D17        0B
//   7B        D15 | 20           21 | D16        0A
//                 +-----------------+

// PWM in has to be a B channel. The PWM slice is used to measure the pulse
// width, and the same pin is also used (as a GPIO) to trigger an interrupt
// on the falling edge of the pulse. The interrupt handler reads the counter
// value to get the pulse width, and then resets the counter to zero.

// PWM out is for testing and can be either channel of a different slice.

// If set, an output pin is driven with a fixed pulse width, with the intent
// that it is connected to the input pin for testing.
#define DRIVE_OUTPUT 0

#if DRIVE_OUTPUT

static constexpr int pwm_out_gpio = 16;

// pwm out is 5 msec period (200 Hz), 1 msec pulse (20% duty)
static constexpr int pwm_out_period_us = 5000;
static constexpr int pwm_out_us = 1000;

static const uint pwm_out_slice_num = pwm_gpio_to_slice_num(pwm_out_gpio);
static const uint pwm_out_chan = pwm_gpio_to_channel(pwm_out_gpio);

static void init_pwm_out();

#endif // DRIVE_OUTPUT

static constexpr int pwm_in_gpio = 15;

static const uint pwm_in_slice_num = pwm_gpio_to_slice_num(pwm_in_gpio);
static const uint pwm_in_chan = pwm_gpio_to_channel(pwm_in_gpio);

static void init_pwm_in();

static void gpio_irq_handler(uint gpio, uint32_t events, intptr_t arg);

// Pololu 4064: new pulse every 9 msec (close range) to 20 msec (longer range)
static const int pwm_avg_len = 4;
static uint32_t pwm_count = 0; // avg_len times the actual count


int main()
{
    stdio_init_all();

#if 1
    while (!stdio_usb_connected())
        tight_loop_contents();
#endif

    sleep_ms(100);

    printf("\n");
    printf("pwm_measure\n");
    printf("\n");

#if DRIVE_OUTPUT
    // can't use the same slice for in and out
    assert(pwm_in_slice_num != pwm_out_slice_num);

    init_pwm_out();
#endif // DRIVE_OUTPUT

    init_pwm_in();

    uint16_t last_us = UINT16_MAX;
    while (true) {
        // print only when the average changes
        uint16_t new_us = (pwm_count + pwm_avg_len / 2) / pwm_avg_len;
        if (new_us != last_us) {
            printf("pwm_count = %u", new_us);
            // Pololu 4064: pulse width 2000_us means nothing detected
            if (new_us < 1990) {
                // Pololu 4064: d_mm = 3_mm / 4_us * (t_us - 1000_us)
                printf("; dist_mm = %d", ((new_us - 1000) * 3 + 2) / 4);
            }
            printf("\n");
            last_us = new_us;
        }
    }

    return 0;
}


#if DRIVE_OUTPUT

static void init_pwm_out()
{
    // Clock the pwm at 1 MHz so everything's in microseconds
    //                                                    Pico2     Pico
    const uint32_t sys_clk_hz = clock_get_hz(clk_sys); // 150 MHz   125 MHz
    const uint32_t pwm_clk_hz = 1'000'000;             //
    const uint32_t pwm_div = sys_clk_hz / pwm_clk_hz;  // 150       125
    assert(pwm_div <= 255); // must fit in 8 bit register

    uint32_t pwm_out_wrap = pwm_out_period_us - 1;
    assert(pwm_out_wrap <= UINT16_MAX);

    uint16_t pwm_out_level = pwm_out_us;
    assert(pwm_out_level <= pwm_out_wrap);

    gpio_set_function(pwm_out_gpio, GPIO_FUNC_PWM);

    pwm_set_enabled(pwm_out_slice_num, false);
    pwm_config pc = pwm_get_default_config();
    pwm_config_set_clkdiv_int(&pc, pwm_div);
    pwm_init(pwm_out_slice_num, &pc, false);
    pwm_set_wrap(pwm_out_slice_num, pwm_out_wrap);
    pwm_set_chan_level(pwm_out_slice_num, pwm_out_chan, pwm_out_level);
    pwm_set_enabled(pwm_out_slice_num, true);
}

#endif // DRIVE_OUTPUT


static void init_pwm_in()
{
    assert(pwm_in_chan == PWM_CHAN_B);

    // In the chip, the pad goes to the gpio edge detector & interrupt
    // generator in parallel with going to the pwm slice, so we can use the
    // same pin as the pwm gate and the gpio interrupt source. We need the
    // interrupt at the end of the pulse to read and reset the counter.

    // set up pin for falling edge interrupt
    gpio_init(pwm_in_gpio);
    gpiox_set_callback(pwm_in_gpio, gpio_irq_handler, 0);
    gpio_set_irq_enabled(pwm_in_gpio, GPIO_IRQ_EDGE_FALL, true);

    // Clock the pwm at 1 MHz for 1 usec resolution       Pico2     Pico
    const uint32_t sys_clk_hz = clock_get_hz(clk_sys); // 150 MHz   125 MHz
    const uint32_t pwm_clk_hz = 1'000'000;             //
    const uint32_t pwm_div = sys_clk_hz / pwm_clk_hz;  // 150       125
    assert(pwm_div <= 255); // must fit in 8 bit register

    // set up pwm with pin as counter gate
    gpio_set_function(pwm_in_gpio, GPIO_FUNC_PWM);
    pwm_set_enabled(pwm_in_slice_num, false);
    pwm_config pc = pwm_get_default_config();
    pwm_config_set_clkdiv_mode(&pc, PWM_DIV_B_HIGH); // count while pin high
    pwm_config_set_clkdiv_int(&pc, pwm_div);
    pwm_init(pwm_in_slice_num, &pc, false);
    pwm_set_chan_level(pwm_in_slice_num, pwm_in_chan, 0);
    pwm_set_enabled(pwm_in_slice_num, true);
}


static void gpio_irq_handler(uint gpio, uint32_t events, [[maybe_unused]] intptr_t arg)
{
    // should only be called for pwm_in_gpio
    assert(gpio == pwm_in_gpio);
    assert(events == GPIO_IRQ_EDGE_FALL);

    // read and clear counter
    uint16_t new_count = pwm_get_counter(pwm_in_slice_num);
    pwm_set_counter(pwm_in_slice_num, 0);

    // Pololu 4064: pulse width is always a multiple of 4 usec
    new_count = (new_count + 2) / 4 * 4;

    // update average
    pwm_count =
        (pwm_count * (pwm_avg_len - 1) + pwm_avg_len / 2) / pwm_avg_len +
        new_count;
}
