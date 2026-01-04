#include "dma_irq_mux.h"

#include <stdint.h>
#include <strings.h> // ffs()

#include "hardware/dma.h"
#include "hardware/irq.h"


// handlers for up to 16 channels for each irq

static volatile struct {
    void (*func)(void *);
    void *arg;
} dma_irq_mux_handlers[2][NUM_DMA_CHANNELS];


// The actual dma irq handlers. Figure out which channel interrupted and
// call the user-installed handler for that channel.
static void dma_irq0_handler()
{
    uint32_t active = dma_hw->irq_ctrl[0].ints;

    // datasheet doesn't say what the upper bits are; clear them
    active &= ((1 << NUM_DMA_CHANNELS) - 1); // 16 -> 0xffff

    // For each active interrupt, clear it before calling the handler. That
    // way if the handler is still running when the next interrupt happens,
    // it'll "queue" and the handler will run again.

    while (active != 0) {
        int channel = ffs(active) - 1; // ffs() returns 1..16; channel is 0..15
        dma_irqn_acknowledge_channel(0, channel);
        active &= ~(1 << channel); // clear bit in active
        dma_irqn_mux_force(0, channel, false); // if it was forced, clear the force
        dma_irq_mux_handlers[0][channel].func(
            dma_irq_mux_handlers[0][channel].arg);
    }
}


static void dma_irq1_handler()
{
    uint32_t active = dma_hw->irq_ctrl[1].ints;
    active &= ((1 << NUM_DMA_CHANNELS) - 1); // 16 -> 0xffff
    while (active != 0) {
        int channel = ffs(active) - 1; // ffs() returns 1..16; channel is 0..15
        dma_irqn_acknowledge_channel(1, channel);
        active &= ~(1 << channel); // clear bit in active
        dma_irq_mux_handlers[1][channel].func(
            dma_irq_mux_handlers[1][channel].arg);
    }
}


// Install a handler for a channel: void my_handler(void *arg)
void dma_irq_mux_connect(uint irqn, uint channel, void (*func)(void *), void *arg)
{
    static int init = 0;

    if (init == 0) {
        for (uint i = 0; i < 2; i++) {
            for (uint c = 0; c < NUM_DMA_CHANNELS; c++) {
                dma_irq_mux_handlers[i][c].func = NULL;
                dma_irq_mux_handlers[i][c].arg = NULL;
                dma_irqn_set_channel_enabled(i, c, false);
                dma_irqn_acknowledge_channel(i, c);
            }
        }
        irq_set_exclusive_handler(DMA_IRQ_0, dma_irq0_handler);
        irq_set_enabled(DMA_IRQ_0, true);
        irq_set_exclusive_handler(DMA_IRQ_1, dma_irq1_handler);
        irq_set_enabled(DMA_IRQ_1, true);
        init = 1;
    }

    dma_irq_mux_handlers[irqn][channel].func = func;
    dma_irq_mux_handlers[irqn][channel].arg = arg;
}