/**
 * @file track_registry.c
 * @brief 运行时扫描目录生成轨迹注册表实现
 */

#include "../../inc/loader/track_registry.h"
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdio.h>

static TrackEntry_t* s_entries = NULL;
static int s_count = 0;
static char** s_keys = NULL;
static char** s_names = NULL;
static char** s_paths = NULL;

static int has_track_ext(const char* name)
{
    size_t n = strlen(name);
    if (n > 4 && strcmp(name + n - 4, ".bin") == 0) return 1;
    if (n > 4 && strcmp(name + n - 4, ".kml") == 0) return 1;
    if (n > 4 && strcmp(name + n - 4, ".gpx") == 0) return 1;
    return 0;
}

int track_registry_init(const char* dir)
{
    if (!dir) dir = "./data";

    DIR* dp = opendir(dir);
    if (!dp) {
        perror("track_registry: opendir");
        return -1;
    }

    struct dirent* de;
    int count = 0;

    /* First pass: count matching files */
    while ((de = readdir(dp)) != NULL) {
        if (de->d_type == DT_DIR) continue;
        if (has_track_ext(de->d_name)) count++;
    }

    if (count == 0) {
        closedir(dp);
        s_count = 0;
        s_entries = NULL;
        return 0;
    }

    /* allocate arrays */
    s_keys = (char**)malloc(sizeof(char*) * count);
    s_names = (char**)malloc(sizeof(char*) * count);
    s_paths = (char**)malloc(sizeof(char*) * count);
    if (!s_keys || !s_names || !s_paths) goto fail_alloc;

    /* Second pass: fill entries */
    rewinddir(dp);
    int idx = 0;
    while ((de = readdir(dp)) != NULL) {
        if (de->d_type == DT_DIR) continue;
        if (!has_track_ext(de->d_name)) continue;

        /* key = filename without extension */
        char* dot = strrchr(de->d_name, '.');
        size_t base_len = dot ? (size_t)(dot - de->d_name) : strlen(de->d_name);
        s_keys[idx] = (char*)malloc(base_len + 1);
        if (!s_keys[idx]) goto fail_alloc;
        memcpy(s_keys[idx], de->d_name, base_len);
        s_keys[idx][base_len] = '\0';

        /* name = filename */
        s_names[idx] = strdup(de->d_name);
        if (!s_names[idx]) goto fail_alloc;

        /* path = dir + '/' + filename */
        size_t plen = strlen(dir) + 1 + strlen(de->d_name) + 1;
        s_paths[idx] = (char*)malloc(plen);
        if (!s_paths[idx]) goto fail_alloc;
        snprintf(s_paths[idx], plen, "%s/%s", dir, de->d_name);

        idx++;
    }

    closedir(dp);

    s_count = idx;
    s_entries = (TrackEntry_t*)malloc(sizeof(TrackEntry_t) * s_count);
    if (!s_entries) goto fail_alloc;

    for (int i = 0; i < s_count; i++) {
        s_entries[i].key = s_keys[i];
        s_entries[i].name = s_names[i];
        s_entries[i].path = s_paths[i];
    }

    return 0;

fail_alloc:
    perror("track_registry: alloc failed");
    if (dp) closedir(dp);
    track_registry_deinit();
    return -1;
}

void track_registry_deinit(void)
{
    if (s_entries) {
        free(s_entries);
        s_entries = NULL;
    }
    if (s_keys) {
        for (int i = 0; i < s_count; i++) free(s_keys[i]);
        free(s_keys);
        s_keys = NULL;
    }
    if (s_names) {
        for (int i = 0; i < s_count; i++) free(s_names[i]);
        free(s_names);
        s_names = NULL;
    }
    if (s_paths) {
        for (int i = 0; i < s_count; i++) free(s_paths[i]);
        free(s_paths);
        s_paths = NULL;
    }
    s_count = 0;
}

int track_registry_count(void)
{
    return s_count;
}

const TrackEntry_t* track_registry_get(int index)
{
    if (index < 0 || index >= s_count) return NULL;
    return &s_entries[index];
}
