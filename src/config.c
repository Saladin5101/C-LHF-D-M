#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>
#include <errno.h>
#include "config.h"

/**
 * Create a new configuration object with default values
 */
clhfm_config_t* clhfm_config_create(void) {
    clhfm_config_t *config = (clhfm_config_t *)malloc(sizeof(clhfm_config_t));
    if (!config) {
        return NULL;
    }

    /* Initialize with defaults */
    config->max_concurrent_ops = 32;
    config->connection_timeout = 30;
    config->registry_url = (char *)malloc(strlen(CLHFM_DEFAULT_REGISTRY) + 1);
    strcpy(config->registry_url, CLHFM_DEFAULT_REGISTRY);

    /* Set up directory paths relative to home */
    char *home = getenv("HOME");
    if (!home) {
        struct passwd *pw = getpwuid(getuid());
        if (pw) {
            home = pw->pw_dir;
        } else {
            home = "/tmp";
        }
    }

    size_t home_len = strlen(home);
    
    /* Create home directory path */
    config->home_dir = (char *)malloc(home_len + strlen(CLHFM_HOME_DIR) + 2);
    sprintf(config->home_dir, "%s/%s", home, CLHFM_HOME_DIR);

    /* Create packages directory path */
    config->packages_dir = (char *)malloc(strlen(config->home_dir) + strlen(CLHFM_PACKAGES_DIR) + 2);
    sprintf(config->packages_dir, "%s/%s", config->home_dir, CLHFM_PACKAGES_DIR);

    /* Create cache directory path */
    config->cache_dir = (char *)malloc(strlen(config->home_dir) + strlen(CLHFM_CACHE_DIR) + 2);
    sprintf(config->cache_dir, "%s/%s", config->home_dir, CLHFM_CACHE_DIR);

    /* Create registry directory path */
    config->registry_dir = (char *)malloc(strlen(config->home_dir) + strlen(CLHFM_REGISTRY_DIR) + 2);
    sprintf(config->registry_dir, "%s/%s", config->home_dir, CLHFM_REGISTRY_DIR);

    return config;
}

/**
 * Free configuration object
 */
void clhfm_config_free(clhfm_config_t *config) {
    if (!config) {
        return;
    }
    free(config->home_dir);
    free(config->packages_dir);
    free(config->cache_dir);
    free(config->registry_dir);
    free(config->registry_url);
    free(config);
}

/**
 * Initialize configuration - create directories if needed
 */
int clhfm_config_init(clhfm_config_t *config) {
    if (!config) {
        return -1;
    }

    /* Create main home directory */
    if (mkdir(config->home_dir, 0755) != 0 && errno != EEXIST) {
        fprintf(stderr, "Error creating config directory: %s\n", config->home_dir);
        return -1;
    }

    /* Create subdirectories */
    const char *dirs[] = {
        config->packages_dir,
        config->cache_dir,
        config->registry_dir
    };

    for (int i = 0; i < 3; i++) {
        if (mkdir(dirs[i], 0755) != 0 && errno != EEXIST) {
            fprintf(stderr, "Error creating directory: %s\n", dirs[i]);
            return -1;
        }
    }

    return 0;
}

/**
 * Get the configuration directory path
 */
char* clhfm_get_config_path(void) {
    char *home = getenv("HOME");
    if (!home) {
        struct passwd *pw = getpwuid(getuid());
        if (pw) {
            home = pw->pw_dir;
        } else {
            home = "/tmp";
        }
    }

    char *config_path = (char *)malloc(strlen(home) + strlen(CLHFM_HOME_DIR) + 2);
    sprintf(config_path, "%s/%s", home, CLHFM_HOME_DIR);
    return config_path;
}

/**
 * Get configuration file path
 */
char* clhfm_config_get_file_path(void) {
    char *config_dir = clhfm_get_config_path();
    if (!config_dir) {
        return NULL;
    }

    char *config_file = (char *)malloc(strlen(config_dir) + strlen(CLHFM_CONFIG_FILE) + 2);
    sprintf(config_file, "%s/%s", config_dir, CLHFM_CONFIG_FILE);
    free(config_dir);

    return config_file;
}

/**
 * Parse a configuration line in format: KEY=VALUE
 */
