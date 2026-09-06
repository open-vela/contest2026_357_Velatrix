/**
 * @file page_manager.h
 * @brief 页面管理器
 */

#ifndef __PAGE_MANAGER_H
#define __PAGE_MANAGER_H

#include "lvgl/lvgl.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_PAGES 16
#define PAGE_NAME_LEN 32

typedef struct {
    char name[PAGE_NAME_LEN];
    lv_obj_t* page;
    void* data;
    void (*on_show)(lv_obj_t* page, void* data);
    void (*on_hide)(lv_obj_t* page);
} PageEntry_t;

typedef struct {
    lv_obj_t* root;
    PageEntry_t pages[MAX_PAGES];
    int page_count;
    int current_index;
} PageManager_t;

PageManager_t* page_manager_create(lv_obj_t* root);
void page_manager_destroy(PageManager_t* pm);

bool page_manager_register(PageManager_t* pm, const char* name, lv_obj_t* page,
                           void (*on_show)(lv_obj_t* page, void* data),
                           void (*on_hide)(lv_obj_t* page));
lv_obj_t* page_manager_unregister(PageManager_t* pm, const char* name);

bool page_manager_switch_to(PageManager_t* pm, const char* name, void* data);
bool page_manager_go_back(PageManager_t* pm);

const char* page_manager_get_current(PageManager_t* pm);
void* page_manager_get_current_data(PageManager_t* pm);

#ifdef __cplusplus
}
#endif

#endif /* __PAGE_MANAGER_H */