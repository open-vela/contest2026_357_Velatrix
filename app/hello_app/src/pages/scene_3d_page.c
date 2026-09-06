/**
 * @file scene_3d_page.c
 * @brief 按需创建和销毁的 3D 轨迹场景页
 */

#include "../../inc/pages/scene_3d_page.h"
#include "../../inc/pages/page_manager.h"
#include "../../inc/loader/track_registry.h"
#include "../../inc/core/renderer3d.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    PageManager_t* page_manager;
    lv_obj_t* page;
    Renderer3D_t* renderer;
    TrackData_t track;
    lv_point_t last_point;
    bool dragging;
} Scene3DContext_t;

static Scene3DContext_t* g_context;

static void input_event_cb(lv_event_t* e)
{
    Renderer3D_t* renderer = (Renderer3D_t*)lv_event_get_user_data(e);
    if (!renderer || !g_context) return;

    switch (lv_event_get_code(e)) {
    case LV_EVENT_PRESSED: {
        lv_indev_get_point(lv_indev_get_act(), &g_context->last_point);
        g_context->dragging = true;
        printf("[Scene3D] 拖拽开始: (%d, %d)\n",
               g_context->last_point.x, g_context->last_point.y);
        break;
    }
    case LV_EVENT_PRESSING: {
        lv_point_t point;
        lv_indev_get_point(lv_indev_get_act(), &point);
        if (g_context->dragging) {
            int dx = point.x - g_context->last_point.x;
            int dy = point.y - g_context->last_point.y;
            renderer->camera.theta += dx * 0.005f;
            renderer->camera.phi += dy * 0.005f;
            renderer->camera.phi = clampf(
                renderer->camera.phi,
                -80.0f * (float)M_PI / 180.0f,
                80.0f * (float)M_PI / 180.0f);
            g_context->last_point = point;
            renderer3d_mark_dirty(renderer);
        }
        break;
    }
    case LV_EVENT_RELEASED:
        g_context->dragging = false;
        printf("[Scene3D] 拖拽结束\n");
        break;
    case LV_EVENT_SCROLL: {
        lv_dir_t direction = lv_indev_get_scroll_dir(lv_indev_get_act());
        renderer3d_handle_zoom(renderer, direction == LV_DIR_TOP);
        break;
    }
    case LV_EVENT_ROTARY:
        renderer3d_handle_zoom(renderer, lv_event_get_rotary_diff(e) > 0);
        break;
    default:
        break;
    }
}

static void zoom_in_cb(lv_event_t* e)
{
    Renderer3D_t* renderer = (Renderer3D_t*)lv_event_get_user_data(e);
    renderer3d_handle_zoom(renderer, true);
}

static void zoom_out_cb(lv_event_t* e)
{
    Renderer3D_t* renderer = (Renderer3D_t*)lv_event_get_user_data(e);
    renderer3d_handle_zoom(renderer, false);
}

static void btn_back_cb(lv_event_t* e)
{
    Scene3DContext_t* context =
        (Scene3DContext_t*)lv_event_get_user_data(e);
    if (!context || !context->page_manager) return;

    PageManager_t* page_manager = context->page_manager;
    printf("[Scene3D] 返回列表\n");
    if (!page_manager_switch_to(page_manager, "track_list", NULL)) {
        printf("[Scene3D] 返回列表失败\n");
        return;
    }
    lv_obj_t* page =
        page_manager_unregister(page_manager, "scene_3d");
    if (page) lv_obj_del(page);
}

static lv_obj_t* create_button(lv_obj_t* parent, const char* text,
                               lv_align_t align, int x, int y,
                               lv_event_cb_t callback, void* user_data)
{
    lv_obj_t* button = lv_btn_create(parent);
    lv_obj_set_size(button, 70, 40);
    lv_obj_align(button, align, x, y);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x333355), 0);
    lv_obj_set_style_radius(button, 8, 0);
    lv_obj_add_event_cb(button, callback, LV_EVENT_CLICKED, user_data);

    lv_obj_t* label = lv_label_create(button);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_center(label);
    return button;
}

