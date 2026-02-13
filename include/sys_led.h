#pragma once

#include <climits>
#include <cstdint>
#include "hardware/timer.h"
#include "hardware/gpio.h"

// required as of SDK 2.2.0
#ifdef __cplusplus
extern "C" {
#endif

#include "pico/status_led.h"

#ifdef __cplusplus
}
#endif


class SysLed {

    public:

        static void init()
        {
            // can't call status_led_init() more than once on wifi picos
            if (_ready)
                return;

#ifdef TINY2040_LED_R_PIN
            gpio_init(TINY2040_LED_R_PIN);
            gpio_put(TINY2040_LED_R_PIN, 1);
            gpio_set_dir(TINY2040_LED_R_PIN, GPIO_OUT);
#endif
#ifdef TINY2040_LED_G_PIN
            gpio_init(TINY2040_LED_G_PIN);
            gpio_put(TINY2040_LED_G_PIN, 1);
            gpio_set_dir(TINY2040_LED_G_PIN, GPIO_OUT);
#endif
#ifdef TINY2040_LED_B_PIN
            gpio_init(TINY2040_LED_B_PIN);
            gpio_put(TINY2040_LED_B_PIN, 1);
            gpio_set_dir(TINY2040_LED_B_PIN, GPIO_OUT);
#endif
            status_led_init();
            off();

            _ready = true;
        }

        static void set(bool on)
        {
            status_led_set_state(on);

            _on_ms = 0;
            _off_ms = 0;
        }

        static void on()
        {
            set(true);
        }

        static void off()
        {
            set(false);
        }

        // on for on_ms, off for off_ms, repeat
        static void pattern(uint32_t on_ms, uint32_t off_ms)
        {
            _on_ms = on_ms;
            _off_ms = off_ms;

            if (_on_ms > 0) {
                _on_next_ms = time_us_64() / 1000;
                _off_next_ms = UINT64_MAX;
            } else if (_off_ms > 0) {
                _on_next_ms = UINT64_MAX;
                _off_next_ms = time_us_64() / 1000;
            } // else pattern disabled by _on_ms == _off_ms == 0

            loop();
        }

        // This only has to be called if pattern() needs to work.
        static void loop()
        {
            if (_on_ms == 0 && _off_ms == 0)
                return;

            uint64_t now_ms = time_us_64() / 1000;
            if (_on_next_ms <= now_ms) {
                status_led_set_state(true);
                _on_next_ms = UINT64_MAX;
                // if s2 should be on for any duration,
                // schedule it to come on after s1's duration
                if (_off_ms > 0) {
                    _off_next_ms = now_ms + _on_ms;
                }
            } else if (_off_next_ms <= now_ms) {
                status_led_set_state(false);
                _off_next_ms = UINT64_MAX;
                if (_on_ms > 0) {
                    _on_next_ms = now_ms + _off_ms;
                }
            }
        }

    private:

        static bool _ready;

        static uint32_t _on_ms;
        static uint32_t _off_ms;

        static uint64_t _on_next_ms;
        static uint64_t _off_next_ms;

}; // class SysLed