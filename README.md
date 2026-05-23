# memory-allocator-c

Custom memory allocator in C — free list, block metadata, OS-level heap management.

![C99](https://img.shields.io/badge/C-99-blue) ![status](https://img.shields.io/badge/status-learning%20project-green) ![license](https://img.shields.io/badge/license-MIT-purple)

---

## Overview

Implements `my_malloc` and `my_free` from scratch, managing a heap divided
into blocks tracked by a doubly linked list. Each block carries a header
(`area`) with size, in-use status, and magic bytes for corruption detection.
A global `my_stats` header tracks overall allocator state.

---

## Features

- **Free list** — doubly linked list of blocks; O(n) first-fit search, O(1) coalescing
- **sbrk / OS heap** — requests raw memory from the OS, avoids libc malloc entirely
- **Magic bytes** — block corruption detection via header sentinels; catches wild writes early
- **Thread-safe** — coarse-grained lock on alloc/free

---

---

## Build & run

```bash
# compile
gcc -Wall -Wextra -Werror -pedantic -std=c99 allocator.c -o allocator

# run
./allocator

# with address sanitizer (recommended)
gcc -fsanitize=address -g allocator.c -o allocator && ./allocator
```

---

## API reference

| function / macro | description |
|---|---|
| `my_malloc(size)` | allocates `size` bytes from the heap; returns pointer or NULL on failure |
| `my_free(ptr)` | frees a previously allocated block; checks magic bytes before freeing |
| `my_stats` | global header — tracks total allocated, total freed, block count |
| `area` (block header) | per-block metadata: size, in-use flag, prev/next pointers, magic sentinel |

---

## Based on

- [Malloc is not magic](https://levelup.gitconnected.com/malloc-is-not-magic-implementing-my-own-memory-allocator-e0354e914402) — article walkthrough
- [Untangling lifetimes: the arena allocator — Ryan Fleury](https://www.rfleury.com/p/untangling-lifetimes-the-arena-allocator) — via YouTube description

---

## Related

See also `arena.c / arena.h` in this repo — a separate arena allocator
(reserve/commit model, mmap/VirtualAlloc) developed alongside this project.
The arena is simpler and faster for lifetime-scoped allocations; the free list
handles general-purpose dynamic allocation.
