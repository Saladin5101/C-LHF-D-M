#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "package_manager.h"

#define VERSION "1.0.0"

void print_help(void) {
    printf("C-LHF&D-M - C Language Header Files and Dependency Manager v%s\n", VERSION);
    printf("Usage: clhfm <command> [arguments]\n\n");
    printf("Commands:\n");
    printf("  install <package> [version]     - Install a package\n");
    printf("  remove <package>                - Remove a package\n");
    printf("  update [package]                - Update package(s) to latest version\n");
    printf("  push <package> <version>        - Push a custom dependency\n");
    printf("  tag <package> <tag> <version>   - Create a tag for a dependency\n");
    printf("  list                            - List all installed packages\n");
    printf("  info <package>                  - Show package information\n");
    printf("  config [get|set] [key] [value] - Manage configuration\n");
    printf("  help                            - Show this help message\n");
}

void print_config_help(void) {
    printf("\nConfiguration commands:\n");
    printf("  clhfm config show               - Display current configuration\n");
    printf("  clhfm config get <key>          - Get configuration value\n");
    printf("  clhfm config set <key> <value>  - Set configuration value\n");
    printf("  clhfm config init               - Initialize default configuration file\n");
    printf("\nConfiguration keys:\n");
    printf("  registry_url                    - Package registry URL\n");
    printf("  connection_timeout              - Connection timeout in seconds\n");
    printf("  max_concurrent_ops              - Maximum concurrent operations\n");
}

