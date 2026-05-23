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

## Memory layout
