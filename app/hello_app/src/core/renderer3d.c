/**
 * @file renderer3d.c
 * @brief 3D 渲染器实现
 * @author Velatrix Team (357)
 * @version 1.2
 */

#include "../../inc/core/renderer3d.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/**
 * @brief 银河渐变色表
 */
static const uint16_t galaxy_colors[] =
{
    0x3C64,  /* 蓝 */
    0x6450,  /* 紫 */
    0xC850,  /* 粉紫 */
    0xFF50,  /* 粉红 */
    0xFF78,  /* 橙红 */
    0x64C8,  /* 青 */
};

/**
 * @brief 星星颜色表（按真实恒星颜色比例）
 * 白色/蓝白色 ~90%, 淡黄色 ~5%, 橙红色 ~3%, 淡蓝色 ~2%
 */
static const uint16_t star_color_table[] =
{
    0xFFFF,  /* 白色 - 90% */
    0xFFFF,  /* 白色 */
    0xFFFF,  /* 白色 */
    0xFFFF,  /* 白色 */
    0xFFFF,  /* 白色 */
    0xFFFF,  /* 白色 */
    0xFFFF,  /* 白色 */
    0xFFFF,  /* 白色 */
    0xFFFF,  /* 白色 */
    0xFFFF,  /* 白色 */
    0xFFF0,  /* 淡黄色 - 5% */
    0xFFF0,  /* 淡黄色 */
    0xFF80,  /* 橙红色 - 3% */
    0xB4DC,  /* 淡蓝色 - 2% */
};

/**
 * @brief 获取银河渐变色
 */
static lv_color_t get_galaxy_color(float t)
{
    int idx = (int)(t * 5);
    if (idx > 5) idx = 5;
    return lv_color_hex((uint32_t)galaxy_colors[idx]);
}

/**
 * @brief 获取星星颜色
 */
static lv_color_t get_star_color(int color_type, float brightness)
{
    uint16_t base = star_color_table[color_type % 14];
    uint8_t r = ((base >> 11) & 0x1F) * brightness;
    uint8_t g = ((base >> 5) & 0x3F) * brightness;
    uint8_t b = (base & 0x1F) * brightness;
    return lv_color_make(r * 8, g * 4, b * 8);
}

/**
 * @brief 渲染单个星星（天空盒模式：屏幕空间，跟随视角旋转）
 * 
 * 星星在无穷远，视角旋转时移动速度极慢，增强深邃感
 */
static void render_star(lv_layer_t* layer, const SkyStar_t* star,
                        const Camera_t* cam, int screen_w, int screen_h,
                        uint32_t time)
{
    /* ============================================================ */
    /* 1. 将星星的球面坐标映射到屏幕坐标（0 ~ screen_w/h） */
    /* ============================================================ */
    float px = (star->theta / (2.0f * M_PI)) * screen_w;
    float py = (star->phi / M_PI + 0.5f) * screen_h;
    
    /* ============================================================ */
    /* 2. 应用摄像机旋转偏移（星星跟随视角旋转，但速度极慢） */
    /* ============================================================ */
    /* ⭐ 无穷远效果：跟随速度 10%，移动非常缓慢 */
    float follow_factor = 0.10f;
    
    float offset_x = (cam->theta / (2.0f * M_PI)) * screen_w * follow_factor;
    float offset_y = (cam->phi / M_PI) * screen_h * follow_factor;
    
    px -= offset_x;
    py -= offset_y;
    
    /* ============================================================ */
    /* 3. 循环平铺（实现无限星空） */
    /* ============================================================ */
    px = fmodf(px, screen_w);
    if (px < 0) px += screen_w;
    
    py = fmodf(py, screen_h);
    if (py < 0) py += screen_h;
    
    /* ============================================================ */
    /* 4. 计算亮度（固定亮度，无闪烁） */
    /* ============================================================ */
    float brightness = star->base_brightness;
    int size = star->size;
    if (size <= 0) return;
    
    /* ============================================================ */
    /* 5. 获取颜色 */
    /* ============================================================ */
    lv_color_t color = get_star_color(star->color_type, brightness);
    
    /* ============================================================ */
    /* 6. 绘制星星（圆形） */
    /* ============================================================ */
    lv_draw_rect_dsc_t rect_dsc;
    lv_draw_rect_dsc_init(&rect_dsc);
    rect_dsc.bg_color = color;
    rect_dsc.bg_opa = LV_OPA_COVER;
    rect_dsc.radius = size;

    lv_area_t area = {
        .x1 = (lv_coord_t)(px - size),
        .y1 = (lv_coord_t)(py - size),
        .x2 = (lv_coord_t)(px + size),
        .y2 = (lv_coord_t)(py + size)
    };
    lv_draw_rect(layer, &rect_dsc, &area);
}

