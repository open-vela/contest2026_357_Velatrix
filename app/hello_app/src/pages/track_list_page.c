/**
 * @file track_list_page.c
 * @brief 轨迹列表页实现（使用 track_registry）
 */

#include "../../inc/pages/track_list_page.h"
#include "../../inc/pages/page_manager.h"
#include "../../inc/pages/scene_3d_page.h"
#include "../../inc/loader/track_registry.h"
#include <stdio.h>
#include <string.h>

static PageManager_t* g_pm = NULL;

static void btn_back_cb(lv_event_t* e)
{
    PageManager_t* pm = (PageManager_t*)lv_event_get_user_data(e);
    if (pm) {
        page_manager_go_back(pm);
    }
}

static void btn_track_cb(lv_event_t* e)
{
    const TrackEntry_t* entry = (const TrackEntry_t*)lv_event_get_user_data(e);
    if (g_pm && entry) {
        printf("[TrackList] 选择轨迹: %s (%s)\n", entry->name, entry->path);
        lv_obj_t* scene_page = scene_3d_page_create(g_pm->root, g_pm);
        if (!scene_page ||
            !page_manager_register(g_pm, "scene_3d", scene_page,
                                   scene_3d_page_on_show,
                                   scene_3d_page_on_hide)) {
            printf("[TrackList] 创建 3D 场景失败\n");
            if (scene_page) lv_obj_del(scene_page);
            return;
        }

        page_manager_switch_to(g_pm, "scene_3d", (void*)entry);
    }
}

lv_obj_t* track_list_page_create(lv_obj_t* parent, void* user_data)
{
    g_pm = (PageManager_t*)user_data;

    lv_obj_t* page = lv_obj_create(parent);
    lv_obj_remove_style_all(page);
    lv_obj_set_size(page, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(page, lv_color_make(20, 20, 30), 0);
    lv_obj_set_style_bg_opa(page, LV_OPA_COVER, 0);

    /* 返回按钮 */
    lv_obj_t* btn_back = lv_btn_create(page);
    lv_obj_set_size(btn_back, 50, 40);
    lv_obj_align(btn_back, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_obj_set_style_bg_color(btn_back, lv_color_hex(0x333355), 0);
    lv_obj_set_style_radius(btn_back, 8, 0);
    lv_obj_add_event_cb(btn_back, btn_back_cb, LV_EVENT_CLICKED, g_pm);

    lv_obj_t* label_back = lv_label_create(btn_back);
    lv_label_set_text(label_back, "←");
    lv_obj_set_style_text_color(label_back, lv_color_white(), 0);
    lv_obj_set_style_text_font(label_back, &lv_font_montserrat_24, 0);
    lv_obj_center(label_back);

    /* 标题 */
    lv_obj_t* title = lv_label_create(page);
    lv_label_set_text(title, "My Track");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 70, 18);

    /* 从注册表生成轨迹列表 */
    int count = track_registry_count();
    for (int i = 0; i < count; i++) {
        const TrackEntry_t* entry = track_registry_get(i);
        if (!entry) continue;

        lv_obj_t* btn = lv_btn_create(page);
        lv_obj_set_size(btn, LV_HOR_RES - 40, 60);
        lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 70 + i * 75);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x333355), 0);
        lv_obj_set_style_radius(btn, 8, 0);
        /* 将注册表条目指针作为事件 user_data 传递 */
        lv_obj_add_event_cb(btn, btn_track_cb, LV_EVENT_CLICKED, (void*)entry);

        lv_obj_t* label = lv_label_create(btn);
        const char* suffix = strrchr(entry->name, '.');
        int name_length = suffix && strcmp(suffix, ".bin") == 0 ?
                          (int)(suffix - entry->name) :
                          (int)strlen(entry->name);
        lv_label_set_text_fmt(label, "%.*s", name_length, entry->name);
        lv_obj_set_style_text_color(label, lv_color_hex(0xCCCCDD), 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
        lv_obj_center(label);
    }

    return page;
}

void track_list_page_on_show(lv_obj_t* page, void* data)
{
    if (!page) return;
    (void)data;
    printf("[TrackList] 显示\n");
}

void track_list_page_on_hide(lv_obj_t* page)
{
    if (!page) return;
    printf("[TrackList] 隐藏\n");
}
