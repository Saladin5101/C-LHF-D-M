#ifndef CLHFM_NETWORK_H
#define CLHFM_NETWORK_H

#include "config.h"

/**
 * @file network.h
 * @brief HTTP networking layer using libcurl
 *
 * Registry API conventions (all relative to registry_url):
 *   GET  /packages/<name>/<version>.tar.gz  — download package archive
 *   GET  /packages/<name>/latest            — returns "VERSION\n"
 *   GET  /packages/<name>/info              — returns NAME|VERSION|DESC\n
 *   POST /packages/<name>/<version>         — upload package archive (push)
 *   POST /packages/<name>/tags/<tag>        — create tag
 */

/* Result of a download operation */
typedef struct {
    char  *data;    /* heap-allocated, NULL-terminated for text responses */
    size_t size;
} clhfm_http_buf_t;

/* Initialise / teardown (call once per process) */
void clhfm_network_init(void);
void clhfm_network_cleanup(void);

/**
 * Download a package archive from the registry and save it to dest_path.
 * Returns 0 on success, -1 on failure.
 */
int clhfm_network_download_package(
    const clhfm_config_t *config,
    const char *package_name,
    const char *version,
    const char *dest_path
);

/**
 * Query the latest version string for a package.
 * Caller must free() the returned string.
 * Returns NULL on failure.
 */
char *clhfm_network_fetch_latest_version(
    const clhfm_config_t *config,
    const char *package_name
);

/**
 * Upload a local archive to the registry (push).
 * Returns 0 on success, -1 on failure.
 */
int clhfm_network_push_package(
    const clhfm_config_t *config,
    const char *package_name,
    const char *version,
    const char *archive_path
);

/**
 * Create a tag on the registry for a package version.
 * Returns 0 on success, -1 on failure.
 */
int clhfm_network_create_tag(
    const clhfm_config_t *config,
    const char *package_name,
    const char *tag_name,
    const char *version
);

#endif /* CLHFM_NETWORK_H */
