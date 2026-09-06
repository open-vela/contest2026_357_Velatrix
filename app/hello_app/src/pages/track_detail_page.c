/**
 * @file track_detail_page.c
 * @brief 轨迹详情页实现
 */

#include "../../inc/pages/track_detail_page.h"
#include "../../inc/pages/page_manager.h"
#include <stdio.h>

static PageManager_t* g_pm = NULL;

static void btn_back_cb(lv_event_t* e)
{
    PageManager_t* pm = (PageManager_t*)lv_event_get_user_data(e);
    if (pm) {
        page_manager_go_back(pm);
    }
}

static void btn_3d_scene_cb(lv_event_t* e)
{
    PageManager_t* pm = (PageManager_t*)lv_event_get_user_data(e);
    if (pm) {
        printf("[TrackDetail] 进入 3D 场景\n");
        page_manager_switch_to(pm, "scene_3d", NULL);
    }
}

lv_obj_t* track_detail_page_create(lv_obj_t* parent, void* user_data)
{
    g_pm = (PageManager_t*)user_data;

    lv_obj_t* page = lv_obj_create(parent);
    lv_obj_remove_style_all(page);
    lv_obj_set_size(page, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(page, lv_color_make(20, 20, 30), 0);
    lv_obj_set_style_bg_opa(page, LV_OPA_COVER, 0);
    // lv_obj_set_visible(page, false);

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
    lv_label_set_text(title, "Xihu Pig Line");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 70, 18);

    /* 数据展示 */
    const char* info[] = {
        "Total Distance: 13.97 km",
        "Total Climb: 226 m",
        "Max Altitude: 776 m",
        "Total Time: 1h0min",
        "Average Speed: 13.97 km/h"
    };

    for (int i = 0; i < 5; i++) {
        lv_obj_t* label = lv_label_create(page);
        lv_label_set_text(label, info[i]);
        lv_obj_set_style_text_color(label, lv_color_hex(0xCCCCDD), 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 20, 70 + i * 35);
    }

    /* 3D 场景按钮 */
    lv_obj_t* btn_3d = lv_btn_create(page);
    lv_obj_set_size(btn_3d, LV_HOR_RES - 40, 50);
    lv_obj_align(btn_3d, LV_ALIGN_BOTTOM_MID, 0, -30);
    lv_obj_set_style_bg_color(btn_3d, lv_color_hex(0x4A6CF7), 0);
    lv_obj_set_style_radius(btn_3d, 12, 0);
    lv_obj_add_event_cb(btn_3d, btn_3d_scene_cb, LV_EVENT_CLICKED, g_pm);

    lv_obj_t* label_3d = lv_label_create(btn_3d);
    lv_label_set_text(label_3d, "🎬 3D Scene");
    lv_obj_set_style_text_color(label_3d, lv_color_white(), 0);
    lv_obj_set_style_text_font(label_3d, &lv_font_montserrat_16, 0);
    lv_obj_center(label_3d);

    return page;
}

void track_detail_page_on_show(lv_obj_t* page, void* data)
{
    if (!page) return;
    (void)data;
    printf("[TrackDetail] 显示\n");
    // lv_obj_set_visible(page, true);
}

void track_detail_page_on_hide(lv_obj_t* page)
{
    if (!page) return;
    printf("[TrackDetail] 隐藏\n");
    // lv_obj_set_visible(page, false);
}