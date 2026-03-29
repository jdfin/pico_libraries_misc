#include "gpio_extra.h"

#include <assert.h>
#include <stdint.h>
#include <strings.h> // ffs()

#include "hardware/gpio.h"
#include "hardware/irq.h"


// handlers for each pin
// RP2040: NUM_BANK0_GPIOS = 30
// RP2350: NUM_BANK0_GPIOS = 48
static volatile struct {
    void (*func)(uint gpio, uint32_t event_mask, intptr_t arg);
    intptr_t arg;
} gpio_irq_mux_handlers[NUM_BANK0_GPIOS];


// The actual gpio interrupt handler (gpio_irq_callback_t).
// Call the user-installed handler for that pin.
static void gpio_irq_callback(uint gpio, uint32_t event_mask)
{
    if (gpio_irq_mux_handlers[gpio].func != NULL) {
        // it's up to the user handler to clear the interrupt if necessary
        gpio_irq_mux_handlers[gpio].func(gpio, event_mask,
                                         gpio_irq_mux_handlers[gpio].arg);
    } else {
        // no user handler for this pin; clear it
        gpio_acknowledge_irq(gpio, event_mask);
    }
}


static void gpiox_init()
{
    // clear handler table
    for (uint g = 0; g < NUM_BANK0_GPIOS; g++) {
        gpio_irq_mux_handlers[g].func = NULL;
        gpio_irq_mux_handlers[g].arg = 0;
    }

    gpio_set_irq_callback(gpio_irq_callback);
    irq_set_enabled(IO_IRQ_BANK0, true);
}


// Install a handler for a pin:
// void my_handler(uint gpio, uint32_t event_mask, intptr_t arg)
void gpiox_set_callback(uint gpio, //
                        void (*func)(uint, uint32_t, intptr_t), intptr_t arg)
{
    static int init = 0;

    if (init == 0) {
        gpiox_init();
        init = 1;
    }

    assert(gpio < NUM_BANK0_GPIOS);

    gpio_irq_mux_handlers[gpio].func = func;
    gpio_irq_mux_handlers[gpio].arg = arg;
}
