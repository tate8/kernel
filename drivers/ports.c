#include "../cpu/types.h"

/*
    Port drivers for reading and writing to I/O ports
    A port is a specific address on the x86's IO bus. This bus provides communication with hardware devices

    Remember: 
     - The source and destination registers are switched from NASM
     - Inputs and outputs are separated by colons
*/
 
 
/* A handy C wrapper function that reads a byte from the specified port
 * "=a" (result) means: put AL register in C variable 'result' when finished
 * "d" (port) means: load EDX with C variable 'port'
 */
u8 port_byte_in(u16 port) {
    u8 result;
    __asm__("in %%dx, %%al" : "=a" (result) : "d" (port));
    return result;
}

/* "a" (data) means: load EAX with data
 * "d" (port) means: load EDX with port
 */
void port_byte_out(u16 port, u8 data) {
    __asm__("out %%al, %%dx" : : "a" (data), "d" (port));
}

u16 port_word_in(u16 port) {
    u16 result;
    __asm__("in %%dx, %%ax" : "=a" (result) : "d" (port));
    return result;
}

void port_word_out(u16 port, u16 data) {
    __asm__("out %%ax, %%dx" : : "a" (data), "d" (port));
}