/**
 * @brief 绘制简化网格（外框 + 十字线）
 * 总共 6 根线：4 条边框 + 2 条十字线
 */
static void render_grid(lv_layer_t* layer, const Camera_t* cam,
                        int screen_w, int screen_h)
{
    float half = 1.2f;
    
    Vec3D_t corners[4] = {
        {-half, 0, -half},
        { half, 0, -half},
        { half, 0,  half},
        {-half, 0,  half}
    };
    
    Vec2D_t proj[4];
    bool valid[4];
    
    for (int i = 0; i < 4; i++) {
        Vec2D_t p = project_point(corners[i], cam, screen_w, screen_h);
        /* 有效条件：在屏幕附近 */
        if (p.x > -50 && p.x < screen_w + 50 &&
            p.y > -50 && p.y < screen_h + 50) {
            proj[i] = p;
            valid[i] = true;
        } else {
            proj[i] = p;
            valid[i] = false;
        }
    }
    
    lv_draw_line_dsc_t line_dsc;
    lv_draw_line_dsc_init(&line_dsc);
    line_dsc.width = 1;
    line_dsc.color = lv_color_make(80, 200, 255);
    line_dsc.opa = LV_OPA_60;
    
    /* ⭐ 四条边：只有两个端点都有效才画 */
    int edges[4][2] = {{0, 1}, {1, 2}, {2, 3}, {3, 0}};
    for (int e = 0; e < 4; e++) {
        int i = edges[e][0];
        int j = edges[e][1];
        /* ⭐ 两个端点都有效才画 */
        if (valid[i] && valid[j]) {
            line_dsc.p1.x = (lv_coord_t)proj[i].x;
            line_dsc.p1.y = (lv_coord_t)proj[i].y;
            line_dsc.p2.x = (lv_coord_t)proj[j].x;
            line_dsc.p2.y = (lv_coord_t)proj[j].y;
            lv_draw_line(layer, &line_dsc);
        }
    }
    
    /* 十字线同样处理 */
    lv_color_t grid_color_dim = lv_color_make(40, 100, 128);
    line_dsc.color = grid_color_dim;
    line_dsc.opa = LV_OPA_40;
    
    Vec3D_t cx1 = {0, 0, -half};
    Vec3D_t cx2 = {0, 0, half};
    Vec2D_t pcx1 = project_point(cx1, cam, screen_w, screen_h);
    Vec2D_t pcx2 = project_point(cx2, cam, screen_w, screen_h);
    
    bool vcx1 = (pcx1.x > -50 && pcx1.x < screen_w + 50 &&
                 pcx1.y > -50 && pcx1.y < screen_h + 50);
    bool vcx2 = (pcx2.x > -50 && pcx2.x < screen_w + 50 &&
                 pcx2.y > -50 && pcx2.y < screen_h + 50);
    
    if (vcx1 && vcx2) {
        line_dsc.p1.x = (lv_coord_t)pcx1.x;
        line_dsc.p1.y = (lv_coord_t)pcx1.y;
        line_dsc.p2.x = (lv_coord_t)pcx2.x;
        line_dsc.p2.y = (lv_coord_t)pcx2.y;
        lv_draw_line(layer, &line_dsc);
    }
    
    Vec3D_t cz1 = {-half, 0, 0};
    Vec3D_t cz2 = {half, 0, 0};
    Vec2D_t pcz1 = project_point(cz1, cam, screen_w, screen_h);
    Vec2D_t pcz2 = project_point(cz2, cam, screen_w, screen_h);
    
    bool vcz1 = (pcz1.x > -50 && pcz1.x < screen_w + 50 &&
                 pcz1.y > -50 && pcz1.y < screen_h + 50);
    bool vcz2 = (pcz2.x > -50 && pcz2.x < screen_w + 50 &&
                 pcz2.y > -50 && pcz2.y < screen_h + 50);
    
    if (vcz1 && vcz2) {
        line_dsc.p1.x = (lv_coord_t)pcz1.x;
        line_dsc.p1.y = (lv_coord_t)pcz1.y;
        line_dsc.p2.x = (lv_coord_t)pcz2.x;
        line_dsc.p2.y = (lv_coord_t)pcz2.y;
        lv_draw_line(layer, &line_dsc);
    }
}

