#include "../include/bitmap.h"

struct bitmap {
    uint8_t *data;
    uint8_t leftover_bits;
    size_t bit_count, byte_count;
};

// lookup instead of always shifting bits. Should be faster? Confirmed: 10% faster
// Also, using native int width because it should be faster as well. - Negligible/indeterminate
const static uint8_t mask[] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

const static uint8_t invert_mask[] = { 0xFE, 0xFD, 0xFB, 0xF7, 0xEF, 0xDF, 0xBF, 0x7F};
// I won't lie, I only knew 0xFE without getting a calculator
// Way more testing than I should waste my time on suggested uint8_t was faster
// but it may still be negligible/indeterminate.
// Power's out and I'm on battery, so my machine may be sabotaging timings out of self-preservaton
// It shouldn't matter since one of them being non-int-width is the limiting factor
// At least making it uint8_t should lower the footprint?
// (Nope? No diff in binary size, potentially due to gaps?)

void bitmap_set(bitmap_t *const bitmap, const size_t bit) {
    bitmap->data[bit >> 3] |= mask[bit & 0x07];
}

void bitmap_reset(bitmap_t *const bitmap, const size_t bit) {
    bitmap->data[bit >> 3] &= invert_mask[bit & 0x07];
}

bool bitmap_test(const bitmap_t *const bitmap, const size_t bit) {
    return bitmap->data[bit >> 3] & mask[bit & 0x07];
}

bool bitmap_at(const bitmap_t *const bitmap, const  size_t bit) {
    return bitmap_test(bitmap, bit);
}

void bitmap_flip(bitmap_t *const bitmap, const size_t bit) {
    bitmap->data[bit >> 3] ^= mask[bit & 0x07];
}

size_t bitmap_get_bits(const bitmap_t *const bitmap) {
    return bitmap->bit_count;
}

size_t bitmap_get_bytes(const bitmap_t *const bitmap) {
    return bitmap->byte_count;
}

const uint8_t *bitmap_export(const bitmap_t *const bitmap) {
    return bitmap->data;
}

void bitmap_format(bitmap_t *const bitmap, const uint8_t pattern) {
    memset(bitmap->data, pattern, bitmap->byte_count);
}

bitmap_t *bitmap_create(const size_t n_bits) {
    if (n_bits) { // must be non-zero
        bitmap_t *bitmap = (bitmap_t *) malloc(sizeof(bitmap_t));
        if (bitmap) {
            bitmap->bit_count = n_bits;
            bitmap->byte_count = n_bits >> 3;
            bitmap->leftover_bits = n_bits & 0x07;
            bitmap->byte_count += (bitmap->leftover_bits ? 1 : 0);
            bitmap->data = (uint8_t *)calloc(bitmap->byte_count, 1);

            if (bitmap->data) {
                return bitmap;
            }
            free(bitmap);
        }
    }
    return NULL;
}

bitmap_t *bitmap_import(const size_t n_bits, const uint8_t *const bitmap_data) {
    bitmap_t *bitmap = bitmap_create(n_bits);
    if (bitmap && bitmap_data) {
        memcpy(bitmap->data, bitmap_data, bitmap->byte_count);
        return bitmap;
    }
    free(bitmap);
    return NULL;
}

void bitmap_destroy(bitmap_t *bitmap) {
    if (bitmap) {
        free(bitmap->data);
        free(bitmap);
    }
}


size_t bitmap_ffs(const bitmap_t *const bitmap) {
    if (bitmap) {
        // I've spent over an hour trying to write it in a smart way.
        // I give up.
        size_t result = 0;
        for (; result < bitmap->bit_count && !bitmap_test(bitmap, result); ++result) {}
        return (result == bitmap->bit_count ? SIZE_MAX : result);
    }
    return SIZE_MAX;
}

size_t bitmap_ffz(const bitmap_t *const bitmap) {
    if (bitmap) {
        // I've spent over an hour trying to write it in a smart way.
        // I give up.
        size_t result = 0;
        for (; result < bitmap->bit_count && bitmap_test(bitmap, result); ++result) {}
        return (result == bitmap->bit_count ? SIZE_MAX : result);
    }
    return SIZE_MAX;
}

// fls flz?
