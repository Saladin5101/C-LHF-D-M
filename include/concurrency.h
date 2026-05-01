#ifndef CLHFM_CONCURRENCY_H
#define CLHFM_CONCURRENCY_H

#include <stdint.h>

/**
 * @file concurrency.h
 * @brief Concurrency utilities with lock-free operations
 * 
 * This module implements atomic operations and concurrent-write
 * capabilities without mutex locks to avoid APT-like blocking.
 */

/* Atomic reference counter for copy-on-write semantics */
typedef struct {
    volatile uint32_t count;
} clhfm_atomic_counter_t;

/* Operation result with version info for MVCC */
typedef struct {
    int success;
    uint64_t version;
    char *error_msg;
} clhfm_op_result_t;

/* Atomic operations */
uint32_t clhfm_atomic_increment(volatile uint32_t *counter);
uint32_t clhfm_atomic_decrement(volatile uint32_t *counter);
int clhfm_atomic_compare_swap(volatile uint32_t *dest, uint32_t expected, uint32_t new_val);

/* File system operations with atomic semantics */
int clhfm_atomic_write_file(const char *filepath, const void *data, size_t size);
int clhfm_atomic_rename(const char *old_path, const char *new_path);

/* Generate atomic write-friendly temporary file path */
char* clhfm_temp_path(const char *base_path);

/* Resource cleanup */
void clhfm_op_result_free(clhfm_op_result_t *result);

#endif /* CLHFM_CONCURRENCY_H */
