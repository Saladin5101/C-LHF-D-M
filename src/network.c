#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "network.h"

/* ------------------------------------------------------------------ */
/* Internal helpers                                                     */
/* ------------------------------------------------------------------ */

/* libcurl write callback — appends received bytes to clhfm_http_buf_t */
static size_t write_to_buf(void *ptr, size_t size, size_t nmemb, void *userdata) {
    size_t bytes = size * nmemb;
    clhfm_http_buf_t *buf = (clhfm_http_buf_t *)userdata;

    char *tmp = realloc(buf->data, buf->size + bytes + 1);
    if (!tmp) return 0;

    buf->data = tmp;
    memcpy(buf->data + buf->size, ptr, bytes);
    buf->size += bytes;
    buf->data[buf->size] = '\0';
    return bytes;
}

/* libcurl write callback — writes directly to a FILE* */
static size_t write_to_file(void *ptr, size_t size, size_t nmemb, void *userdata) {
    return fwrite(ptr, size, nmemb, (FILE *)userdata);
}

/* Build a URL from base + path, caller must free() result */
static char *build_url(const char *base, const char *path) {
    /* strip trailing slash from base */
    size_t blen = strlen(base);
    while (blen > 0 && base[blen - 1] == '/') blen--;

    char *url = malloc(blen + strlen(path) + 2);
    if (!url) return NULL;
    memcpy(url, base, blen);
    url[blen] = '/';
    strcpy(url + blen + 1, path);
    return url;
}

/* Apply common curl options (timeout, TLS verification) */
static void apply_common_opts(CURL *curl, const clhfm_config_t *config) {
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, (long)config->connection_timeout);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "clhfm/1.0");
}

/* ------------------------------------------------------------------ */
/* Public API                                                           */
/* ------------------------------------------------------------------ */

void clhfm_network_init(void) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

void clhfm_network_cleanup(void) {
    curl_global_cleanup();
}

int clhfm_network_download_package(
    const clhfm_config_t *config,
    const char *package_name,
    const char *version,
    const char *dest_path)
{
    if (!config || !package_name || !version || !dest_path) return -1;

    /* Build URL: <registry>/packages/<name>/<version>.tar.gz */
    char path[512];
    snprintf(path, sizeof(path), "packages/%s/%s.tar.gz", package_name, version);
    char *url = build_url(config->registry_url, path);
    if (!url) return -1;

    FILE *fp = fopen(dest_path, "wb");
    if (!fp) {
        fprintf(stderr, "Error: cannot open %s for writing\n", dest_path);
        free(url);
        return -1;
    }

    CURL *curl = curl_easy_init();
    if (!curl) {
        fclose(fp);
        free(url);
        return -1;
    }

    apply_common_opts(curl, config);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_file);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    curl_easy_cleanup(curl);
    fclose(fp);
    free(url);

    if (res != CURLE_OK) {
        fprintf(stderr, "Network error: %s\n", curl_easy_strerror(res));
        remove(dest_path);
        return -1;
    }
    if (http_code != 200) {
        fprintf(stderr, "Server returned HTTP %ld for %s@%s\n", http_code, package_name, version);
        remove(dest_path);
        return -1;
    }
    return 0;
}

char *clhfm_network_fetch_latest_version(
    const clhfm_config_t *config,
    const char *package_name)
{
    if (!config || !package_name) return NULL;

    char path[512];
    snprintf(path, sizeof(path), "packages/%s/latest", package_name);
    char *url = build_url(config->registry_url, path);
    if (!url) return NULL;

    clhfm_http_buf_t buf = {NULL, 0};

    CURL *curl = curl_easy_init();
    if (!curl) { free(url); return NULL; }

    apply_common_opts(curl, config);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_buf);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    curl_easy_cleanup(curl);
    free(url);

    if (res != CURLE_OK || http_code != 200) {
        fprintf(stderr, "Could not fetch latest version for %s\n", package_name);
        free(buf.data);
        return NULL;
    }

    /* Trim trailing newline */
    if (buf.data) {
        size_t len = strlen(buf.data);
        while (len > 0 && (buf.data[len - 1] == '\n' || buf.data[len - 1] == '\r'))
            buf.data[--len] = '\0';
    }
    return buf.data; /* caller must free() */
}

int clhfm_network_push_package(
    const clhfm_config_t *config,
    const char *package_name,
    const char *version,
    const char *archive_path)
{
    if (!config || !package_name || !version || !archive_path) return -1;

    char path[512];
    snprintf(path, sizeof(path), "packages/%s/%s", package_name, version);
    char *url = build_url(config->registry_url, path);
    if (!url) return -1;

    CURL *curl = curl_easy_init();
    if (!curl) { free(url); return -1; }

    /* Use multipart/form-data upload */
    curl_mime *form = curl_mime_init(curl);
    curl_mimepart *part = curl_mime_addpart(form);
    curl_mime_name(part, "archive");
    if (curl_mime_filedata(part, archive_path) != CURLE_OK) {
        fprintf(stderr, "Error: cannot read archive %s\n", archive_path);
        curl_mime_free(form);
        curl_easy_cleanup(curl);
        free(url);
        return -1;
    }

    clhfm_http_buf_t resp = {NULL, 0};
    apply_common_opts(curl, config);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_buf);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    curl_mime_free(form);
    curl_easy_cleanup(curl);
    free(url);
    free(resp.data);

    if (res != CURLE_OK) {
        fprintf(stderr, "Network error: %s\n", curl_easy_strerror(res));
        return -1;
    }
    if (http_code != 200 && http_code != 201) {
        fprintf(stderr, "Server returned HTTP %ld when pushing %s@%s\n", http_code, package_name, version);
        return -1;
    }
    return 0;
}

int clhfm_network_create_tag(
    const clhfm_config_t *config,
    const char *package_name,
    const char *tag_name,
    const char *version)
{
    if (!config || !package_name || !tag_name || !version) return -1;

    char path[512];
    snprintf(path, sizeof(path), "packages/%s/tags/%s", package_name, tag_name);
    char *url = build_url(config->registry_url, path);
    if (!url) return -1;

    /* POST body: version=<version> */
    char post_body[256];
    snprintf(post_body, sizeof(post_body), "version=%s", version);

    clhfm_http_buf_t resp = {NULL, 0};

    CURL *curl = curl_easy_init();
    if (!curl) { free(url); return -1; }

    apply_common_opts(curl, config);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_body);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_buf);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    curl_easy_cleanup(curl);
    free(url);
    free(resp.data);

    if (res != CURLE_OK) {
        fprintf(stderr, "Network error: %s\n", curl_easy_strerror(res));
        return -1;
    }
    if (http_code != 200 && http_code != 201) {
        fprintf(stderr, "Server returned HTTP %ld when creating tag %s\n", http_code, tag_name);
        return -1;
    }
    return 0;
}
