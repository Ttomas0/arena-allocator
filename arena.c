#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "arena.h"

mem_arena *arena_create(u64 capacity); //is like doing malloc(sizeof(mem_arena) + capacity), position = 0
void arena_destroy(mem_arena* arena); //its like doing free(arena), because in arena we free all at once

void *arena_push(mem_arena *arena, u64 size);//returns  base+position, next +=size
void arena_pop(mem_arena *arena, u64 size);//position -= size, destroy the last push
void arena_pop_to(mem_arena *arena, u64 position);// position = position, destroy the arena to the position that we want
void arena_clear(mem_arena *arena);