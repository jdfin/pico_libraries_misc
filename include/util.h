#pragma once


inline char to_hex(unsigned i)
{
    if (i <= 9)
        return '0' + i;
    else if (i <= 15)
        return 'a' + (i - 10);
    else
        return '?';
}


inline bool is_xip(const void *addr)
{
    return (addr >= (void *)XIP_BASE) && (addr < (void *)SRAM_BASE);
}


inline bool is_ram(const void *addr)
{
    return (addr >= (void *)SRAM_BASE) && (addr < (void *)SRAM_END);
}


// convert an XIP address to an XIP address that won't update the cache
inline const void *xip_nocache(const void *xip_adrs)
{
    if (is_xip(xip_adrs)) {
        uint32_t xip = reinterpret_cast<uint32_t>(xip_adrs);
        xip = (xip & ~0x0f000000) | 0x03000000;
        xip_adrs = reinterpret_cast<void *>(xip);
    }
    return xip_adrs;
}