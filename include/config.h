#ifndef CLHFM_CONFIG_H
#define CLHFM_CONFIG_H

#include <time.h>

/**
 * @file config.h
 * @brief Configuration management for C-LHF&D-M
 */

/* Default paths */
#define CLHFM_HOME_DIR ".clhfm"
#define CLHFM_PACKAGES_DIR "packages"
#define CLHFM_CACHE_DIR "cache"
#define CLHFM_REGISTRY_DIR "registry"
#define CLHFM_LOCK_DIR "locks"
#define CLHFM_REGISTRY_FILE "registry.json"

/* Registry server (placeholder - can be configured) */
#define CLHFM_DEFAULT_REGISTRY "https://saladin510-orangeyouhome-erath.online"

/* Configuration structure */
typedef struct {
    char *home_dir;
    char *packages_dir;
    char *cache_dir;
    char *registry_dir;
    char *registry_url;
    int max_concurrent_ops;
    int connection_timeout;
} clhfm_config_t;

/* Configuration functions */
clhfm_config_t* clhfm_config_create(void);
void clhfm_config_free(clhfm_config_t *config);
int clhfm_config_init(clhfm_config_t *config);
char* clhfm_get_config_path(void);

/* Configuration file functions */
#define CLHFM_CONFIG_FILE "config.ini"
int clhfm_config_load_from_file(clhfm_config_t *config);
int clhfm_config_save_to_file(const clhfm_config_t *config);
int clhfm_config_create_default(void);
char* clhfm_config_get_file_path(void);

/* Configuration utility functions */
int clhfm_config_set_registry_url(clhfm_config_t *config, const char *url);
int clhfm_config_set_timeout(clhfm_config_t *config, int timeout);
int clhfm_config_set_max_concurrent(clhfm_config_t *config, int max_ops);

#endif /* CLHFM_CONFIG_H */
