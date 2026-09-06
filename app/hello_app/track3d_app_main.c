/**
 * @file track3d_app_main.c
 * @brief 3D 轨迹显示应用主入口
 * @author Velatrix Team (357)
 * @version 2.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lvgl/lvgl.h"
#include <uv.h>
#include <fcntl.h>
#include <unistd.h>
#include <nuttx/input/mouse.h>

/* 页面管理 */
#include "inc/pages/page_manager.h"
#include "inc/pages/track_list_page.h"
#include "inc/loader/track_registry.h"

static PageManager_t* g_pm = NULL;

int main(int argc, char* argv[])
{
    printf("[App] 3D Track Visualization\n");
    printf("[App] Team: Velatrix (357)\n");

    /* 初始化 LVGL */
    if (!lv_is_initialized())
    {
        lv_init();

        lv_nuttx_dsc_t info;
        lv_nuttx_result_t result;
        lv_nuttx_dsc_init(&info);

        info.input_path = "/dev/input0";
#if LV_USE_NUTTX_MOUSE
        info.mouse_path = "/dev/mouse0";
#endif

        lv_nuttx_init(&info, &result);

        if (result.disp == NULL)
        {
            printf("[App] 显示初始化失败\n");
            return -1;
        }

        printf("[App] 显示初始化成功\n");
        printf("[App] 输入设备: %s\n", info.input_path);
#if LV_USE_NUTTX_MOUSE
        printf("[App] 鼠标设备: %s\n", info.mouse_path);
#endif
    }

    /* 创建页面管理器 */
    g_pm = page_manager_create(lv_scr_act());
    if (!g_pm) {
        printf("[App] 页面管理器创建失败\n");
        return -1;
    }

    /* 初始化轨迹注册表（默认扫描 ./data 目录） */
    if (track_registry_init("./data") != 0) {
        printf("[App] 警告：track_registry 初始化失败或目录无文件\n");
        /* 仍然继续，页面会显示为空 */
    }

    /* 注册页面：简化为轨迹列表和 3D 场景 */
    lv_obj_t* list_page = track_list_page_create(lv_scr_act(), g_pm);
    page_manager_register(g_pm, "track_list", list_page,
                          track_list_page_on_show, track_list_page_on_hide);

    /* 默认显示轨迹列表 */
    page_manager_switch_to(g_pm, "track_list", NULL);

    /* 运行主循环 */
    lv_nuttx_uv_t uv_info;
    lv_nuttx_result_t result;
    uv_loop_t loop;

    memset(&uv_info, 0, sizeof(uv_info));
    uv_loop_init(&loop);

    uv_info.loop = &loop;
    uv_info.disp = result.disp;
    uv_info.indev = result.indev;
#ifdef CONFIG_UINPUT_TOUCH
    uv_info.uindev = result.utouch_indev;
#endif

    void* data = lv_nuttx_uv_init(&uv_info);
    uv_run(&loop, UV_RUN_DEFAULT);
    lv_nuttx_uv_deinit(&data);

    /* 清理 */
    track_registry_deinit();
    page_manager_destroy(g_pm);
    lv_deinit();

    printf("[App] 程序退出\n");
    return 0;
}