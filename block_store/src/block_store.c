#include "../include/block_store.h"

// Overriding these will probably break it since I'm not testing it that much
// it probably won't go crazy so long as the sizes are reasonable and powers of two
// So just don't touch it unless you want to debug it
#define BLOCK_COUNT 65536
#define BLOCK_SIZE 1024
#define FBM_SIZE ((BLOCK_COUNT >> 3) / BLOCK_SIZE)
#if (((FBM_SIZE * BLOCK_SIZE) << 3) != BLOCK_COUNT)
    #error "BLOCK MATH DIDN'T CHECK OUT"
#endif

// Handy macro, does what it says on the tin
#define BLOCKID_VALID(id) ((id > (FBM_SIZE - 1)) && (id < BLOCK_COUNT))

size_t utility_read_file(const int fd, uint8_t *buffer, const size_t count);

size_t utility_write_file(const int fd, const uint8_t *buffer, const size_t count);

struct block_store {
    bitmap_t *dbm;
    bitmap_t *fbm;
    uint8_t *data_blocks;
};

block_store_t *block_store_create() {
    block_store_t *bs = malloc(sizeof(block_store_t));
    if (bs) {
        bs->fbm = bitmap_create(BLOCK_COUNT);
        if (bs->fbm) {
            bs->dbm = bitmap_create(BLOCK_COUNT);
            if (bs->dbm) {
                // Eh, calloc, why not (technically a security risk if we don't)
                bs->data_blocks = calloc(BLOCK_SIZE, BLOCK_COUNT - FBM_SIZE);
                if (bs->data_blocks) {
                    for (size_t idx = 0; idx < FBM_SIZE; ++idx) {
                        bitmap_set(bs->fbm, idx);
                        bitmap_set(bs->dbm, idx);
                    }
                    block_store_errno = BS_OK;
                    return bs;
                }
                bitmap_destroy(bs->dbm);
            }
            bitmap_destroy(bs->fbm);
        }
        free(bs);
    }
    block_store_errno = BS_MEMORY;
    return NULL;
}

void block_store_destroy(block_store_t *const bs) {
    if (bs) {
        bitmap_destroy(bs->fbm);
        bs->fbm = NULL;
        bitmap_destroy(bs->dbm);
        bs->dbm = NULL;
        free(bs->data_blocks);
        bs->data_blocks = NULL;
        free(bs);
        block_store_errno = BS_OK;
        return;
    }
    block_store_errno = BS_PARAM;
}

size_t block_store_allocate(block_store_t *const bs) {
    if (bs && bs->fbm) {
        size_t free_block = bitmap_ffz(bs->fbm);
        if (free_block != SIZE_MAX) {
            bitmap_set(bs->fbm, free_block);
            bitmap_set(bs->dbm, (free_block >> 3) / BLOCK_SIZE); // Set that FBM block as changed
            // not going to mark dbm because there's no change (yet)
            return free_block;
        }
        block_store_errno = BS_FULL;
        return 0;
    }
    block_store_errno = BS_PARAM;
    return 0;
}

size_t block_store_release(block_store_t *const bs, const size_t block_id) {
    if (bs && bs->fbm && BLOCKID_VALID(block_id)) {
        // we could clear the dirty bit, since the info is no longer in use but...
        // We'll keep it. Could be useful. Doesn't really hurt anything.
        // Keeps it more true to a standard block device.
        // You could also use this function to format the specified block for security reasons
        bitmap_reset(bs->fbm, block_id);
        block_store_errno = BS_OK;
        return block_id;
    }
    block_store_errno = BS_PARAM;
    return 0;
}

size_t block_store_read(const block_store_t *const bs, const size_t block_id, void *buffer, const size_t nbytes, const size_t offset) {
    if (bs && bs->fbm && bs->data_blocks && BLOCKID_VALID(block_id)
            && buffer && nbytes && (nbytes + offset <= BLOCK_SIZE)) {
        // Not going to forbid reading of not-in-use blocks (but we'll log it via errno)
        size_t total_offset = offset + (BLOCK_SIZE * (block_id - FBM_SIZE));
        memcpy((void *)buffer, (void *)(bs->data_blocks + total_offset), nbytes);
        block_store_errno = bitmap_test(bs->fbm, block_id) ? BS_OK : BS_FBM_REQUEST_MISMATCH;
        return nbytes;
    }
    // technically we return BS_PARAM even if the internal structure of the BS object is busted
    // Which, in reality, would be more of a BS_INTERNAL or a BS_FATAL... but it'll add another branch to everything
    // And technically the bs is a parameter...
    block_store_errno = BS_PARAM;
    return 0;
}

size_t block_store_write(block_store_t *const bs, const size_t block_id, const void *buffer, const size_t nbytes, const size_t offset) {
    if (bs && bs->fbm && bs->dbm && bs->data_blocks && BLOCKID_VALID(block_id)
            && buffer && nbytes && (nbytes + offset <= BLOCK_SIZE)) {
        // Not going to forbid writing of not-in-use blocks (but we'll log it via errno)
        bitmap_set(bs->dbm, block_id);
        size_t total_offset = offset + (BLOCK_SIZE * (block_id - FBM_SIZE));
        memcpy((void *)(bs->data_blocks + total_offset), buffer, nbytes);
        block_store_errno = bitmap_test(bs->fbm, block_id) ? BS_OK : BS_FBM_REQUEST_MISMATCH;
        return nbytes;
    }
    block_store_errno = BS_PARAM;
    return 0;
}

