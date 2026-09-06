/**
 * @file page_manager.c
 * @brief 页面管理器实现
 */

#include "../../inc/pages/page_manager.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

PageManager_t* page_manager_create(lv_obj_t* root)
{
    PageManager_t* pm = malloc(sizeof(PageManager_t));
    if (!pm) return NULL;

    memset(pm, 0, sizeof(PageManager_t));
    pm->root = root;
    pm->page_count = 0;
    pm->current_index = -1;

    return pm;
}

void page_manager_destroy(PageManager_t* pm)
{
    if (pm) free(pm);
}

bool page_manager_register(PageManager_t* pm, const char* name, lv_obj_t* page,
                           void (*on_show)(lv_obj_t* page, void* data),
                           void (*on_hide)(lv_obj_t* page))
{
    if (!pm || !name || !page || pm->page_count >= MAX_PAGES) return false;

    strncpy(pm->pages[pm->page_count].name, name, PAGE_NAME_LEN - 1);
    pm->pages[pm->page_count].page = page;
    pm->pages[pm->page_count].data = NULL;
    pm->pages[pm->page_count].on_show = on_show;
    pm->pages[pm->page_count].on_hide = on_hide;
    pm->page_count++;

    // lv_obj_set_visible(page, false);

    printf("[PageManager] 注册页面: %s\n", name);
    return true;
}

lv_obj_t* page_manager_unregister(PageManager_t* pm, const char* name)
{
    if (!pm || !name) return NULL;

    for (int i = 0; i < pm->page_count; i++) {
        if (strcmp(pm->pages[i].name, name) != 0) continue;

        lv_obj_t* page = pm->pages[i].page;
        if (pm->current_index == i) {
            pm->current_index = -1;
        } else if (pm->current_index > i) {
            pm->current_index--;
        }

        for (int j = i; j < pm->page_count - 1; j++) {
            pm->pages[j] = pm->pages[j + 1];
        }
        pm->page_count--;
        printf("[PageManager] 注销页面: %s\n", name);
        return page;
    }

    return NULL;
}

bool page_manager_switch_to(PageManager_t* pm, const char* name, void* data)
{
    if (!pm) return false;

    int target = -1;
    for (int i = 0; i < pm->page_count; i++) {
        if (strcmp(pm->pages[i].name, name) == 0) {
            target = i;
            break;
        }
    }

    if (target < 0) {
        printf("[PageManager] 页面不存在: %s\n", name);
        return false;
    }

    if (pm->current_index >= 0 && pm->current_index < pm->page_count) {
        PageEntry_t* cur = &pm->pages[pm->current_index];
        if (cur->on_hide) {
            cur->on_hide(cur->page);
        }
        // lv_obj_set_visible(cur->page, false);
    }

    PageEntry_t* target_page = &pm->pages[target];
    target_page->data = data;
    if (target_page->on_show) {
        target_page->on_show(target_page->page, data);
    }
    // lv_obj_set_visible(target_page->page, true);

    pm->current_index = target;
    printf("[PageManager] 切换到: %s\n", name);
    return true;
}

bool page_manager_go_back(PageManager_t* pm)
{
    if (!pm || pm->current_index <= 0) {
        printf("[PageManager] 无法返回\n");
        return false;
    }
    return page_manager_switch_to(pm, pm->pages[pm->current_index - 1].name, NULL);
}

const char* page_manager_get_current(PageManager_t* pm)
{
    if (!pm || pm->current_index < 0) return NULL;
    return pm->pages[pm->current_index].name;
}

void* page_manager_get_current_data(PageManager_t* pm)
{
    if (!pm || pm->current_index < 0) return NULL;
    return pm->pages[pm->current_index].data;
}