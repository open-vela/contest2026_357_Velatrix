/**
 * @file renderer3d.h
 * @brief 3D 渲染器
 * @author Velatrix Team (357)
 * @version 1.2
 */

#ifndef __RENDERER3D_H
#define __RENDERER3D_H

#include "math3d.h"
#include "../loader/track_loader.h"
#include "lvgl/lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 屏幕大小（画布大小）
 */
#define RENDERER_SCREEN_SIZE 460

/**
 * @brief 星星数量
 */
#define RENDERER_STAR_COUNT 100

/**
 * @brief 网格数量
 */
#define RENDERER_GRID_COUNT 10

/**
 * @brief 最大垂直角度（度）
 */
#define RENDERER_MAX_PHI_DEG 80

/**
 * @brief 默认焦距（标准透视，视野约 90°）
 */
#define RENDERER_DEFAULT_FOCAL_LENGTH 1.0f

/**
 * @brief 默认摄像机半径（轨迹范围 -1~1，半径 2.5 可完整显示）
 */
#define RENDERER_DEFAULT_RADIUS 1.5f

/**
 * @brief 轨迹绘制模式
 */
typedef enum {
    TRACK_MODE_FULL,      /**< 完整轨迹 */
    TRACK_MODE_DYNAMIC,   /**< 动态绘制（从起点到终点） */
    TRACK_MODE_LOOP,      /**< 循环绘制（反复播放） */
} TrackMode_t;

/**
 * @brief 星星生成风格
 */
typedef enum {
    STAR_STYLE_NORMAL,    /**< 普通星空 */
    STAR_STYLE_DENSE,     /**< 密集星空 */
    STAR_STYLE_SPARSE,    /**< 稀疏星空 */
    STAR_STYLE_GALAXY,    /**< 银河风格（星星集中在一条带） */
} StarStyle_t;

/**
 * @brief 星星结构体
 */
typedef struct
{
    float theta;                /**< 水平角度（弧度） */
    float phi;                  /**< 垂直角度（弧度） */
    int size;                   /**< 星星大小 */
    float phase;                /**< 闪烁相位 */
    float speed;                /**< 闪烁速度 */
    float base_brightness;      /**< 基础亮度 0.0~1.0 */
    int color_type;             /**< 颜色类型索引 */
} SkyStar_t;

/**
 * @brief 渲染器结构体
 */
typedef struct
{
    lv_obj_t* canvas;           /**< LVGL 画布 */
    lv_color_t* framebuffer;    /**< 画布缓冲区指针（用于释放内存） */
    int width;                  /**< 屏幕宽度 */
    int height;                 /**< 屏幕高度 */
    Camera_t camera;            /**< 摄像机 */
    TrackData_t* track;         /**< 轨迹数据 */
    SkyStar_t stars[RENDERER_STAR_COUNT];  /**< 星星数组 */
    uint32_t last_frame_time;   /**< 上一帧时间 */
    int target_fps;             /**< 目标帧率 */
    uint8_t slow_frame_count;   /**< 连续超出当前帧预算的次数 */
    lv_timer_t* render_timer;   /**< 渲染定时器 */
    bool is_dragging;           /**< 是否正在拖拽 */
    bool dirty;                 /**< 是否需要重绘（优化：合并重绘请求） */
    Vec2D_t projected_cache[TRACK_MAX_POINTS];  /**< 投影点缓存（避免 malloc/free） */
    
    /* 轨迹动画 */
    TrackMode_t track_mode;     /**< 轨迹绘制模式 */
    float track_progress;       /**< 绘制进度 0.0 ~ 1.0 */
    float track_speed;          /**< 绘制速度（每帧增量） */
    bool track_forward;         /**< 绘制方向 */
    uint32_t track_segments;    /**< 当前绘制的线段数量 */
} Renderer3D_t;

/**
 * @brief 触摸状态
 */
typedef struct
{
    bool is_dragging;
    int last_x;
    int last_y;
    float sensitivity;
} TouchState_t;

/**
 * @brief 创建渲染器
 *
 * @param parent 父对象
 * @param track 轨迹数据指针
 * @return Renderer3D_t* 渲染器指针，失败返回 NULL
 */
Renderer3D_t* renderer3d_create(lv_obj_t* parent, TrackData_t* track);

/**
 * @brief 销毁渲染器
 *
 * @param renderer 渲染器指针
 */
void renderer3d_destroy(Renderer3D_t* renderer);

/**
 * @brief 渲染一帧
 *
 * @param renderer 渲染器指针
 */
void renderer3d_render_frame(Renderer3D_t* renderer);

/**
 * @brief 启动渲染循环
 *
 * @param renderer 渲染器指针
 */
void renderer3d_start(Renderer3D_t* renderer);

/**
 * @brief 停止渲染循环
 *
 * @param renderer 渲染器指针
 */
void renderer3d_stop(Renderer3D_t* renderer);

/**
 * @brief 重置摄像机视角
 *
 * @param renderer 渲染器指针
 */
void renderer3d_reset_camera(Renderer3D_t* renderer);

/**
 * @brief 处理触摸事件
 *
 * @param renderer 渲染器指针
 * @param x 触摸 X 坐标
 * @param y 触摸 Y 坐标
 * @param pressed 是否按下
 */
void renderer3d_handle_touch(Renderer3D_t* renderer,
                             int x, int y, bool pressed);

/**
 * @brief 处理缩放事件（通过改变摄像机半径）
 *
 * @param renderer 渲染器指针
 * @param zoom_in 是否放大
 */
void renderer3d_handle_zoom(Renderer3D_t* renderer, bool zoom_in);

/**
 * @brief 设置拖拽状态
 *
 * @param renderer 渲染器指针
 * @param dragging 是否正在拖拽
 */
void renderer3d_set_dragging(Renderer3D_t* renderer, bool dragging);

/**
 * @brief 生成星星（支持多种风格，每次进入不同）
 *
 * @param renderer 渲染器指针
 */
void renderer3d_generate_stars(Renderer3D_t* renderer);

/**
 * @brief 强制重新生成星星（用于刷新星空）
 *
 * @param renderer 渲染器指针
 */
void renderer3d_regenerate_stars(Renderer3D_t* renderer);

/**
 * @brief 标记需要重绘（外部调用，如拖拽时）
 *
 * @param renderer 渲染器指针
 */
void renderer3d_mark_dirty(Renderer3D_t* renderer);

/**
 * @brief 设置轨迹绘制模式
 *
 * @param renderer 渲染器指针
 * @param mode 绘制模式
 */
void renderer3d_set_track_mode(Renderer3D_t* renderer, TrackMode_t mode);

/**
 * @brief 重置轨迹动画
 *
 * @param renderer 渲染器指针
 */
void renderer3d_reset_track_animation(Renderer3D_t* renderer);

#ifdef __cplusplus
}
#endif

#endif /* __RENDERER3D_H */