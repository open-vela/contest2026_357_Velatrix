/**
 * @file track_loader.h
 * @brief 轨迹文件加载器 - 支持 track_visualizer v2.0 bin 文件解析
 * @author Velatrix Team (357)
 * @version 1.0
 */

#ifndef __TRACK_LOADER_H
#define __TRACK_LOADER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief bin 文件魔数 "VELX" (小端格式)
 */
#define TRACK_MAGIC 0x584C4556  /* "VELX" little-endian */

/**
 * @brief bin 文件版本
 */
#define TRACK_VERSION 0x0200

/**
 * @brief 最大轨迹点数
 */
#define TRACK_MAX_POINTS 65535

/**
 * @brief v2.0 文件头（后续还有名称和元数据字段）
 */
#pragma pack(push, 1)
typedef struct
{
    uint32_t magic;          /**< 魔数 "VELX" */
    uint16_t version;        /**< 版本号 0x0100 */
    uint16_t flags;          /**< 文件标志 */
    uint16_t point_count;    /**< 轨迹点数量 */
    uint16_t name_length;    /**< UTF-8 名称长度 */
} TrackFileHeader_t;
#pragma pack(pop)

/**
 * @brief 3D 轨迹点结构体
 */
typedef struct
{
    float x;                 /**< 归一化 X 坐标 [-1, 1] */
    float y;                 /**< 归一化 Y 坐标（海拔） */
    float z;                 /**< 归一化 Z 坐标 [-1, 1] */
} TrackPoint3D_t;

/**
 * @brief 轨迹数据结构体
 */
typedef struct
{
    TrackPoint3D_t points[TRACK_MAX_POINTS];  /**< 归一化后的 3D 坐标 */
    uint32_t count;           /**< 点数量 */
    float alt_min;            /**< 最小海拔 */
    float alt_max;            /**< 最大海拔 */
} TrackData_t;

/**
 * @brief 加载 bin 文件
 *
 * @param filename 文件路径
 * @param track 轨迹数据指针
 * @return true 成功
 * @return false 失败
 */
bool track_loader_load(const char* filename, TrackData_t* track);

/**
 * @brief 获取轨迹点
 *
 * @param track 轨迹数据指针
 * @param index 点索引
 * @return TrackPoint3D_t* 点指针，索引无效返回 NULL
 */
const TrackPoint3D_t* track_loader_get_point(const TrackData_t* track,
                                             uint32_t index);

/**
 * @brief 获取轨迹点数量
 *
 * @param track 轨迹数据指针
 * @return uint32_t 点数量
 */
uint32_t track_loader_get_count(const TrackData_t* track);

/**
 * @brief 清空轨迹数据
 *
 * @param track 轨迹数据指针
 */
void track_loader_clear(TrackData_t* track);

#ifdef __cplusplus
}
#endif

#endif /* __TRACK_LOADER_H */
