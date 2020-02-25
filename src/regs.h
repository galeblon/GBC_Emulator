#ifndef __REGS_H_
#define __REGS_H_

#include "types.h"

struct cpu_flags {
    u8 zero : 4;
    u8 C : 1;
    u8 H : 1;
    u8 N : 1;
    u8 Z : 1;
}__attribute__((packed));

struct cpu_regs {
    union {
        struct {
            union {
                struct cpu_flags FLAGS;
                u8 F;
            };
            u8 A;
        }__attribute__((packed));
        u16 AF;
    };
    union {
        struct {
            u8 C;
            u8 B;
        }__attribute__((packed));
        u16 BC;
    };
    union {
        struct {
            u8 E;
            u8 D;
        }__attribute__((packed));
        u16 DE;
    };
    union {
        struct {
            u8 L;
            u8 H;
        }__attribute__((packed));
        u16 HL;
    };
    u16 SP;
    u16 PC;
}__attribute__((packed)) REGISTERS;

void registers_prepare();

#endif // __REGS_H_