int main(int argc, char *argv[]) {
    int ret_code = 0;

    if (argc < 2) {
        print_help();
        return 0;
    }

    /* Initialize configuration system first */
    if (clhfm_config_create_default() != 0) {
        fprintf(stderr, "Warning: Could not initialize default configuration\n");
    }

    /* Initialize package manager */
    clhfm_manager_t *manager = clhfm_manager_create();
    if (!manager) {
        fprintf(stderr, "Error: Failed to initialize package manager\n");
        return 1;
    }

    /* Load configuration */
    if (manager->config) {
        if (clhfm_config_load_from_file(manager->config) != 0) {
            fprintf(stderr, "Warning: Could not load configuration file\n");
        }
    }

    /* Initialize directories */
    if (clhfm_manager_init(manager) != 0) {
        fprintf(stderr, "Error: Failed to initialize package manager directories\n");
        clhfm_manager_free(manager);
        return 1;
    }

    const char *command = argv[1];

    if (strcmp(command, "install") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: package name required\n");
            ret_code = 1;
        } else {
            const char *version = (argc > 3) ? argv[3] : "latest";
            if (clhfm_manager_install(manager, argv[2], version) != 0) {
                ret_code = 1;
            }
        }
    } else if (strcmp(command, "remove") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: package name required\n");
            ret_code = 1;
        } else {
            if (clhfm_manager_remove(manager, argv[2]) != 0) {
                ret_code = 1;
            }
        }
    } else if (strcmp(command, "update") == 0) {
        if (argc > 2) {
            /* Update specific package */
            if (clhfm_manager_update(manager, argv[2]) != 0) {
                ret_code = 1;
            }
        } else {
            /* Update all packages */
            if (clhfm_manager_update_all(manager) != 0) {
                ret_code = 1;
            }
        }
    } else if (strcmp(command, "push") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Error: package name and version required\n");
            ret_code = 1;
        } else {
            const char *description = (argc > 4) ? argv[4] : "";
            const char *url = (argc > 5) ? argv[5] : "";
            if (clhfm_manager_push_package(manager, argv[2], argv[3], description, url) != 0) {
                ret_code = 1;
            }
        }
    } else if (strcmp(command, "tag") == 0) {
        if (argc < 5) {
            fprintf(stderr, "Error: package name, tag, and version required\n");
            ret_code = 1;
        } else {
            if (clhfm_manager_create_tag(manager, argv[2], argv[3], argv[4]) != 0) {
                ret_code = 1;
            }
        }
    } else if (strcmp(command, "list") == 0) {
        int count = 0;
        char **packages = clhfm_manager_list_installed(manager, &count);
        if (packages && count > 0) {
            printf("=== Installed Packages ===\n");
            for (int i = 0; i < count; i++) {
                printf("  - %s\n", packages[i]);
                free(packages[i]);
            }
            free(packages);
        } else {
            printf("No packages installed\n");
        }
    } else if (strcmp(command, "info") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: package name required\n");
            ret_code = 1;
        } else {
            clhfm_package_t *pkg = clhfm_manager_get_package_info(manager, argv[2]);
            if (pkg) {
                printf("=== Package Information ===\n");
                printf("Name: %s\n", pkg->name);
                printf("Version: %s\n", pkg->version);
                if (pkg->description) {
                    printf("Description: %s\n", pkg->description);
                }
                printf("Installed: %s", ctime(&pkg->installed_time));
                if (pkg->dependency_count > 0) {
                    printf("Dependencies:\n");
                    for (int i = 0; i < pkg->dependency_count; i++) {
                        printf("  - %s (%s)\n", pkg->dependencies[i], pkg->dep_versions[i]);
                    }
                }
                clhfm_package_free(pkg);
            } else {
                fprintf(stderr, "Error: package not found\n");
                ret_code = 1;
            }
        }
    } else if (strcmp(command, "config") == 0) {
        /* Configuration management */
        if (argc < 3) {
            print_config_help();
            ret_code = 1;
        } else {
            const char *config_cmd = argv[2];
            
            if (strcmp(config_cmd, "show") == 0) {
                /* Show current configuration */
                clhfm_config_t *config = clhfm_config_create();
                if (config) {
                    clhfm_config_load_from_file(config);
                    printf("=== Current Configuration ===\n");
                    printf("Registry URL:        %s\n", config->registry_url);
                    printf("Timeout:             %d seconds\n", config->connection_timeout);
                    printf("Max Concurrent Ops:  %d\n", config->max_concurrent_ops);
                    printf("Config Directory:    %s\n", config->home_dir);
                    printf("Packages Directory:  %s\n", config->packages_dir);
                    printf("Cache Directory:     %s\n", config->cache_dir);
                    printf("Registry Directory:  %s\n", config->registry_dir);
                    clhfm_config_free(config);
                } else {
                    fprintf(stderr, "Error: Failed to load configuration\n");
                    ret_code = 1;
                }
            } else if (strcmp(config_cmd, "get") == 0) {
                /* Get specific config value */
                if (argc < 4) {
                    fprintf(stderr, "Error: key name required\n");
                    ret_code = 1;
                } else {
                    const char *key = argv[3];
                    clhfm_config_t *config = clhfm_config_create();
                    if (config) {
                        clhfm_config_load_from_file(config);
                        if (strcmp(key, "registry_url") == 0) {
                            printf("%s\n", config->registry_url);
                        } else if (strcmp(key, "connection_timeout") == 0) {
                            printf("%d\n", config->connection_timeout);
                        } else if (strcmp(key, "max_concurrent_ops") == 0) {
                            printf("%d\n", config->max_concurrent_ops);
                        } else {
                            fprintf(stderr, "Error: unknown configuration key\n");
                            ret_code = 1;
                        }
                        clhfm_config_free(config);
                    } else {
                        fprintf(stderr, "Error: Failed to load configuration\n");
                        ret_code = 1;
                    }
                }
            } else if (strcmp(config_cmd, "set") == 0) {
                /* Set config value */
                if (argc < 5) {
                    fprintf(stderr, "Error: key and value required\n");
                    ret_code = 1;
                } else {
                    const char *key = argv[3];
                    const char *value = argv[4];
                    clhfm_config_t *config = clhfm_config_create();
                    if (config) {
                        clhfm_config_load_from_file(config);
                        
                        if (strcmp(key, "registry_url") == 0) {
                            clhfm_config_set_registry_url(config, value);
                            printf("Registry URL set to: %s\n", value);
                        } else if (strcmp(key, "connection_timeout") == 0) {
                            int timeout = atoi(value);
                            if (clhfm_config_set_timeout(config, timeout) == 0) {
                                printf("Timeout set to: %d seconds\n", timeout);
                            } else {
                                fprintf(stderr, "Error: Invalid timeout value\n");
                                ret_code = 1;
                            }
                        } else if (strcmp(key, "max_concurrent_ops") == 0) {
                            int max_ops = atoi(value);
                            if (clhfm_config_set_max_concurrent(config, max_ops) == 0) {
                                printf("Max concurrent operations set to: %d\n", max_ops);
                            } else {
                                fprintf(stderr, "Error: Invalid max concurrent value\n");
                                ret_code = 1;
                            }
                        } else {
                            fprintf(stderr, "Error: unknown configuration key\n");
                            ret_code = 1;
                        }
                        
                        if (ret_code == 0 && clhfm_config_save_to_file(config) == 0) {
                            printf("Configuration saved\n");
                        }
                        
                        clhfm_config_free(config);
                    } else {
                        fprintf(stderr, "Error: Failed to load configuration\n");
                        ret_code = 1;
                    }
                }
            } else if (strcmp(config_cmd, "init") == 0) {
                /* Initialize default config */
                if (clhfm_config_create_default() == 0) {
                    printf("Default configuration initialized\n");
                } else {
                    fprintf(stderr, "Error: Failed to initialize configuration\n");
                    ret_code = 1;
                }
            } else {
                fprintf(stderr, "Error: unknown config command '%s'\n", config_cmd);
                print_config_help();
                ret_code = 1;
            }
        }
    } else if (strcmp(command, "help") == 0 || strcmp(command, "-h") == 0 || strcmp(command, "--help") == 0) {
        print_help();
    } else {
        fprintf(stderr, "Error: unknown command '%s'\n", command);
        printf("Use 'clhfm help' to see available commands\n");
        ret_code = 1;
    }

    clhfm_manager_free(manager);
    return ret_code;
}
