#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <errno.h>
#include "package_manager.h"
#include "concurrency.h"
#include "network.h"

/**
 * Create a new package manager instance
 */
clhfm_manager_t* clhfm_manager_create(void) {
    clhfm_manager_t *manager = (clhfm_manager_t *)malloc(sizeof(clhfm_manager_t));
    if (!manager) {
        return NULL;
    }

    manager->config = clhfm_config_create();
    if (!manager->config) {
        free(manager);
        return NULL;
    }

    manager->local_registry_path = (char *)malloc(strlen(manager->config->registry_dir) + 1);
    strcpy(manager->local_registry_path, manager->config->registry_dir);

    manager->packages_cache_path = (char *)malloc(strlen(manager->config->packages_dir) + 1);
    strcpy(manager->packages_cache_path, manager->config->packages_dir);

    return manager;
}

/**
 * Free package manager
 */
void clhfm_manager_free(clhfm_manager_t *manager) {
    if (!manager) {
        return;
    }

    if (manager->config) {
        clhfm_config_free(manager->config);
    }

    free(manager->local_registry_path);
    free(manager->packages_cache_path);
    free(manager);
}

/**
 * Initialize package manager - create necessary directories
 */
int clhfm_manager_init(clhfm_manager_t *manager) {
    if (!manager || !manager->config) {
        return -1;
    }

    return clhfm_config_init(manager->config);
}

/**
 * Install a package with version
 */
int clhfm_manager_install(clhfm_manager_t *manager, const char *package_name, const char *version) {
    if (!manager || !package_name || !version) {
        return -1;
    }

    /* Resolve "latest" to a real version number */
    char *resolved_version = NULL;
    if (strcmp(version, "latest") == 0) {
        printf("Fetching latest version for %s...\n", package_name);
        resolved_version = clhfm_network_fetch_latest_version(manager->config, package_name);
        if (!resolved_version) {
            fprintf(stderr, "Error: could not determine latest version of %s\n", package_name);
            return -1;
        }
        version = resolved_version;
    }

    printf("Installing package: %s@%s\n", package_name, version);

    /* Download archive to cache dir */
    char archive_path[512];
    snprintf(archive_path, sizeof(archive_path), "%s/%s-%s.tar.gz",
             manager->config->cache_dir, package_name, version);

    if (clhfm_network_download_package(manager->config, package_name, version, archive_path) != 0) {
        free(resolved_version);
        return -1;
    }

    /* Extract archive into packages dir */
    char install_dir[512];
    snprintf(install_dir, sizeof(install_dir), "%s/%s", manager->packages_cache_path, package_name);
    if (mkdir(install_dir, 0755) != 0 && errno != EEXIST) {
        fprintf(stderr, "Error creating install directory: %s\n", install_dir);
        free(resolved_version);
        return -1;
    }

    char tar_cmd[1024];
    snprintf(tar_cmd, sizeof(tar_cmd), "tar -xzf \"%s\" -C \"%s\" 2>&1", archive_path, install_dir);
    if (system(tar_cmd) != 0) {
        fprintf(stderr, "Error extracting archive for %s\n", package_name);
        free(resolved_version);
        return -1;
    }

    /* Save package metadata */
    clhfm_package_t *pkg = clhfm_package_create(package_name, version);
    if (!pkg) {
        free(resolved_version);
        return -1;
    }
    pkg->install_path = (char *)malloc(strlen(install_dir) + 1);
    strcpy(pkg->install_path, install_dir);

    char pkg_path[512];
    snprintf(pkg_path, sizeof(pkg_path), "%s/%s.pkg", manager->packages_cache_path, package_name);
    int ret = clhfm_package_save_atomic(pkg, pkg_path);
    clhfm_package_free(pkg);
    free(resolved_version);

    if (ret != 0) {
        fprintf(stderr, "Error saving package metadata\n");
        return -1;
    }

    printf("Successfully installed: %s@%s\n", package_name, version);
    return 0;
}

/**
 * Remove a package
 */
int clhfm_manager_remove(clhfm_manager_t *manager, const char *package_name) {
    if (!manager || !package_name) {
        return -1;
    }

    printf("Removing package: %s\n", package_name);

    /* Build package file path */
    char pkg_path[512];
    snprintf(pkg_path, sizeof(pkg_path), "%s/%s.pkg", manager->packages_cache_path, package_name);

    /* Check if package exists */
    struct stat st;
    if (stat(pkg_path, &st) != 0) {
        fprintf(stderr, "Package not found: %s\n", package_name);
        return -1;
    }

    /* Remove the package file */
    if (unlink(pkg_path) != 0) {
        fprintf(stderr, "Error removing package: %s\n", package_name);
        return -1;
    }

    printf("Successfully removed: %s\n", package_name);
    return 0;
}

/**
 * Update a specific package
 */
int clhfm_manager_update(clhfm_manager_t *manager, const char *package_name) {
    if (!manager || !package_name) {
        return -1;
    }

    printf("Checking for updates: %s\n", package_name);

    /* Load current package */
    char pkg_path[512];
    snprintf(pkg_path, sizeof(pkg_path), "%s/%s.pkg", manager->packages_cache_path, package_name);

    clhfm_package_t *pkg = clhfm_package_load(pkg_path);
    if (!pkg) {
        fprintf(stderr, "Package not found: %s\n", package_name);
        return -1;
    }

    /* Query registry for the latest version */
    char *latest = clhfm_network_fetch_latest_version(manager->config, package_name);
    if (!latest) {
        fprintf(stderr, "Could not fetch latest version info for %s\n", package_name);
        clhfm_package_free(pkg);
        return -1;
    }

    if (strcmp(latest, pkg->version) == 0) {
        printf("%s is already at the latest version (%s)\n", package_name, pkg->version);
        free(latest);
        clhfm_package_free(pkg);
        return 0;
    }

    printf("Updating %s: %s -> %s\n", package_name, pkg->version, latest);
    clhfm_package_free(pkg);

    int ret = clhfm_manager_install(manager, package_name, latest);
    free(latest);
    return ret;
}

