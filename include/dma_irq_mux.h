#pragma once

#include "hardware/dma.h"

// RP2040 dma interrupt mux

// The RP2040 dma has two interrupts to the NVIC (irq0 and irq1), either of
// which can be raised by any of the 16 channels. The goal of this is to let
// them handle interrupts a bit more independently. In addition to being able
// to attach a handler to just one of the channels (and have a different
// handler for other channels), this also lets one specify a parameter to the
// handler, one parameter per channel.


#ifdef __cplusplus
extern "C" {
#endif


// install a handler
extern void dma_irq_mux_connect(uint irqn, uint channel_num,
                                void (*func)(void *), void *arg);


// enable/disable interrupts for a channel
static inline void dma_irq_mux_enable(uint irqn, uint channel, bool enable)
{
    dma_irqn_set_channel_enabled(irqn, channel, enable);
}

// trigger an interrupt for a channel
static inline void dma_irqn_mux_force(uint irqn, uint channel, bool force)
{
    if (force)
        dma_hw->irq_ctrl[irqn].intf |= (1u << channel);
    else
        dma_hw->irq_ctrl[irqn].intf &= ~(1u << channel);
}


#ifdef __cplusplus
}
#endif
