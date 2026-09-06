/**
 * @file track_list_page.h
 * @brief 轨迹列表页
 */

#ifndef __TRACK_LIST_PAGE_H
#define __TRACK_LIST_PAGE_H

#include "lvgl/lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

lv_obj_t* track_list_page_create(lv_obj_t* parent, void* user_data);
void track_list_page_on_show(lv_obj_t* page, void* data);
void track_list_page_on_hide(lv_obj_t* page);

#ifdef __cplusplus
}
#endif

#endif /* __TRACK_LIST_PAGE_H */