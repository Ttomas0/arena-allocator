#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "arena.h"


u32   plat_get_pagesize(void); // returns the OS page size (typically 4096 bytes)

void *plat_mem_reserve(u64 size); // reserves virtual address space without allocating physical memory
b32   plat_mem_commit(void *ptr, u64 size);// maps physical memory to a previously reserved region
b32   plat_mem_decommit(void *ptr, u64 size); // unmaps physical memory from a region, keeping the address space reserved
b32   plat_mem_release(void *ptr, u64 size); // releases the reserved virtual address space back to the OS






mem_arena *arena_create(u64 capacity){
    mem_arena *arena = (mem_arena *)malloc(sizeof(mem_arena) + capacity); //we are definig a place of memory for capacity + the mem_arena cast in the mem_arena sizes
    //we cast with mem_arena so the compiler nows how to return the direction of the pointer
    

    arena->capacity = capacity;
    arena->position = ARENA_BASE_POSITION;// The struct lives at the start of the block, the usable memory follows it.
}

//[ mem_arena | ............ capacity bytes ............ ]
//      ^                          ^
//   the struct                the mem of the user
//   (capacity, position)

void arena_destroy(mem_arena *arena){
    free(arena);
}

void *arena_push(mem_arena *arena, u64 size, b32 non_zero){
    u64 pos_aligned = ALING_UP_POW2(arena->position, ARENA_ALING);
    // round up the current position to the next multiple of ARENA_ALIGN (8 bytes),
    // so the returned pointer is always valid for any type.
    u64 new_pos = pos_aligned + size;

    if (new_pos > arena->capacity){
        return NULL;
    }

    arena->position = new_pos;
    
    u8 *out = (u8*)arena + pos_aligned; 
    // (u8*) casts arena to byte pointer so pointer arithmetic advances byte by byte.
    // without it: arena + 24 -> 24 * sizeof(mem_arena) = 24 * 24 = 576 bytes forward.
    // with it:  (u8*)arena + 24 -> 24 * sizeof(u8)      = 24 * 1  = 24  bytes forward.

    if (!non_zero){
        memset(out, 0, size);
    }
    // arena_push only reserves the block, it does not write to it (bytes may contain garbage).
    // if non_zero is 0, memset clears the block so the caller gets clean zeroed memory.

    return out; //this returns where your block is like malloc does

}

void arena_pop(mem_arena *arena, u64 size){
    size = MIN(size, arena->position - ARENA_BASE_POSITION); //this is a security check so the user cant pop more thatn the sizes that we actually have

    arena->position -= size;
}

void arena_pop_to(mem_arena *arena, u64 position){
    u64 size = position < arena->position ? arena->position - position : 0;//how many positions i want to pop

    arena_pop(arena, size);
}


void arena_clear(mem_arena *arena){
    arena_pop_to(arena, ARENA_BASE_POSITION);
}

int main(void){
    mem_arena *perm_arena = arena_create(MiB(1));

    // Fat pointer: instead of just storing the address, you also carry the size.
    //
    // Regular pointer:
    //   int *p = arr;        → only knows WHERE it starts, not how long it is
    //
    // Fat pointer (manual in C):
    //   struct { int *ptr; size_t len; } fp = { arr, 5 };
    //                        → knows WHERE it starts AND how long it is

    arena_destroy(perm_arena);
}