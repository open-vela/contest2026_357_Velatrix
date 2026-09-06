/**
 * @file home_page.c
 * @brief 首页实现 - 标题 + 三个功能按钮
 */

#include "../../inc/pages/home_page.h"
#include "../../inc/pages/page_manager.h"
#include <stdio.h>

static PageManager_t* g_pm = NULL;

static void btn_my_tracks_cb(lv_event_t* e)
{
    PageManager_t* pm = (PageManager_t*)lv_event_get_user_data(e);
    if (pm) {
        printf("[Home] 点击: 我的轨迹\n");
        page_manager_switch_to(pm, "track_list", NULL);
    }
}

static void btn_track_settings_cb(lv_event_t* e)
{
    printf("[Home] 点击: 轨迹设置\n");
}

static void btn_debug_settings_cb(lv_event_t* e)
{
    printf("[Home] 点击: 调试设置\n");
}

lv_obj_t* home_page_create(lv_obj_t* parent, void* user_data)
{
    g_pm = (PageManager_t*)user_data;

    lv_obj_t* page = lv_obj_create(parent);
    lv_obj_remove_style_all(page);
    lv_obj_set_size(page, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(page, lv_color_make(10, 10, 26), 0);
    lv_obj_set_style_bg_opa(page, LV_OPA_COVER, 0);
    // lv_obj_set_visible(page, false);

    /* 标题: "track" */
    lv_obj_t* title = lv_label_create(page);
    lv_label_set_text(title, "track");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_32, 0);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 20, 20);

    /* 按钮1: My Track */
    lv_obj_t* btn1 = lv_btn_create(page);
    lv_obj_set_size(btn1, LV_HOR_RES - 40, 55);
    lv_obj_align(btn1, LV_ALIGN_TOP_MID, 0, 90);
    lv_obj_set_style_bg_color(btn1, lv_color_hex(0x333355), 0);
    lv_obj_set_style_bg_opa(btn1, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(btn1, 10, 0);
    lv_obj_add_event_cb(btn1, btn_my_tracks_cb, LV_EVENT_CLICKED, g_pm);

    lv_obj_t* label1 = lv_label_create(btn1);
    lv_label_set_text(label1, "📋 My Track");
    lv_obj_set_style_text_color(label1, lv_color_white(), 0);
    lv_obj_set_style_text_font(label1, &lv_font_montserrat_16, 0);
    lv_obj_center(label1);

    /* 按钮2: Track Set */
    lv_obj_t* btn2 = lv_btn_create(page);
    lv_obj_set_size(btn2, LV_HOR_RES - 40, 55);
    lv_obj_align(btn2, LV_ALIGN_TOP_MID, 0, 160);
    lv_obj_set_style_bg_color(btn2, lv_color_hex(0x333355), 0);
    lv_obj_set_style_bg_opa(btn2, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(btn2, 10, 0);
    lv_obj_add_event_cb(btn2, btn_track_settings_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t* label2 = lv_label_create(btn2);
    lv_label_set_text(label2, "⚙️ Track Set");
    lv_obj_set_style_text_color(label2, lv_color_white(), 0);
    lv_obj_set_style_text_font(label2, &lv_font_montserrat_16, 0);
    lv_obj_center(label2);

    /* 按钮3: Debug Set */
    lv_obj_t* btn3 = lv_btn_create(page);
    lv_obj_set_size(btn3, LV_HOR_RES - 40, 55);
    lv_obj_align(btn3, LV_ALIGN_TOP_MID, 0, 230);
    lv_obj_set_style_bg_color(btn3, lv_color_hex(0x333355), 0);
    lv_obj_set_style_bg_opa(btn3, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(btn3, 10, 0);
    lv_obj_add_event_cb(btn3, btn_debug_settings_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t* label3 = lv_label_create(btn3);
    lv_label_set_text(label3, "🐛 Debug Set");
    lv_obj_set_style_text_color(label3, lv_color_white(), 0);
    lv_obj_set_style_text_font(label3, &lv_font_montserrat_16, 0);
    lv_obj_center(label3);

    /* 底部: 队伍信息 */
    lv_obj_t* footer = lv_label_create(page);
    lv_label_set_text(footer, "Velatrix Team (357)");
    lv_obj_set_style_text_color(footer, lv_color_hex(0x444466), 0);
    lv_obj_set_style_text_font(footer, &lv_font_montserrat_16, 0);
    lv_obj_align(footer, LV_ALIGN_BOTTOM_MID, 0, -20);

    printf("[Home] 首页创建完成\n");
    return page;
}

void home_page_on_show(lv_obj_t* page, void* data)
{
    if (!page) return;
    (void)data;
    printf("[Home] 显示\n");
    // lv_obj_set_visible(page, true);
}

void home_page_on_hide(lv_obj_t* page)
{
    if (!page) return;
    printf("[Home] 隐藏\n");
    // lv_obj_set_visible(page, false);
}