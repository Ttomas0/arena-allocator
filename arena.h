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
#define ALING_UP_POW2(n, p) (((u64)(n) + (u64)(p) - 1) & (~((u64)(p) - 1)))

/**
 * @brief Rounds n up to the next multiple of p (p must be a power of 2).
*
 * Why?: the CPU reads data most efficiently when its address is a multiple of 
 * its size. Misaligned reads either cost two memory fetches (x86) or crash (ARM).
 *
 * @param n  Value to align (current arena position).
 * @param p  Alignment boundary, must be a power of 2 (1, 2, 4, 8), basically the type that we are working on.
 * @return   Smallest multiple of p that is >= n.
 */
typedef struct {

    u64 reserve_size;
    u64 commit_size;

    u64 position;
    u64 commit_position;

}mem_arena; //this is the actual block of memory


#define ARENA_BASE_POSITION (sizeof(mem_arena))
#define ARENA_ALING (sizeof(void *)) //this returns 8 that is the max that any type can give so te arena is always aling at any type

#define PUSH_STRUCT(arena, T) (T*)arena_push((arena), sizeof(T), false)
#define PUSH_STRUCT_NZ(arena, T) (T*)arena_push((arena), sizeof(T), true)
#define PUSH_ARRAY(arena, T, n) (T*)arena_push((arena), n * sizeof(T), false)
#define PUSH_ARRAY_NZ(arena, T, n) (T*)arena_push((arena), n * sizeof(T), true)
// arena_push returns void* — a pointer with no type. (T*) tells the compiler
// what type of data will be stored there, so it can be used directly without casting manually every time.


u32   plat_get_pagesize(void);
void *plat_mem_reserve(u64 size);
b32   plat_mem_commit(void *ptr, u64 size);
b32   plat_mem_decommit(void *ptr, u64 size);
b32   plat_mem_release(void *ptr, u64 size);


#endif

