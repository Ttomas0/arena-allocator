#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "arena.h"








mem_arena *arena_create(u64 reserve_size, u64 commit_size){
    u32 pagesize = plat_get_pagesize();

    reserve_size = ALING_UP_POW2(reserve_size, pagesize);
    commit_size = ALING_UP_POW2(commit_size, pagesize);

    mem_arena *arena = plat_mem_reserve(reserve_size);

    if(!plat_mem_commit(arena, commit_size)){
        return NULL;
    }

    arena->reserve_size = reserve_size;
    arena->commit_size = commit_size;
    arena->position = ARENA_BASE_POSITION;
    arena->commit_position = ARENA_BASE_POSITION + commit_size;

    return arena;

}

/** 
 * @brief diring all the program commit_size and reserve_size are gonna be fixed values that we wouldnt want to touch, this values generally the are gonna be 4096 each
*/



// reserve_size = 1GB  <- virtual address space reserved, no physical memory yet
// commit_size  = 4KB  <- physical memory mapped at startup
//
// [ mem_arena | 4KB committed | ............. 1GB reserved but no RAM yet ............. ]
//               ^               ^                                                        ^
//               ARENA_BASE_POS  commit_position = ARENA_BASE_POS + commit_size  



void arena_destroy(mem_arena *arena){
    plat_mem_release(arena, arena->reserve_size);
}

void *arena_push(mem_arena *arena, u64 size, b32 non_zero){
    u64 pos_aligned = ALING_UP_POW2(arena->position, ARENA_ALING);
    // round up the current position to the next multiple of ARENA_ALIGN (8 bytes),
    // so the returned pointer is always valid for any type.
    u64 new_pos = pos_aligned + size;

    if (new_pos > arena->reserve_size){
        return NULL;
    }

    if(new_pos > arena->commit_position){
        u64 new_commit_pos = new_pos;
        new_commit_pos += arena->commit_position - 1;
        new_commit_pos -=  new_commit_pos % arena->commit_size;
        new_commit_pos = MIN(new_commit_pos, arena->reserve_size);

        // the OS only commits memory in multiples of page size (4096 bytes) — just like reserving.
        // without rounding: you ask for 4100 -> OS gives 8192 -> you think you have 4100 -> commit_position is wrong -> bug
        // with rounding:    you ask for 8192 -> OS gives 8192 -> commit_position = 8192 -> correct

        u8 *mem = (u8 *)arena + arena->commit_position;
        u64 commit_size = new_commit_pos - arena->commit_position; // only commit the new chunk, not the already committed memory

        if(!plat_mem_commit(mem, commit_size)){ //mem, commit_size the chuck of mem that we already commit
            return NULL;
        }

        arena->commit_position = new_commit_pos;
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
    mem_arena *perm_arena = arena_create(GiB(1), MiB(1));

    while (true){
        arena_push(perm_arena, MiB(16), false);
        getc(stdin); //waits for enter to continue
    }



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

#ifdef _WIN32
#include <windows.h> 

/**
 * @brief Returns the OS page size — the minimum unit the OS uses to manage virtual memory (typically 4096 bytes).
 * @return Page size in bytes.
 */

u32   plat_get_pagesize(void){
    SYSTEM_INFO sysinfo = {0};// zero-initialized to prevent meomory trash
    GetSystemInfo(&sysinfo); // this basically fills the struct with system info (page size, cpu count, etc)

    return sysinfo.dwPageSize //dwPageSize is the field that holds the OS page size (typically 4096)
}

/**
 * @brief Reserves virtual address space without allocating physical memory.
 * @param size Number of bytes to reserve.
 * @return Pointer to the reserved region, or NULL on failure.
 * @see https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualalloc
 */

void *plat_mem_reserve(u64 size){
    return VirtualAlloc(
    NULL,    // address to reserve at — NULL lets the OS choose
    size,      // number of bytes to reserve
    MEM_RESERVE,    // reserves virtual address space, no physical memory yet
    PAGE_READWRITE   // when committed, allow read and write access
);
}

/**
 * @brief Maps physical memory to a previously reserved region.
 * @param ptr  Address inside a reserved region.
 * @param size Number of bytes to commit.
 * @return true on success, false on failure.
 * @see https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualalloc
 */

b32   plat_mem_commit(void *ptr, u64 size){
    void *ret = VirtualAlloc(
        ptr, // address to commit — must be inside a previously reserved region
        size, 
        MEM_COMMIT, // maps physical memory to the reserved address space
        PAGE_READWRITE
    );  

    return ret != NULL;
}

/**
 * @brief Returns physical memory to the OS, keeping the virtual address space reserved.
 * @param ptr  Address of the region to decommit.
 * @param size Number of bytes to decommit.
 * @return true on success, false on failure.
 * @see https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualfree
 */


b32   plat_mem_decommit(void *ptr, u64 size){
    return VirtualFree(
        ptr, 
        size, 
        MEM_DECOMMIT  // returns physical memory to the OS, but keeps the virtual address space reserved
    );

}

/**
 * @brief Returns physical memory to the OS and releases the virtual address space.
 * @param ptr  Address of the region to release.
 * @param size Number of bytes to release.
 * @return true on success, false on failure.
 * @see https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualfree
 */

b32   plat_mem_release(void *ptr, u64 size){
    return VirtualFree(
        ptr, 
        size, 
        MEM_RELEASE  // returns physical memory to the OS, but keeps the virtual address space reserved
    );

}


#else
#include <sys/mman.h>
#include <unistd.h>

/**
 * @brief Returns the OS page size — the minimum unit the OS uses to manage virtual memory (typically 4096 bytes).
 * @return Page size in bytes.
 */
u32 plat_get_pagesize(void) {
    return getpagesize();
}

/**
 * @brief Reserves virtual address space without allocating physical memory.
 * @param size Number of bytes to reserve.
 * @return Pointer to the reserved region, or NULL on failure.
 */
void *plat_mem_reserve(u64 size) {
    return mmap(
        NULL,                        // address to reserve at — NULL lets the OS choose
        size,                        // number of bytes to reserve
        PROT_NONE,                   // no read/write yet, just reserve address space
        MAP_PRIVATE | MAP_ANONYMOUS, // not backed by any file
        -1, 0
    );
}

/**
 * @brief Maps physical memory to a previously reserved region.
 * @param ptr  Address inside a reserved region.
 * @param size Number of bytes to commit.
 * @return true on success, false on failure.
 */
b32 plat_mem_commit(void *ptr, u64 size) {
    return mprotect(ptr, size, PROT_READ | PROT_WRITE) == 0; // enables read/write on reserved region
}

/**
 * @brief Returns physical memory to the OS, keeping the virtual address space reserved.
 * @param ptr  Address of the region to decommit.
 * @param size Number of bytes to decommit.
 * @return true on success, false on failure.
 */
b32 plat_mem_decommit(void *ptr, u64 size) {
    return mprotect(ptr, size, PROT_NONE) == 0; // removes read/write access, keeps address space reserved
}

/**
 * @brief Returns physical memory to the OS and releases the virtual address space.
 * @param ptr  Address of the region to release.
 * @param size Number of bytes to release.
 * @return true on success, false on failure.
 */
b32 plat_mem_release(void *ptr, u64 size) {
    return munmap(ptr, size) == 0; // releases the virtual address space back to the OS
}

#endif