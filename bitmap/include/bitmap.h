#ifndef BITMAP_H__
#define BITMAP_H__

#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

typedef struct bitmap bitmap_t;



// WARNING: Bit requests outside the bitmap and NULL pointers WILL result in a segfault
// This is by design. There is as little overhead as possible. Use it right and everything will be fine.

// Sets requested bit in bitmap
void bitmap_set(bitmap_t *bitmap, size_t bit);

// Clears requested bit in bitmap
void bitmap_reset(bitmap_t *bitmap, size_t bit);

// Returns bit in bitmap
bool bitmap_test(bitmap_t *bitmap, size_t bit);
bool bitmap_at(bitmap_t *bitmap, size_t bit);

// Flips bit in bitmap
void bitmap_flip(bitmap_t *bitmap, size_t bit);

// Resets bitmap contents to the desired pattern
// (pattern not guarenteed accurate for final bits
// 		if bit_count is not a multiple of 8)
void bitmap_format(bitmap_t *bitmap, uint8_t pattern);

// Gets total number of bits in bitmap
size_t bitmap_get_bits(bitmap_t *bitmap);

// Gets total number of bytes in bitmap
size_t bitmap_get_bytes(bitmap_t *bitmap);

// Gets pointer for writing bitmap to file
const uint8_t *bitmap_export(bitmap_t *bitmap);

// Creates a new bitmap with the provided data and size
bitmap_t *bitmap_import(size_t n_bits, uint8_t *bitmap_data);

// Creates a bitmap to contain n bits (zero initialized)
bitmap_t *bitmap_initialize(size_t n_bits);

// Destructs and destroys bitmap object
void bitmap_destroy(bitmap_t *bitmap);


#endif