/**
 * Update all installed packages
 */
int clhfm_manager_update_all(clhfm_manager_t *manager) {
    if (!manager) {
        return -1;
    }

    printf("Updating all packages...\n");

    /* Get list of installed packages */
    int count = 0;
    char **packages = clhfm_manager_list_installed(manager, &count);

    if (!packages || count == 0) {
        printf("No packages to update\n");
        return 0;
    }

    /* Update each package */
    for (int i = 0; i < count; i++) {
        clhfm_manager_update(manager, packages[i]);
    }

    /* Cleanup */
    for (int i = 0; i < count; i++) {
        free(packages[i]);
    }
    free(packages);

    return 0;
}

/**
 * Get package information
 */
clhfm_package_t* clhfm_manager_get_package_info(clhfm_manager_t *manager, const char *package_name) {
    if (!manager || !package_name) {
        return NULL;
    }

    char pkg_path[512];
    snprintf(pkg_path, sizeof(pkg_path), "%s/%s.pkg", manager->packages_cache_path, package_name);

    return clhfm_package_load(pkg_path);
}

/**
 * Check if package is installed
 */
int clhfm_manager_package_exists(clhfm_manager_t *manager, const char *package_name) {
    if (!manager || !package_name) {
        return 0;
    }

    char pkg_path[512];
    snprintf(pkg_path, sizeof(pkg_path), "%s/%s.pkg", manager->packages_cache_path, package_name);

    struct stat st;
    return (stat(pkg_path, &st) == 0) ? 1 : 0;
}

/**
 * List all installed packages
 */
char** clhfm_manager_list_installed(clhfm_manager_t *manager, int *count) {
    if (!manager || !count) {
        return NULL;
    }

    *count = 0;

    DIR *dir = opendir(manager->packages_cache_path);
    if (!dir) {
        fprintf(stderr, "Error opening packages directory\n");
        return NULL;
    }

    /* First pass: count packages */
    struct dirent *entry;
    int pkg_count = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, ".pkg")) {
            pkg_count++;
        }
    }

    if (pkg_count == 0) {
        closedir(dir);
        return NULL;
    }

    /* Allocate array */
    char **packages = (char **)malloc(pkg_count * sizeof(char *));
    if (!packages) {
        closedir(dir);
        return NULL;
    }

    /* Second pass: collect package names */
    rewinddir(dir);
    int idx = 0;
    while ((entry = readdir(dir)) != NULL && idx < pkg_count) {
        char *pkg_ext = strstr(entry->d_name, ".pkg");
        if (pkg_ext) {
            int name_len = pkg_ext - entry->d_name;
            packages[idx] = (char *)malloc(name_len + 1);
            strncpy(packages[idx], entry->d_name, name_len);
            packages[idx][name_len] = '\0';
            idx++;
        }
    }

    closedir(dir);
    *count = idx;
    return packages;
}

/**
 * Resolve dependencies
 */
clhfm_dep_resolution_t* clhfm_resolve_dependencies(
    clhfm_manager_t *manager,
    const char *package_name,
    const char *version) {

    if (!manager || !package_name || !version) {
        return NULL;
    }

    clhfm_dep_resolution_t *resolution = (clhfm_dep_resolution_t *)malloc(sizeof(clhfm_dep_resolution_t));
    if (!resolution) {
        return NULL;
    }

    /* Load package and check dependencies */
    clhfm_package_t *pkg = clhfm_manager_get_package_info(manager, package_name);
    
    if (!pkg) {
        resolution->resolved_packages = NULL;
        resolution->package_count = 0;
        resolution->resolution_ok = 0;
        resolution->error_msg = (char *)malloc(32);
        strcpy(resolution->error_msg, "Package not found");
        return resolution;
    }

    /* TODO: Implement full dependency resolution with version checking */
    resolution->package_count = pkg->dependency_count;
    resolution->resolved_packages = pkg->dependencies;
    resolution->resolution_ok = 1;
    resolution->error_msg = NULL;

    clhfm_package_free(pkg);
    return resolution;
}

/**
 * Free dependency resolution result
 */
void clhfm_dep_resolution_free(clhfm_dep_resolution_t *resolution) {
    if (!resolution) {
        return;
    }

    if (resolution->resolved_packages) {
        free(resolution->resolved_packages);
    }
    if (resolution->error_msg) {
        free(resolution->error_msg);
    }
    free(resolution);
}

/**
 * Push a custom package to registry
 */
int clhfm_manager_push_package(
    clhfm_manager_t *manager,
    const char *package_name,
    const char *version,
    const char *description,
    const char *source_url) {

    if (!manager || !package_name || !version) {
        return -1;
    }

    printf("Pushing package: %s@%s\n", package_name, version);

    /* source_url is treated as the local archive path to upload */
    if (!source_url || source_url[0] == '\0') {
        fprintf(stderr, "Error: provide the local archive path as the source argument\n");
        return -1;
    }

    return clhfm_network_push_package(manager->config, package_name, version, source_url);
}

/**
 * Create a tag for a package
 */
int clhfm_manager_create_tag(
    clhfm_manager_t *manager,
    const char *package_name,
    const char *tag_name,
    const char *version) {

    if (!manager || !package_name || !tag_name || !version) {
        return -1;
    }

    printf("Creating tag '%s' for %s@%s\n", tag_name, package_name, version);
    return clhfm_network_create_tag(manager->config, package_name, tag_name, version);
}
