#ifndef CLHFM_H
#define CLHFM_H

/**
 * @file clhfm.h
 * @brief Main header file for C-LHF&D-M package manager
 */

/* Package management functions */
void clhfm_install(const char *package_name);
void clhfm_remove(const char *package_name);
void clhfm_update(void);
void clhfm_push(const char *package_name);
void clhfm_tag(const char *package_name, const char *tag);

#endif /* CLHFM_H */
