# libasm

A small x86-64 assembly library that re-implements a subset of common libc string and I/O routines for Linux, together with a C-based regression test suite.

## Overview

This repository builds a static library named `libasm.a` that contains NASM implementations of the following functions:

- `ft_strlen`
- `ft_strcpy`
- `ft_strcmp`
- `ft_write`
- `ft_read`
- `ft_strdup`

The project is organized to make it easy to:

- study how common libc behaviors map to Linux syscalls and low-level assembly,
- compare custom assembly implementations against the standard C library,
- validate crash behavior, return values, and `errno` propagation with automated tests.

## Repository Layout

- `ft_strlen.s`: Returns the length of a null-terminated string.
- `ft_strcpy.s`: Copies a source string into a destination buffer.
- `ft_strcmp.s`: Compares two strings byte by byte.
- `ft_write.s`: Wraps the Linux `write` syscall and mirrors libc-style `errno` handling.
- `ft_read.s`: Wraps the Linux `read` syscall and mirrors libc-style `errno` handling.
- `ft_strdup.s`: Allocates and duplicates a string by reusing `ft_strlen` and `ft_strcpy`.
- `tests/includes/tests.h`: Shared prototypes, macros, and test-suite declarations.
- `tests/src/main.c`: Test runner entry point.
- `tests/src/test_strlen.c`: Validation suite for `ft_strlen`.
- `tests/src/test_strcpy.c`: Validation suite for `ft_strcpy`.
- `tests/src/test_strcmp.c`: Validation suite for `ft_strcmp`.
- `tests/src/test_write.c`: Validation suite for `ft_write`.
- `tests/src/test_read.c`: Validation suite for `ft_read`.
- `tests/src/test_strdup.c`: Validation suite for `ft_strdup`.

## Requirements

The build system expects the following tools to be available:

- `nasm`
- `gcc`
- `make`
- a Linux x86-64 environment

## Build Instructions

### Build the static library

```bash
make
```

This generates `libasm.a`.

### Build the test executable

```bash
make test
```

This generates the test runner executable `lib_test` and links it against `libasm.a`.

### Clean generated files

```bash
make clean
```

Removes intermediate object and dependency directories.

### Full cleanup

```bash
make fclean
```

Removes the static library, the test executable, and generated build artifacts.

### Rebuild from scratch

```bash
make re
```

Performs a full cleanup and rebuild.

## Function Documentation

### `size_t ft_strlen(const char *str)`

Computes the number of bytes that appear before the terminating null byte in `str`.

- **Input:** pointer to a null-terminated string.
- **Return value:** string length in bytes.
- **Important note:** this function behaves like `strlen`; passing an invalid pointer is undefined behavior.

### `char *ft_strcpy(char *dest, const char *src)`

Copies `src` into `dest`, including the terminating null byte.

- **Input:** destination buffer and source string.
- **Return value:** the original `dest` pointer.
- **Important note:** `dest` must point to a buffer large enough to hold the full copied string.

### `int ft_strcmp(const char *s1, const char *s2)`

Compares two strings byte by byte until a difference is found or a null terminator is reached.

- **Input:** two null-terminated strings.
- **Return value:**
  - `0` when both strings are equal,
  - a negative value when `s1` is lexicographically smaller than `s2`,
  - a positive value when `s1` is lexicographically greater than `s2`.

### `ssize_t ft_write(int fd, const void *buf, size_t count)`

Writes up to `count` bytes from `buf` to the file descriptor `fd`.

- **Input:** file descriptor, buffer, and number of bytes to write.
- **Return value:** number of bytes written, or `-1` on failure.
- **Error handling:** when the syscall fails, the function stores the positive error code in `errno`, matching libc behavior.

### `ssize_t ft_read(int fd, void *buf, size_t count)`

Reads up to `count` bytes from `fd` into `buf`.

- **Input:** file descriptor, destination buffer, and maximum number of bytes to read.
- **Return value:** number of bytes read, `0` on end-of-file, or `-1` on failure.
- **Error handling:** when the syscall fails, the function stores the positive error code in `errno`, matching libc behavior.

### `char *ft_strdup(const char *s)`

Allocates a new heap buffer, copies `s` into it, and returns the new pointer.

- **Input:** source string.
- **Return value:** pointer to the duplicated string, or `NULL` if memory allocation fails.
- **Implementation detail:** the routine reuses `ft_strlen`, `malloc`, and `ft_strcpy`.

## Test Suite

The test program compares each assembly routine against the corresponding libc function.

### Covered behaviors

- standard success cases,
- edge cases,
- invalid descriptors,
- `NULL` pointer crash checks where applicable,
- pipe and file descriptor scenarios,
- `errno` propagation,
- repeated allocations and stress-style checks for `ft_strdup`.

### Run the tests

First build the executable:

```bash
make test
```

Then run it:

```bash
./lib_test
```

## Notes About the Assembly Sources

The `.s` files include comments that describe:

- the purpose of each routine,
- the registers used for parameters and return values,
- why specific registers are preserved,
- why external calls such as `malloc`, `ft_strlen`, `ft_strcpy`, and `__errno_location` are needed.

These comments are intended to make the low-level control flow easier to understand while keeping the implementation unchanged.
