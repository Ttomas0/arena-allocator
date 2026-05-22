/**
 * @file arena.h
 * @brief Arena memory allocator implementation
 *
 * @details This allocator:
 * 
 */



#ifndef _ARENA_H
#define _ARENA_H 

#include <stdint.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef i8 b8;
typedef i32 b32;

/**
 * @brief   Boolean type with explicit size.
 * @details Uses an i32 (4 bytes) instead of bool to ensure
 *          alignment in structs and avoid compiler padding surprises.
 */

#define KiB(n) ((u64)(n) << 10) 
#define MiB(n) ((u64)(n) << 20)
#define GiB(n) ((u64)(n) << 30)

/**
 * @details
 * - (u64)(n): cast n to 64 bites system in binary 
 * @example 0000 0000 0000 0000 0000 0000 0000 0100
 * 
 * - the "<< 10": shift the number 10 times to the left and 
 * @example 0000 0000 0000 0000 0001 0000 0000 0000 and
 * this is basically like doing 4*2^10
 */

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

typedef struct {
    u64 capacity;
    u64 position;
}mem_arena; //this is the actual block of memory




#endif