/**
 * @brief 渲染轨迹线（支持动态绘制）
 */
static void render_track(lv_layer_t* layer, const TrackData_t* track,
                         const Camera_t* cam, int screen_w, int screen_h,
                         Vec2D_t* projected_cache,
                         TrackMode_t mode, float progress)
{
    if (track->count < 2) return;

    /* 计算实际要绘制的线段数量 */
    uint32_t total_segments = track->count - 1;
    uint32_t draw_segments = total_segments;
    
    if (mode == TRACK_MODE_DYNAMIC || mode == TRACK_MODE_LOOP) {
        draw_segments = (uint32_t)(total_segments * progress);
        if (draw_segments > total_segments) draw_segments = total_segments;
        if (draw_segments < 1) draw_segments = 1;
    }
    
    /* 投影所有点（只投影需要的部分，但为了简单，全部投影） */
    for (uint32_t i = 0; i < track->count; i++)
    {
        Vec3D_t point = vec3d_create(track->points[i].x,
                                     track->points[i].y,
                                     track->points[i].z);
        projected_cache[i] = project_point(point, cam, screen_w, screen_h);
    }

    /* 绘制线段 */
    lv_draw_line_dsc_t line_dsc;
    lv_draw_line_dsc_init(&line_dsc);
    line_dsc.width = 2;
    line_dsc.round_end = 1;
    line_dsc.round_start = 1;

    for (uint32_t i = 0; i < draw_segments; i++)
    {
        Vec2D_t p1 = projected_cache[i];
        Vec2D_t p2 = projected_cache[i + 1];

        /* 边界检查：只要有一个点在屏幕附近就绘制 */
        if ((p1.x > -50 && p1.x < screen_w + 50 &&
             p1.y > -50 && p1.y < screen_h + 50) ||
            (p2.x > -50 && p2.x < screen_w + 50 &&
             p2.y > -50 && p2.y < screen_h + 50))
        {
            /* 计算渐变色 */
            float t = (float)i / (track->count - 1);
            line_dsc.color = get_galaxy_color(t);

            /* 设置线段端点 */
            line_dsc.p1.x = (lv_coord_t)p1.x;
            line_dsc.p1.y = (lv_coord_t)p1.y;
            line_dsc.p2.x = (lv_coord_t)p2.x;
            line_dsc.p2.y = (lv_coord_t)p2.y;

            lv_draw_line(layer, &line_dsc);
        }
    }
}

/**
 * @brief 渲染定时器回调（优化：只在 dirty 且时间到达时渲染）
 */
static void render_timer_cb(lv_timer_t* timer)
{
    Renderer3D_t* renderer = (Renderer3D_t*)timer->user_data;
    if (!renderer) return;

    uint32_t now = lv_tick_get();
    uint32_t frame_interval = 1000 / renderer->target_fps;
    uint32_t elapsed = now - renderer->last_frame_time;

    /* 如果正在动态绘制，需要持续更新 */
    bool need_animation = (renderer->track_mode == TRACK_MODE_DYNAMIC ||
                           renderer->track_mode == TRACK_MODE_LOOP);
    
    if (need_animation && renderer->track) {
        /* 更新轨迹绘制进度 */
        if (renderer->track_forward) {
            renderer->track_progress += renderer->track_speed;
            if (renderer->track_progress >= 1.0f) {
                if (renderer->track_mode == TRACK_MODE_LOOP) {
                    renderer->track_progress = 0.0f;
                } else {
                    renderer->track_progress = 1.0f;
                    renderer->track_forward = false;
                }
            }
        }
        renderer->dirty = true;
    }

    /* 输入事件只更新相机并置脏，实际绘制保持固定帧率。 */
    if (renderer->dirty && elapsed >= frame_interval)
    {
        uint32_t render_start = lv_tick_get();
        renderer3d_render_frame(renderer);
        uint32_t render_elapsed = lv_tick_get() - render_start;
        renderer->last_frame_time = lv_tick_get();
        renderer->dirty = false;

        if (renderer->target_fps == 60 &&
            render_elapsed > frame_interval) {
            if (++renderer->slow_frame_count >= 3) {
                renderer->target_fps = 30;
                lv_timer_set_period(timer, 1000 / renderer->target_fps);
                printf("[Renderer3D] 渲染负载较高，降至 30 FPS\n");
            }
        } else if (renderer->target_fps == 60) {
            renderer->slow_frame_count = 0;
        }
    }
}