block_store_t *block_store_import(const char *const filename) {
    block_store_t *bs = NULL;
    int fd = 0;
    if (filename) {
        struct stat file_stat;
        if ((!stat(filename, &file_stat)) && (file_stat.st_size == BLOCK_COUNT * BLOCK_SIZE)) {
            // file got stat'd and it looks ok
            fd = open(filename, O_RDONLY);
            if (fd != -1) {
                bs = malloc(sizeof(block_store_t));
                if (bs) {
                    bs->dbm = bitmap_create(BLOCK_COUNT);
                    bs->data_blocks = malloc(BLOCK_SIZE * (BLOCK_COUNT - FBM_SIZE));
                    if (bs->dbm && bs->data_blocks) {

                        // Ok, bs, dbm, and data_blocks exist
                        // File is open.
                        // Read in the file, suing data_blocks as a buffer

                        // Read in FBM
                        if (utility_read_file(fd, bs->data_blocks, FBM_SIZE * BLOCK_SIZE) == (FBM_SIZE * BLOCK_SIZE)) {
                            bs->fbm = bitmap_import(BLOCK_COUNT, bs->data_blocks);
                            if (bs->fbm) {

                                // FBM has been read in, all that is left is the reading of the actual block contents
                                // going to be super lazy/terrible and just tell the OS to read it all at once

                                if (utility_read_file(fd, bs->data_blocks, BLOCK_SIZE * (BLOCK_COUNT - FBM_SIZE)) == (BLOCK_SIZE * (BLOCK_COUNT - FBM_SIZE))) {
                                    //Done. Finally. Phew.
                                    close(fd);
                                    block_store_errno = BS_OK;
                                    return bs;
                                }
                                block_store_errno = BS_FILE_IO;
                                goto bs_import_unwind_D;
                            }
                            block_store_errno = BS_MEMORY;
                            goto bs_import_unwind_C;
                        }
                        block_store_errno = BS_FILE_IO;
                        goto bs_import_unwind_C;
                    }
                    block_store_errno = BS_MEMORY;
                    goto bs_import_unwind_C;
                }
                block_store_errno = BS_MEMORY;
                goto bs_import_unwind_B;
            }
        }
        // couldn't stat or open
        block_store_errno = BS_FILE_ACCESS;
        goto bs_import_unwind_A;
    }
    block_store_errno = BS_PARAM;
    goto bs_import_unwind_A;

bs_import_unwind_D:
    free(bs->fbm);
bs_import_unwind_C:
    free(bs->dbm);
    free(bs->data_blocks);
    free(bs);
bs_import_unwind_B:
    close(fd);
bs_import_unwind_A:
    return NULL;
}

size_t block_store_export(const block_store_t *const bs, const char *const filename) {
    // Thankfully, this is less of a mess than import...
    // we're going to ignore dbm, we'll treat export like it's making a new copy of the drive
    if (filename && bs && bs->fbm && bs->data_blocks) {
        const int fd = open(filename, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP);
        if (fd != -1) {
            if (utility_write_file(fd, bitmap_export(bs->fbm), FBM_SIZE * BLOCK_SIZE) == (FBM_SIZE * BLOCK_SIZE)) {
                if (utility_write_file(fd, bs->data_blocks, BLOCK_SIZE * (BLOCK_COUNT - FBM_SIZE)) == (BLOCK_SIZE * (BLOCK_COUNT - FBM_SIZE))) {
                    block_store_errno = BS_OK;
                    close(fd);
                    return BLOCK_SIZE * BLOCK_COUNT;
                }
            }
            block_store_errno = BS_FILE_IO;
            close(fd);
            return 0;
        }
        block_store_errno = BS_FILE_ACCESS;
        return 0;
    }
    block_store_errno = BS_PARAM;
    return 0;
}

const char *block_store_strerror(block_store_status bs_err) {
    switch (bs_err) {
        case BS_OK:
            return "Ok";
        case BS_PARAM:
            return "Parameter error";
        case BS_INTERNAL:
            return "Generic internal error";
        case BS_FULL:
            return "Device full";
        case BS_IN_USE:
            return "Block in use";
        case BS_NOT_IN_USE:
            return "Block not in use";
        case BS_FILE_ACCESS:
            return "Could not access file";
        case BS_FATAL:
            return "Generic fatal error";
        case BS_FILE_IO:
            return "Error during disk I/O";
        case BS_MEMORY:
            return "Memory allocation failure";
        case BS_WARN:
            return "Generic warning";
        case BS_FBM_REQUEST_MISMATCH:
            return "Read/write request to a block not marked in use";
        default:
            return "???";
    }
}


// V2 idea:
//  add an fd field to the struct (and have export(change name?) fill it out if it doesn't exist)
//  and use that for sync. When we sync, we only write the dirtied blocks
//  and once the FULL sync is complete, we format the dbm
//  So, if at any time, the connection to the file dies, we have the dbm saved so we can try again
//   but it's probably totally broken if the sync failed for whatever reason
//   I guess a new export will fix that?


size_t utility_read_file(const int fd, uint8_t *buffer, const size_t count) {
    // reads in the requested ammount of data
    // Attempts to read all at once, will attempt to retry and finish read if incomplete
    size_t have_read = 0, will_read = count;
    ssize_t data_read;
    do {
        data_read = read(fd, buffer + have_read, will_read);
        if (data_read == -1) {
            if (errno == EINTR) { continue; }
            return 0;
        }
        will_read -= data_read;
        have_read += data_read;
    } while (will_read);
    return have_read;
}

size_t utility_write_file(const int fd, const uint8_t *buffer, const size_t count) {
    size_t have_written = 0, will_write = count;
    ssize_t data_written;
    do {
        data_written = write(fd, buffer + have_written, will_write);
        if (data_written == -1) {
            if (errno == EINTR) { continue; }
            return 0;
        }
        will_write -= data_written;
        have_written += data_written;
    } while (will_write);
    return have_written;
}