static void clhfm_config_parse_line(clhfm_config_t *config, const char *line) {
    if (!line || line[0] == '\0' || line[0] == '#' || line[0] == ';') {
        return;  /* Empty line or comment */
    }

    /* Find the equals sign */
    char *eq = strchr(line, '=');
    if (!eq) {
        return;  /* Invalid line format */
    }

    /* Extract key and value */
    int key_len = eq - line;
    char key[256] = {0};
    strncpy(key, line, key_len);

    const char *value = eq + 1;

    /* Trim whitespace from key */
    while (key_len > 0 && (key[key_len - 1] == ' ' || key[key_len - 1] == '\t')) {
        key[--key_len] = '\0';
    }

    /* Process key-value pairs */
    if (strcmp(key, "registry_url") == 0) {
        free(config->registry_url);
        config->registry_url = (char *)malloc(strlen(value) + 1);
        strcpy(config->registry_url, value);
    } else if (strcmp(key, "max_concurrent_ops") == 0) {
        config->max_concurrent_ops = atoi(value);
    } else if (strcmp(key, "connection_timeout") == 0) {
        config->connection_timeout = atoi(value);
    }
}

/**
 * Load configuration from file (INI format)
 */
int clhfm_config_load_from_file(clhfm_config_t *config) {
    if (!config) {
        return -1;
    }

    char *config_file = clhfm_config_get_file_path();
    if (!config_file) {
        return -1;
    }

    FILE *f = fopen(config_file, "r");
    if (!f) {
        /* File doesn't exist yet, use defaults */
        free(config_file);
        return 0;
    }

    char line[512];
    while (fgets(line, sizeof(line), f)) {
        /* Remove newline */
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }

        clhfm_config_parse_line(config, line);
    }

    fclose(config_file);
    free(config_file);

    return 0;
}

/**
 * Save configuration to file (INI format)
 */
int clhfm_config_save_to_file(const clhfm_config_t *config) {
    if (!config) {
        return -1;
    }

    char *config_file = clhfm_config_get_file_path();
    if (!config_file) {
        return -1;
    }

    FILE *f = fopen(config_file, "w");
    if (!f) {
        fprintf(stderr, "Error opening config file for writing: %s\n", config_file);
        free(config_file);
        return -1;
    }

    /* Write header */
    fprintf(f, "# C-LHF&D-M Configuration File\n");
    fprintf(f, "# Generated automatically\n\n");

    /* Write configuration values */
    fprintf(f, "[registry]\n");
    fprintf(f, "registry_url=%s\n", config->registry_url);
    fprintf(f, "connection_timeout=%d\n\n", config->connection_timeout);

    fprintf(f, "[performance]\n");
    fprintf(f, "max_concurrent_ops=%d\n\n", config->max_concurrent_ops);

    fprintf(f, "[directories]\n");
    fprintf(f, "packages_dir=%s\n", config->packages_dir);
    fprintf(f, "cache_dir=%s\n", config->cache_dir);
    fprintf(f, "registry_dir=%s\n", config->registry_dir);

    fclose(f);
    free(config_file);

    return 0;
}

/**
 * Create default configuration file
 */
int clhfm_config_create_default(void) {
    char *config_dir = clhfm_get_config_path();
    if (!config_dir) {
        return -1;
    }

    /* Ensure directory exists */
    if (mkdir(config_dir, 0755) != 0 && errno != EEXIST) {
        fprintf(stderr, "Error creating config directory: %s\n", config_dir);
        free(config_dir);
        return -1;
    }

    char *config_file = clhfm_config_get_file_path();
    if (!config_file) {
        free(config_dir);
        return -1;
    }

    /* Check if config already exists */
    struct stat st;
    if (stat(config_file, &st) == 0) {
        free(config_dir);
        free(config_file);
        return 0;  /* Config already exists */
    }

    /* Create default configuration */
    clhfm_config_t *config = clhfm_config_create();
    if (!config) {
        free(config_dir);
        free(config_file);
        return -1;
    }

    int ret = clhfm_config_save_to_file(config);
    clhfm_config_free(config);
    free(config_dir);
    free(config_file);

    if (ret == 0) {
        printf("Created default configuration file\n");
    }

    return ret;
}

/**
 * Set registry URL in configuration
 */
int clhfm_config_set_registry_url(clhfm_config_t *config, const char *url) {
    if (!config || !url) {
        return -1;
    }

    free(config->registry_url);
    config->registry_url = (char *)malloc(strlen(url) + 1);
    if (!config->registry_url) {
        return -1;
    }

    strcpy(config->registry_url, url);
    return 0;
}

/**
 * Set connection timeout
 */
int clhfm_config_set_timeout(clhfm_config_t *config, int timeout) {
    if (!config || timeout <= 0) {
        return -1;
    }

    config->connection_timeout = timeout;
    return 0;
}

/**
 * Set maximum concurrent operations
 */
int clhfm_config_set_max_concurrent(clhfm_config_t *config, int max_ops) {
    if (!config || max_ops <= 0) {
        return -1;
    }

    config->max_concurrent_ops = max_ops;
    return 0;
}