/**
 * @brief 设置拖拽状态
 */
void renderer3d_set_dragging(Renderer3D_t* renderer, bool dragging)
{
    if (renderer)
    {
        renderer->is_dragging = dragging;
    }
}

/**
 * @brief 标记需要重绘
 */
void renderer3d_mark_dirty(Renderer3D_t* renderer)
{
    if (renderer)
    {
        renderer->dirty = true;
    }
}

/**
 * @brief 设置轨迹绘制模式
 */
void renderer3d_set_track_mode(Renderer3D_t* renderer, TrackMode_t mode)
{
    if (!renderer) return;
    renderer->track_mode = mode;
    renderer->track_progress = 0.0f;
    renderer->track_forward = true;
    renderer->dirty = true;
}

/**
 * @brief 重置轨迹动画
 */
void renderer3d_reset_track_animation(Renderer3D_t* renderer)
{
    if (!renderer) return;
    renderer->track_progress = 0.0f;
    renderer->track_forward = true;
    renderer->dirty = true;
}

/**
 * @brief 创建渲染器
 */
Renderer3D_t* renderer3d_create(lv_obj_t* parent, TrackData_t* track)
{
    Renderer3D_t* renderer;

    if (!parent || !track)
    {
        return NULL;
    }

    renderer = malloc(sizeof(Renderer3D_t));
    if (!renderer)
    {
        return NULL;
    }

    memset(renderer, 0, sizeof(Renderer3D_t));

    /* 创建 Canvas */
    renderer->canvas = lv_canvas_create(parent);
    lv_obj_set_size(renderer->canvas, RENDERER_SCREEN_SIZE,
                    RENDERER_SCREEN_SIZE);

    /* 分配缓冲区 */
    lv_color_t* buf = malloc(RENDERER_SCREEN_SIZE * RENDERER_SCREEN_SIZE *
                             sizeof(lv_color_t));
    if (!buf)
    {
        free(renderer);
        return NULL;
    }

    renderer->framebuffer = buf;
    lv_canvas_set_buffer(renderer->canvas, buf, RENDERER_SCREEN_SIZE,
                         RENDERER_SCREEN_SIZE, LV_COLOR_FORMAT_RGB565);

    /* 初始化摄像机 */
    renderer->camera.theta = 0.5f;
    renderer->camera.phi = 0.3f;
    renderer->camera.radius = RENDERER_DEFAULT_RADIUS;
    renderer->camera.focal_length = RENDERER_DEFAULT_FOCAL_LENGTH;

    printf("[Renderer3D] 初始化: radius=%.2f, focal_length=%.2f\n",
           renderer->camera.radius, renderer->camera.focal_length);

    /* 设置其他参数 */
    renderer->track = track;
    renderer->width = RENDERER_SCREEN_SIZE;
    renderer->height = RENDERER_SCREEN_SIZE;
    renderer->target_fps = 60;
    renderer->slow_frame_count = 0;
    renderer->last_frame_time = 0;
    renderer->is_dragging = false;
    renderer->dirty = true;
    
    /* 轨迹动画参数 */
    renderer->track_mode = TRACK_MODE_DYNAMIC;  /* 默认动态绘制 */
    renderer->track_progress = 0.0f;
    renderer->track_speed = 0.008f;  /* 每帧增长 0.8% */
    renderer->track_forward = true;

    /* 生成星星 */
    renderer3d_generate_stars(renderer);

    return renderer;
}

/**
 * @brief 销毁渲染器
 */
