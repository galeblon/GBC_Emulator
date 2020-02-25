#ifndef __REGS_H_
#define __REGS_H_

#include "types.h"

struct cpu_regs {
    union {
        struct {
            d8 F;
            d8 A;
        }__attribute__((packed));
        d16 AF;
    };
    union {
        struct {
            d8 C;
            d8 B;
        }__attribute__((packed));
        d16 BC;
    };
    union {
        struct {
            d8 E;
            d8 D;
        }__attribute__((packed));
        d16 DE;
    };
    union {
        struct {
            d8 L;
            d8 H;
        }__attribute__((packed));
        d16 HL;
    };
    d16 SP;
    d16 PC;
}__attribute__((packed));

#endif // __REGS_H_
