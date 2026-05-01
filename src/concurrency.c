#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include "concurrency.h"

/**
 * Atomic increment operation (no lock required)
 */
uint32_t clhfm_atomic_increment(volatile uint32_t *counter) {
    if (!counter) {
        return 0;
    }
    return __sync_fetch_and_add(counter, 1) + 1;
}

/**
 * Atomic decrement operation (no lock required)
 */
uint32_t clhfm_atomic_decrement(volatile uint32_t *counter) {
    if (!counter) {
        return 0;
    }
    return __sync_fetch_and_sub(counter, 1) - 1;
}

/**
 * Compare-and-swap operation for lock-free algorithms
 */
int clhfm_atomic_compare_swap(volatile uint32_t *dest, uint32_t expected, uint32_t new_val) {
    if (!dest) {
        return 0;
    }
    return __sync_bool_compare_and_swap(dest, expected, new_val);
}

/**
 * Atomic write to file using temp file + rename pattern
 * This avoids locks and allows concurrent reads while write completes
 */
int clhfm_atomic_write_file(const char *filepath, const void *data, size_t size) {
    if (!filepath || !data) {
        return -1;
    }

    /* Create temporary file path */
    char *temp_path = clhfm_temp_path(filepath);
    if (!temp_path) {
        return -1;
    }

    /* Write to temporary file */
    int fd = open(temp_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        fprintf(stderr, "Error opening temp file: %s\n", temp_path);
        free(temp_path);
        return -1;
    }

    /* Write all data */
    ssize_t written = 0;
    while (written < (ssize_t)size) {
        ssize_t ret = write(fd, (char *)data + written, size - written);
        if (ret < 0) {
            fprintf(stderr, "Error writing to file: %s\n", temp_path);
            close(fd);
            unlink(temp_path);
            free(temp_path);
            return -1;
        }
        written += ret;
    }

    /* Fsync to ensure data is written to disk */
    if (fsync(fd) != 0) {
        fprintf(stderr, "Error syncing file: %s\n", temp_path);
        close(fd);
        unlink(temp_path);
        free(temp_path);
        return -1;
    }

    close(fd);

    /* Atomic rename - this is the key operation for lock-free writes */
    if (rename(temp_path, filepath) != 0) {
        fprintf(stderr, "Error renaming temp file to: %s\n", filepath);
        unlink(temp_path);
        free(temp_path);
        return -1;
    }

    free(temp_path);
    return 0;
}

/**
 * Atomic file rename operation
 */
int clhfm_atomic_rename(const char *old_path, const char *new_path) {
    if (!old_path || !new_path) {
        return -1;
    }

    if (rename(old_path, new_path) != 0) {
        fprintf(stderr, "Error renaming %s to %s\n", old_path, new_path);
        return -1;
    }

    return 0;
}

/**
 * Generate a unique temporary file path using pid and timestamp
 */
char* clhfm_temp_path(const char *base_path) {
    if (!base_path) {
        return NULL;
    }

    /* Generate suffix with PID and current time */
    char suffix[32];
    snprintf(suffix, sizeof(suffix), ".tmp.%d.%ld", getpid(), time(NULL));

    size_t len = strlen(base_path) + strlen(suffix) + 1;
    char *temp_path = (char *)malloc(len);
    if (!temp_path) {
        return NULL;
    }

    snprintf(temp_path, len, "%s%s", base_path, suffix);
    return temp_path;
}

/**
 * Free operation result
 */
void clhfm_op_result_free(clhfm_op_result_t *result) {
    if (!result) {
        return;
    }
    if (result->error_msg) {
        free(result->error_msg);
    }
    free(result);
}
