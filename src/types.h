#ifndef TYPES_H_
#define TYPES_H_

#include<stdint.h>

// 8 bits
typedef unsigned char byte;

// immediate 8 bit data
typedef uint8_t d8;

// immediate 16 bit data
typedef uint16_t d16;

// 8 bit unsigned data, which are added o $FF00 in certain instructions
typedef uint8_t a8;

// 16 bit address
typedef uint16_t a16;

// 8 bit signed data, which are added to program counter
typedef uint8_t r8;

// true-false data
typedef _Bool bool;

#endif /* TYPES_H_ */