lv_obj_t* scene_3d_page_create(lv_obj_t* parent, void* user_data)
{
    if (!parent || !user_data || g_context) return NULL;

    Scene3DContext_t* context = calloc(1, sizeof(*context));
    if (!context) return NULL;

    context->page_manager = (PageManager_t*)user_data;
    context->page = lv_obj_create(parent);
    if (!context->page) {
        free(context);
        return NULL;
    }

    lv_obj_remove_style_all(context->page);
    lv_obj_set_size(context->page, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(context->page, lv_color_make(10, 10, 26), 0);
    lv_obj_set_style_bg_opa(context->page, LV_OPA_COVER, 0);
    g_context = context;
    return context->page;
}

void scene_3d_page_on_show(lv_obj_t* page, void* data)
{
    const TrackEntry_t* entry = (const TrackEntry_t*)data;
    if (!page || !entry || !entry->path || !g_context) return;

    if (!track_loader_load(entry->path, &g_context->track)) {
        lv_obj_t* error = lv_label_create(page);
        lv_label_set_text(error, "Failed to load track");
        lv_obj_center(error);
        return;
    }

    g_context->renderer = renderer3d_create(page, &g_context->track);
    if (!g_context->renderer) {
        track_loader_clear(&g_context->track);
        lv_obj_t* error = lv_label_create(page);
        lv_label_set_text(error, "Failed to create renderer");
        lv_obj_center(error);
        return;
    }

    lv_obj_t* frame = lv_obj_create(page);
    lv_obj_remove_style_all(frame);
    lv_obj_set_size(frame, 480, 480);
    lv_obj_align(frame, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(frame, lv_color_hex(0x00FF00), 0);
    lv_obj_set_style_bg_opa(frame, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(frame, 15, 0);

    lv_obj_set_size(g_context->renderer->canvas, 460, 460);
    lv_obj_align(g_context->renderer->canvas, LV_ALIGN_CENTER, 0, 0);
    lv_obj_move_foreground(g_context->renderer->canvas);
    lv_obj_add_flag(g_context->renderer->canvas, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(g_context->renderer->canvas, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(g_context->renderer->canvas, LV_DIR_ALL);

    lv_obj_add_event_cb(g_context->renderer->canvas, input_event_cb,
                        LV_EVENT_PRESSED, g_context->renderer);
    lv_obj_add_event_cb(g_context->renderer->canvas, input_event_cb,
                        LV_EVENT_PRESSING, g_context->renderer);
    lv_obj_add_event_cb(g_context->renderer->canvas, input_event_cb,
                        LV_EVENT_RELEASED, g_context->renderer);
    lv_obj_add_event_cb(g_context->renderer->canvas, input_event_cb,
                        LV_EVENT_SCROLL, g_context->renderer);
    lv_obj_add_event_cb(g_context->renderer->canvas, input_event_cb,
                        LV_EVENT_ROTARY, g_context->renderer);
    lv_obj_t* title = lv_label_create(page);
    const char* name = entry->name ? entry->name : "Track";
    const char* suffix = strrchr(name, '.');
    int name_length = suffix && strcmp(suffix, ".bin") == 0 ?
                      (int)(suffix - name) : (int)strlen(name);
    lv_label_set_text_fmt(title, "3D: %.*s", name_length, name);
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 90, 18);

    create_button(page, "<", LV_ALIGN_TOP_LEFT, 10, 10, btn_back_cb, g_context);
    create_button(page, "+", LV_ALIGN_BOTTOM_RIGHT, -20, -20,
                  zoom_in_cb, g_context->renderer);
    create_button(page, "-", LV_ALIGN_BOTTOM_LEFT, 20, -20,
                  zoom_out_cb, g_context->renderer);

    renderer3d_start(g_context->renderer);
    renderer3d_render_frame(g_context->renderer);
}

void scene_3d_page_on_hide(lv_obj_t* page)
{
    (void)page;
    if (!g_context) return;

    if (g_context->renderer) {
        renderer3d_destroy(g_context->renderer);
        g_context->renderer = NULL;
    }
    track_loader_clear(&g_context->track);
    free(g_context);
    g_context = NULL;
}
