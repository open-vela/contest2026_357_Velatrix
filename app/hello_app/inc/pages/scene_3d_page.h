/**
 * @file scene_3d_page.h
 * @brief 3D场景页
 */

#ifndef __SCENE_3D_PAGE_H
#define __SCENE_3D_PAGE_H

#include "lvgl/lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

lv_obj_t* scene_3d_page_create(lv_obj_t* parent, void* user_data);
void scene_3d_page_on_show(lv_obj_t* page, void* data);
void scene_3d_page_on_hide(lv_obj_t* page);

#ifdef __cplusplus
}
#endif

#endif /* __SCENE_3D_PAGE_H */