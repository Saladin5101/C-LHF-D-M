#ifndef CLHFM_PACKAGE_MANAGER_H
#define CLHFM_PACKAGE_MANAGER_H

#include "config.h"
#include "storage.h"

/**
 * @file package_manager.h
 * @brief Core package management engine with concurrent write support
 */

/* Package manager state */
typedef struct {
    clhfm_config_t *config;
    char *local_registry_path;
    char *packages_cache_path;
} clhfm_manager_t;

/* Dependency resolution status */
typedef struct {
    char **resolved_packages;
    int package_count;
    int resolution_ok;
    char *error_msg;
} clhfm_dep_resolution_t;

/* Manager initialization and cleanup */
clhfm_manager_t* clhfm_manager_create(void);
void clhfm_manager_free(clhfm_manager_t *manager);
int clhfm_manager_init(clhfm_manager_t *manager);

/* Core package operations - concurrent write safe */
int clhfm_manager_install(clhfm_manager_t *manager, const char *package_name, const char *version);
int clhfm_manager_remove(clhfm_manager_t *manager, const char *package_name);
int clhfm_manager_update(clhfm_manager_t *manager, const char *package_name);
int clhfm_manager_update_all(clhfm_manager_t *manager);

/* Query operations - read-only, highly concurrent */
clhfm_package_t* clhfm_manager_get_package_info(clhfm_manager_t *manager, const char *package_name);
int clhfm_manager_package_exists(clhfm_manager_t *manager, const char *package_name);
char** clhfm_manager_list_installed(clhfm_manager_t *manager, int *count);

/* Dependency resolution */
clhfm_dep_resolution_t* clhfm_resolve_dependencies(
    clhfm_manager_t *manager,
    const char *package_name,
    const char *version
);
void clhfm_dep_resolution_free(clhfm_dep_resolution_t *resolution);

/* Push custom package to registry */
int clhfm_manager_push_package(
    clhfm_manager_t *manager,
    const char *package_name,
    const char *version,
    const char *description,
    const char *source_url
);

/* Tag management */
int clhfm_manager_create_tag(
    clhfm_manager_t *manager,
    const char *package_name,
    const char *tag_name,
    const char *version
);

#endif /* CLHFM_PACKAGE_MANAGER_H */
