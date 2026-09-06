/**
 * @file math3d.h
 * @brief 3D 数学库
 * @author Velatrix Team (357)
 * @version 1.0
 */

#ifndef __MATH3D_H
#define __MATH3D_H

#include <stdint.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 圆周率
 */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * @brief 3D 向量结构体
 */
typedef struct
{
    float x;
    float y;
    float z;
} Vec3D_t;

/**
 * @brief 2D 点结构体
 */
typedef struct
{
    float x;
    float y;
} Vec2D_t;

/**
 * @brief 摄像机结构体
 */
typedef struct
{
    float theta;          /**< 水平角度（弧度） */
    float phi;            /**< 垂直角度（弧度） */
    float radius;         /**< 摄像机距离 */
    float focal_length;   /**< 焦距 */
} Camera_t;

/**
 * @brief 创建 3D 向量
 *
 * @param x X 坐标
 * @param y Y 坐标
 * @param z Z 坐标
 * @return Vec3D_t 向量
 */
static inline Vec3D_t vec3d_create(float x, float y, float z)
{
    Vec3D_t v = {x, y, z};
    return v;
}

/**
 * @brief 创建 2D 点
 *
 * @param x X 坐标
 * @param y Y 坐标
 * @return Vec2D_t 点
 */
static inline Vec2D_t vec2d_create(float x, float y)
{
    Vec2D_t p = {x, y};
    return p;
}

/**
 * @brief 向量加法
 */
static inline Vec3D_t vec3d_add(Vec3D_t a, Vec3D_t b)
{
    return vec3d_create(a.x + b.x, a.y + b.y, a.z + b.z);
}

/**
 * @brief 向量减法
 */
static inline Vec3D_t vec3d_sub(Vec3D_t a, Vec3D_t b)
{
    return vec3d_create(a.x - b.x, a.y - b.y, a.z - b.z);
}

/**
 * @brief 向量缩放
 */
static inline Vec3D_t vec3d_scale(Vec3D_t v, float s)
{
    return vec3d_create(v.x * s, v.y * s, v.z * s);
}

/**
 * @brief 向量点积
 */
static inline float vec3d_dot(Vec3D_t a, Vec3D_t b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

/**
 * @brief 向量叉积
 */
static inline Vec3D_t vec3d_cross(Vec3D_t a, Vec3D_t b)
{
    return vec3d_create(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

/**
 * @brief 向量长度
 */
static inline float vec3d_length(Vec3D_t v)
{
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

/**
 * @brief 向量归一化
 */
static inline Vec3D_t vec3d_normalize(Vec3D_t v)
{
    float len = vec3d_length(v);
    if (len > 0.0001f)
    {
        return vec3d_scale(v, 1.0f / len);
    }
    return v;
}

/**
 * @brief 获取摄像机位置
 *
 * @param cam 摄像机指针
 * @return Vec3D_t 摄像机位置
 */
Vec3D_t camera_get_position(const Camera_t* cam);

/**
 * @brief 获取视线方向（从摄像机指向原点）
 *
 * @param cam 摄像机指针
 * @return Vec3D_t 视线方向
 */
Vec3D_t camera_get_view_direction(const Camera_t* cam);

/**
 * @brief 获取上方向
 *
 * @param cam 摄像机指针
 * @return Vec3D_t 上方向
 */
Vec3D_t camera_get_up_direction(const Camera_t* cam);

/**
 * @brief 获取右方向
 *
 * @param cam 摄像机指针
 * @return Vec3D_t 右方向
 */
Vec3D_t camera_get_right_direction(const Camera_t* cam);

/**
 * @brief 透视投影
 *
 * @param point 3D 点
 * @param cam 摄像机指针
 * @param screen_w 屏幕宽度
 * @param screen_h 屏幕高度
 * @return Vec2D_t 屏幕坐标
 */
Vec2D_t project_point(Vec3D_t point, const Camera_t* cam, int screen_w, int screen_h);

/**
 * @brief 限制角度到 [-PI, PI]
 */
static inline float clamp_angle(float angle)
{
    while (angle > M_PI) angle -= 2.0f * M_PI;
    while (angle < -M_PI) angle += 2.0f * M_PI;
    return angle;
}

/**
 * @brief 限制值到 [min, max]
 */
static inline float clampf(float value, float min, float max)
{
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

#ifdef __cplusplus
}
#endif

#endif /* __MATH3D_H */
