/**
 * @file math3d.c
 * @brief 3D 数学库实现
 * @author Velatrix Team (357)
 * @version 1.0
 */

#include "../../inc/core/math3d.h"

/**
 * @brief 获取摄像机位置
 */
Vec3D_t camera_get_position(const Camera_t* cam)
{
    Vec3D_t pos;
    pos.x = cam->radius * cosf(cam->phi) * sinf(cam->theta);
    pos.y = cam->radius * sinf(cam->phi);
    pos.z = cam->radius * cosf(cam->phi) * cosf(cam->theta);
    return pos;
}

/**
 * @brief 获取视线方向（从摄像机指向原点）
 */
Vec3D_t camera_get_view_direction(const Camera_t* cam)
{
    Vec3D_t pos = camera_get_position(cam);
    float dist = vec3d_length(pos);

    if (dist < 0.0001f)
    {
        return vec3d_create(0, 0, 1);
    }

    return vec3d_scale(pos, -1.0f / dist);
}

/**
 * @brief 获取上方向
 */
Vec3D_t camera_get_up_direction(const Camera_t* cam)
{
    Vec3D_t up = vec3d_create(0, 1, 0);
    Vec3D_t view = camera_get_view_direction(cam);

    /* 如果视线方向接近垂直，使用不同的上方向 */
    if (fabsf(view.y) > 0.99f)
    {
        up = vec3d_create(0, 0, 1);
    }

    /* Gram-Schmidt 正交化 */
    float dot = vec3d_dot(up, view);
    up = vec3d_sub(up, vec3d_scale(view, dot));

    return vec3d_normalize(up);
}

/**
 * @brief 获取右方向
 */
Vec3D_t camera_get_right_direction(const Camera_t* cam)
{
    Vec3D_t view = camera_get_view_direction(cam);
    Vec3D_t up = camera_get_up_direction(cam);
    Vec3D_t right = vec3d_cross(up, view);

    return vec3d_normalize(right);
}

/**
 * @brief 透视投影（NDC 标准流程）
 */
Vec2D_t project_point(Vec3D_t point, const Camera_t* cam, int screen_w, int screen_h)
{
    Vec3D_t cam_pos = camera_get_position(cam);
    Vec3D_t view = camera_get_view_direction(cam);
    Vec3D_t up = camera_get_up_direction(cam);
    Vec3D_t right = camera_get_right_direction(cam);

    /* 计算相对位置 */
    Vec3D_t delta = vec3d_sub(point, cam_pos);

    /* 转换到视图空间 */
    float rel_x = vec3d_dot(delta, right);
    float rel_y = vec3d_dot(delta, up);
    float rel_z = vec3d_dot(delta, view);

    /* 调试打印 */
    static int debug_count = 0;
    if (debug_count < 5) {
        printf("[Math3D] point: (%.2f, %.2f, %.2f)\n", point.x, point.y, point.z);
        printf("[Math3D] cam_pos: (%.2f, %.2f, %.2f)\n", cam_pos.x, cam_pos.y, cam_pos.z);
        printf("[Math3D] rel: (%.2f, %.2f, %.2f)\n", rel_x, rel_y, rel_z);
        printf("[Math3D] focal_length: %.2f\n", cam->focal_length);
        debug_count++;
    }

    /* 透视除法 */
    if (rel_z < 0.1f)
    {
        return vec2d_create(-1000, -1000);  /* 在摄像机后面 */
    }

    /* 透视投影到 NDC [-1, 1] */
    float ndc_x = (rel_x / rel_z) * cam->focal_length;
    float ndc_y = -(rel_y / rel_z) * cam->focal_length;

    /* NDC 到屏幕坐标 */
    float px = (ndc_x + 1.0f) * screen_w / 2.0f;
    float py = (ndc_y + 1.0f) * screen_h / 2.0f;

    if (debug_count <= 5) {
        printf("[Math3D] ndc: (%.2f, %.2f)\n", ndc_x, ndc_y);
        printf("[Math3D] screen: (%.2f, %.2f)\n", px, py);
        debug_count++;
    }

    return vec2d_create(px, py);
}
