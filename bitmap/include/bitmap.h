#ifndef BITMAP_H__
#define BITMAP_H__

#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h> // Welp, there goes windows compatability

typedef struct bitmap bitmap_t;

// WARNING: Bit requests outside the bitmap and NULL pointers WILL result in a segfault
// This is by design. There is as little overhead as possible. Use it right and everything will be fine.

// Sets requested bit in bitmap
void bitmap_set(bitmap_t *const bitmap,const size_t bit);

// Clears requested bit in bitmap
void bitmap_reset(bitmap_t *const bitmap, const size_t bit);

// Returns bit in bitmap
bool bitmap_test(const bitmap_t *const bitmap, const size_t bit);
bool bitmap_at(const bitmap_t *const bitmap, const size_t bit);

// Flips bit in bitmap
void bitmap_flip(bitmap_t *const bitmap, const size_t bit);

// Find first set, SIZE_MAX on error/not found
size_t bitmap_ffs(const bitmap_t *const bitmap);

// Finds first zero, SIZE_MAX on error/not found
size_t bitmap_ffz(const bitmap_t *const bitmap);

// Resets bitmap contents to the desired pattern
// (pattern not guarenteed accurate for final bits
// 		if bit_count is not a multiple of 8)
void bitmap_format(bitmap_t *const bitmap, const uint8_t pattern);

// Gets total number of bits in bitmap
size_t bitmap_get_bits(const bitmap_t *const bitmap);

// Gets total number of bytes in bitmap
size_t bitmap_get_bytes(const bitmap_t *const bitmap);

// Gets pointer for writing bitmap to file
const uint8_t *bitmap_export(const bitmap_t *const bitmap);

// Creates a new bitmap with the provided data and size
bitmap_t *bitmap_import(const size_t n_bits, const  uint8_t *const bitmap_data);

// Creates a bitmap to contain n bits (zero initialized)
bitmap_t *bitmap_create(const size_t n_bits);

// Destructs and destroys bitmap object
void bitmap_destroy(bitmap_t *bitmap);


#endif
