#ifndef CLHFM_STORAGE_H
#define CLHFM_STORAGE_H

#include <time.h>

/**
 * @file storage.h
 * @brief Storage layer for package metadata using JSON
 */

/* Package metadata structure */
typedef struct {
    char *name;
    char *version;
    char *description;
    char *author;
    char **dependencies;        /* Array of dependency names */
    int dependency_count;
    char **dep_versions;        /* Corresponding versions */
    time_t installed_time;
    char *install_path;
    char *source_url;
} clhfm_package_t;

/* Registry entry for remote packages */
typedef struct {
    char *name;
    char *latest_version;
    char *description;
    char **available_versions;  /* All available versions */
    int version_count;
    char *download_url_template;
} clhfm_registry_entry_t;

/* Storage operations */
clhfm_package_t* clhfm_package_create(const char *name, const char *version);
void clhfm_package_free(clhfm_package_t *pkg);
void clhfm_package_add_dependency(clhfm_package_t *pkg, const char *dep_name, const char *dep_version);

/* File I/O with atomic writes */
int clhfm_package_save_atomic(const clhfm_package_t *pkg, const char *filepath);
clhfm_package_t* clhfm_package_load(const char *filepath);

/* Registry operations */
int clhfm_registry_save_atomic(const char *registry_dir);
char* clhfm_registry_load(const char *registry_dir);
int clhfm_registry_entry_exists(const char *registry_dir, const char *package_name);
clhfm_registry_entry_t* clhfm_registry_entry_get(const char *registry_dir, const char *package_name);

#endif /* CLHFM_STORAGE_H */
