#include "pwm_extra.h"

#include <assert.h>
#include <stdint.h>
#include <strings.h> // ffs()

#include "hardware/irq.h"
#include "hardware/pwm.h"


// NUM_PWM_IRQS is 1 for RP2040, and 2 for RP2350
// NUM_PWM_SLICES is 8 for RP2040, and 12 for RP2350

// handlers for 8 slices
static volatile struct {
    void (*func)(intptr_t);
    intptr_t arg;
} pwm_irq_mux_handlers[NUM_PWM_IRQS][NUM_PWM_SLICES];


// The actual pwm interrupt handler. Figure out which slice interrupted and
// call the user-installed handler for that slice.
static void pwm_irqn_handler(uint irqn)
{
    assert(irqn < NUM_PWM_IRQS);

    uint32_t active = pwm_irqn_get_status_mask(irqn);

    // datasheet doesn't say what the upper bits are; clear them
    active &= ((1 << NUM_PWM_SLICES) - 1); // 8 -> 0x00ff, 12 -> 0x0fff

    // For each active interrupt, clear it before calling the handler. That
    // way if the handler is still running when the next interrupt happens,
    // it'll "queue" and the handler will run again. This effect is observed
    // when sending DCC bits, and the processing occasionally takes slightly
    // longer than a bit time.

    while (active != 0) {
        uint slice = ffs(active) - 1; // ffs() is 1-based; slice is 0-based
        assert(slice < NUM_PWM_SLICES);
        pwm_clear_irq(slice);
        active &= ~(1 << slice); // clear bit in active
        pwm_irq_mux_handlers[irqn][slice].func(
            pwm_irq_mux_handlers[irqn][slice].arg);
    }
}


static void pwm_irq0_handler()
{
    pwm_irqn_handler(0);
}


#if (NUM_PWM_IRQS > 1)
static void pwm_irq1_handler()
{
    pwm_irqn_handler(1);
}
#endif


// Clear handler table.
static void pwmx_init()
{
    for (uint i = 0; i < NUM_PWM_IRQS; i++) {
        for (uint s = 0; s < NUM_PWM_SLICES; s++) {
            pwm_irq_mux_handlers[i][s].func = NULL;
            pwm_irq_mux_handlers[i][s].arg = 0;
            pwm_set_irq_enabled(s, false);
            pwm_clear_irq(s);
        }
    }

    // irq_set_exclusive_handler sets the handler for _both_ cores
    // irq_set_enabled enables the interrupt for only the calling core

    // RP2040 has only PWM_IRQ_WRAP
    // RP2350 has PWM_IRQ_WRAP==PWM_IRQ_WRAP_0, and PWM_IRQ_WRAP_1

    irq_set_exclusive_handler(PWM_IRQ_WRAP, pwm_irq0_handler);
    irq_set_enabled(PWM_IRQ_WRAP, true);

#if (NUM_PWM_IRQS > 1)
    irq_set_exclusive_handler(PWM_IRQ_WRAP_1, pwm_irq1_handler);
    irq_set_enabled(PWM_IRQ_WRAP_1, true);
#endif
}


// Install a handler for a slice: void my_handler(intptr_t arg)
void pwmx_irqn_set_slice_handler(uint irqn, uint slice, //
                                 void (*func)(intptr_t), intptr_t arg)
{
    static int init = 0;

    if (init == 0) {
        pwmx_init();
        init = 1;
    }

    assert(irqn < NUM_PWM_IRQS);
    assert(slice < NUM_PWM_SLICES);

    pwm_irq_mux_handlers[irqn][slice].func = func;
    pwm_irq_mux_handlers[irqn][slice].arg = arg;
}