void renderer3d_destroy(Renderer3D_t* renderer)
{
    if (!renderer) return;

    /* 停止渲染 */
    renderer3d_stop(renderer);

    /* 释放 Canvas 缓冲区 */
    if (renderer->framebuffer)
    {
        free(renderer->framebuffer);
        renderer->framebuffer = NULL;
    }

    /* 删除 Canvas */
    if (renderer->canvas)
    {
        lv_obj_del(renderer->canvas);
        renderer->canvas = NULL;
    }

    free(renderer);
}

/**
 * @brief 渲染一帧
 */
void renderer3d_render_frame(Renderer3D_t* renderer)
{
    lv_canvas_t* canvas;
    lv_layer_t layer;
    uint32_t now;

    if (!renderer || !renderer->canvas || !renderer->track)
    {
        return;
    }

    canvas = (lv_canvas_t*)renderer->canvas;
    lv_layer_init(&layer);
    lv_canvas_init_layer(canvas, &layer);

    /* 1. 清除背景（深空色） */
    lv_draw_rect_dsc_t rect_dsc;
    lv_draw_rect_dsc_init(&rect_dsc);
    rect_dsc.bg_color = lv_color_make(10, 10, 26);
    rect_dsc.bg_opa = LV_OPA_COVER;

    lv_area_t bg_area = {0, 0, renderer->width - 1, renderer->height - 1};
    lv_draw_rect(&layer, &rect_dsc, &bg_area);

    /* 2. 渲染星星（天空盒模式） */
    now = lv_tick_get();
    for (int i = 0; i < RENDERER_STAR_COUNT; i++)
    {
        render_star(&layer, &renderer->stars[i], &renderer->camera,
                   renderer->width, renderer->height, now);
    }

    /* 3. 渲染参考网格 */
    render_grid(&layer, &renderer->camera,
                renderer->width, renderer->height);

    /* 4. 渲染轨迹（支持动态绘制） */
    render_track(&layer, renderer->track, &renderer->camera,
                renderer->width, renderer->height,
                renderer->projected_cache,
                renderer->track_mode,
                renderer->track_progress);

    /* 5. 完成绘制 */
    lv_canvas_finish_layer(canvas, &layer);
    lv_obj_invalidate(renderer->canvas);
}

/**
 * @brief 启动渲染循环
 */
void renderer3d_start(Renderer3D_t* renderer)
{
    if (!renderer || renderer->render_timer)
    {
        return;
    }

    uint32_t period = 1000 / renderer->target_fps;
    renderer->render_timer = lv_timer_create(render_timer_cb, period, renderer);
}

/**
 * @brief 停止渲染循环
 */
void renderer3d_stop(Renderer3D_t* renderer)
{
    if (!renderer || !renderer->render_timer)
    {
        return;
    }

    lv_timer_delete(renderer->render_timer);
    renderer->render_timer = NULL;
}

/**
 * @brief 重置摄像机视角
 */
void renderer3d_reset_camera(Renderer3D_t* renderer)
{
    if (!renderer) return;

    renderer->camera.theta = 0.5f;
    renderer->camera.phi = 0.3f;
    renderer->camera.radius = RENDERER_DEFAULT_RADIUS;
    renderer->camera.focal_length = RENDERER_DEFAULT_FOCAL_LENGTH;
    renderer->dirty = true;
}

/**
 * @brief 处理触摸事件
 */
void renderer3d_handle_touch(Renderer3D_t* renderer,
                             int x, int y, bool pressed)
{
    static TouchState_t state = {false, 0, 0, 1.0f};

    if (!renderer) return;

    if (pressed)
    {
        if (!state.is_dragging)
        {
            state.is_dragging = true;
            state.last_x = x;
            state.last_y = y;
            printf("[Renderer3D] 开始拖拽: (%d, %d)\n", x, y);
        }
        else
        {
            int dx = x - state.last_x;
            int dy = y - state.last_y;

            if (dx != 0 || dy != 0)
            {
                /* 更新摄像机角度 */
                renderer->camera.theta += dx * 0.008f * state.sensitivity;
                renderer->camera.phi += dy * 0.008f * state.sensitivity;

                /* 限制垂直角度 */
                float max_phi = RENDERER_MAX_PHI_DEG * M_PI / 180.0f;
                renderer->camera.phi = clampf(renderer->camera.phi,
                                              -max_phi, max_phi);

                state.last_x = x;
                state.last_y = y;

                /* ⭐ 优化：只标记需要重绘，不立即执行 */
                renderer->dirty = true;
            }
        }
    }
    else
    {
        if (state.is_dragging)
        {
            printf("[Renderer3D] 结束拖拽\n");
        }
        state.is_dragging = false;
    }
}

