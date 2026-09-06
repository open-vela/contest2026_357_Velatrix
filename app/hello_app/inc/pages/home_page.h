/**
 * @file home_page.h
 * @brief 首页
 */

#ifndef __HOME_PAGE_H
#define __HOME_PAGE_H

#include "lvgl/lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

lv_obj_t* home_page_create(lv_obj_t* parent, void* user_data);
void home_page_on_show(lv_obj_t* page, void* data);
void home_page_on_hide(lv_obj_t* page);

#ifdef __cplusplus
}
#endif

#endif /* __HOME_PAGE_H */