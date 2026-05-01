#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include "storage.h"
#include "concurrency.h"

/**
 * Create a new package metadata structure
 */
clhfm_package_t* clhfm_package_create(const char *name, const char *version) {
    if (!name || !version) {
        return NULL;
    }

    clhfm_package_t *pkg = (clhfm_package_t *)malloc(sizeof(clhfm_package_t));
    if (!pkg) {
        return NULL;
    }

    pkg->name = (char *)malloc(strlen(name) + 1);
    strcpy(pkg->name, name);

    pkg->version = (char *)malloc(strlen(version) + 1);
    strcpy(pkg->version, version);

    pkg->description = NULL;
    pkg->author = NULL;
    pkg->dependencies = NULL;
    pkg->dependency_count = 0;
    pkg->dep_versions = NULL;
    pkg->installed_time = time(NULL);
    pkg->install_path = NULL;
    pkg->source_url = NULL;

    return pkg;
}

/**
 * Free package metadata
 */
void clhfm_package_free(clhfm_package_t *pkg) {
    if (!pkg) {
        return;
    }

    free(pkg->name);
    free(pkg->version);
    free(pkg->description);
    free(pkg->author);

    if (pkg->dependencies) {
        for (int i = 0; i < pkg->dependency_count; i++) {
            free(pkg->dependencies[i]);
            free(pkg->dep_versions[i]);
        }
        free(pkg->dependencies);
        free(pkg->dep_versions);
    }

    free(pkg->install_path);
    free(pkg->source_url);
    free(pkg);
}

/**
 * Add a dependency to package
 */
void clhfm_package_add_dependency(clhfm_package_t *pkg, const char *dep_name, const char *dep_version) {
    if (!pkg || !dep_name || !dep_version) {
        return;
    }

    /* Resize arrays */
    int new_count = pkg->dependency_count + 1;
    char **new_deps = (char **)realloc(pkg->dependencies, new_count * sizeof(char *));
    char **new_versions = (char **)realloc(pkg->dep_versions, new_count * sizeof(char *));

    if (!new_deps || !new_versions) {
        return;
    }

    pkg->dependencies = new_deps;
    pkg->dep_versions = new_versions;

    pkg->dependencies[new_count - 1] = (char *)malloc(strlen(dep_name) + 1);
    strcpy(pkg->dependencies[new_count - 1], dep_name);

    pkg->dep_versions[new_count - 1] = (char *)malloc(strlen(dep_version) + 1);
    strcpy(pkg->dep_versions[new_count - 1], dep_version);

    pkg->dependency_count = new_count;
}

/**
 * Simple JSON serialization for package metadata
 * Format: name|version|description|author|dep1:ver1,dep2:ver2|installed_time
 */
static char* clhfm_package_to_string(const clhfm_package_t *pkg) {
    if (!pkg) {
        return NULL;
    }

    /* Calculate buffer size */
    size_t size = 256; /* Base size */
    size += strlen(pkg->name) + strlen(pkg->version);
    if (pkg->description) size += strlen(pkg->description);
    if (pkg->author) size += strlen(pkg->author);
    size += pkg->dependency_count * 64; /* Rough estimate for dependencies */

    char *buffer = (char *)malloc(size);
    if (!buffer) {
        return NULL;
    }

    /* Build simple format */
    char *pos = buffer;
    pos += sprintf(pos, "NAME=%s\n", pkg->name);
    pos += sprintf(pos, "VERSION=%s\n", pkg->version);
    if (pkg->description) {
        pos += sprintf(pos, "DESCRIPTION=%s\n", pkg->description);
    }
    if (pkg->author) {
        pos += sprintf(pos, "AUTHOR=%s\n", pkg->author);
    }
    pos += sprintf(pos, "INSTALLED=%ld\n", pkg->installed_time);

    if (pkg->dependency_count > 0) {
        pos += sprintf(pos, "DEPENDENCIES=");
        for (int i = 0; i < pkg->dependency_count; i++) {
            if (i > 0) pos += sprintf(pos, ",");
            pos += sprintf(pos, "%s:%s", pkg->dependencies[i], pkg->dep_versions[i]);
        }
        pos += sprintf(pos, "\n");
    }

    return buffer;
}

/**
 * Save package metadata with atomic write operation
 */
int clhfm_package_save_atomic(const clhfm_package_t *pkg, const char *filepath) {
    if (!pkg || !filepath) {
        return -1;
    }

    char *content = clhfm_package_to_string(pkg);
    if (!content) {
        return -1;
    }

    int ret = clhfm_atomic_write_file(filepath, content, strlen(content));
    free(content);

    return ret;
}

/**
 * Load package metadata from file
 */
clhfm_package_t* clhfm_package_load(const char *filepath) {
    if (!filepath) {
        return NULL;
    }

    FILE *f = fopen(filepath, "r");
    if (!f) {
        return NULL;
    }

    clhfm_package_t *pkg = NULL;
    char line[512];
    char name[128] = {0};
    char version[128] = {0};

    while (fgets(line, sizeof(line), f)) {
        /* Remove newline */
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }

        if (strncmp(line, "NAME=", 5) == 0) {
            strncpy(name, line + 5, sizeof(name) - 1);
        } else if (strncmp(line, "VERSION=", 8) == 0) {
            strncpy(version, line + 8, sizeof(version) - 1);
        }
    }

    fclose(f);

    if (name[0] && version[0]) {
        pkg = clhfm_package_create(name, version);
    }

    return pkg;
}

/**
 * Save registry (simplified)
 */
int clhfm_registry_save_atomic(const char *registry_dir) {
    if (!registry_dir) {
        return -1;
    }
    /* TODO: Implement registry persistence */
    return 0;
}

/**
 * Load registry
 */
char* clhfm_registry_load(const char *registry_dir) {
    if (!registry_dir) {
        return NULL;
    }
    /* TODO: Implement registry loading */
    return NULL;
}

/**
 * Check if registry entry exists
 */
int clhfm_registry_entry_exists(const char *registry_dir, const char *package_name) {
    if (!registry_dir || !package_name) {
        return 0;
    }
    /* TODO: Implement entry existence check */
    return 0;
}

/**
 * Get registry entry
 */
clhfm_registry_entry_t* clhfm_registry_entry_get(const char *registry_dir, const char *package_name) {
    if (!registry_dir || !package_name) {
        return NULL;
    }
    /* TODO: Implement registry entry retrieval */
    return NULL;
}