/**
 * @brief 处理缩放事件（通过改变摄像机半径）
 */
void renderer3d_handle_zoom(Renderer3D_t* renderer, bool zoom_in)
{
    if (!renderer) return;

    float zoom_speed = 0.1f;

    if (zoom_in)
    {
        renderer->camera.radius -= zoom_speed;
        if (renderer->camera.radius < 0.3f)
            renderer->camera.radius = 0.3f;
    }
    else
    {
        renderer->camera.radius += zoom_speed;
        if (renderer->camera.radius > 5.0f)
            renderer->camera.radius = 5.0f;
    }

    printf("[Renderer3D] 缩放: radius=%.2f\n", renderer->camera.radius);
    renderer->dirty = true;
}

/**
 * @brief 生成星星（支持多种风格，每次进入不同）
 */
void renderer3d_generate_stars(Renderer3D_t* renderer)
{
    float golden_ratio = (1.0f + sqrtf(5.0f)) / 2.0f;

    if (!renderer) return;

    /* ⭐ 使用时间作为随机种子，每次进入都不同 */
    srand((unsigned int)lv_tick_get());

    /* 随机选择一种风格 */
    StarStyle_t style = (StarStyle_t)(rand() % 4);
    
    /* 根据风格调整参数 */
    int star_count = RENDERER_STAR_COUNT;
    float size_scale = 1.0f;
    float brightness_scale = 1.0f;
    float concentration = 0.0f;  /* 0 = 均匀分布，1 = 集中在银河平面 */

    switch (style) {
        case STAR_STYLE_DENSE:
            star_count = RENDERER_STAR_COUNT * 1.5f;
            if (star_count > RENDERER_STAR_COUNT) 
                star_count = RENDERER_STAR_COUNT;
            size_scale = 0.8f;
            printf("[Renderer3D] 星空风格: 密集\n");
            break;
        case STAR_STYLE_SPARSE:
            star_count = RENDERER_STAR_COUNT * 0.6f;
            size_scale = 1.5f;
            brightness_scale = 1.3f;
            printf("[Renderer3D] 星空风格: 稀疏\n");
            break;
        case STAR_STYLE_GALAXY:
            concentration = 0.8f;
            printf("[Renderer3D] 星空风格: 银河\n");
            break;
        case STAR_STYLE_NORMAL:
        default:
            printf("[Renderer3D] 星空风格: 普通\n");
            break;
    }

    for (int i = 0; i < star_count; i++)
    {
        float theta = 2.0f * M_PI * i / golden_ratio;
        float phi;
        
        if (concentration > 0.0f) {
            /* 银河风格：phi 集中在 0 附近 */
            float random_offset = (rand() / (float)RAND_MAX - 0.5f) * 0.5f * (1.0f - concentration);
            phi = random_offset + (rand() / (float)RAND_MAX - 0.5f) * 0.3f * concentration;
            if (phi > M_PI/2) phi = M_PI/2;
            if (phi < -M_PI/2) phi = -M_PI/2;
        } else {
            /* 均匀分布 */
            phi = asinf(1.0f - 2.0f * (i + 0.5f) / star_count);
        }

        renderer->stars[i].theta = theta;
        renderer->stars[i].phi = phi;
        renderer->stars[i].size = (rand() % 2 + 1) * size_scale;
        renderer->stars[i].phase = (float)rand() / RAND_MAX * 2.0f * M_PI;
        renderer->stars[i].speed = 0.3f + (float)rand() / RAND_MAX * 0.9f;
        renderer->stars[i].base_brightness = (0.4f + (float)rand() / RAND_MAX * 0.6f) * brightness_scale;
        if (renderer->stars[i].base_brightness > 1.0f) 
            renderer->stars[i].base_brightness = 1.0f;
        renderer->stars[i].color_type = rand() % 14;
    }
}

/**
 * @brief 强制重新生成星星（用于刷新星空）
 */
void renderer3d_regenerate_stars(Renderer3D_t* renderer)
{
    if (!renderer) return;
    renderer3d_generate_stars(renderer);
    renderer->dirty = true;
}