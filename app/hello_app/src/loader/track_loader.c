/**
 * @file track_loader.c
 * @brief 轨迹文件加载器实现
 * @author Velatrix Team (357)
 * @version 1.0
 */

#include "track_loader.h"
#include <stdio.h>
#include <string.h>

/**
 * @brief 读取小端 uint16_t
 */
static int read_le16(FILE* file, uint16_t* value)
{
    uint8_t buf[2];
    if (fread(buf, 1, sizeof(buf), file) != sizeof(buf)) return 0;
    *value = (uint16_t)buf[0] | ((uint16_t)buf[1] << 8);
    return 1;
}

/**
 * @brief 读取小端 uint32_t
 */
static int read_le32(FILE* file, uint32_t* value)
{
    uint8_t buf[4];
    if (fread(buf, 1, sizeof(buf), file) != sizeof(buf)) return 0;
    *value = (uint32_t)buf[0] | ((uint32_t)buf[1] << 8) |
             ((uint32_t)buf[2] << 16) | ((uint32_t)buf[3] << 24);
    return 1;
}

/**
 * @brief 读取小端 float
 */
static int read_le_float(FILE* file, float* value)
{
    uint32_t val;
    if (!read_le32(file, &val)) return 0;
    memcpy(value, &val, sizeof(*value));
    return 1;
}

/**
 * @brief 加载 bin 文件
 */
bool track_loader_load(const char* filename, TrackData_t* track)
{
    FILE* file;
    uint32_t magic;
    uint16_t version;
    uint16_t flags;
    uint16_t point_count;
    uint16_t name_length;
    float metadata[4];

    if (!filename || !track)
    {
        printf("[TrackLoader] 参数错误\n");
        return false;
    }

    /* 清空轨迹数据 */
    track_loader_clear(track);

    /* 打开文件 */
    file = fopen(filename, "rb");
    if (!file)
    {
        printf("[TrackLoader] 无法打开文件: %s\n", filename);
        return false;
    }

    /* 读取并验证魔数 "VELX" (4 bytes) */
    if (!read_le32(file, &magic)) {
        fclose(file);
        return false;
    }
    if (magic != TRACK_MAGIC)
    {
        printf("[TrackLoader] 无效的文件格式 (magic: 0x%08X, expected: 0x%08X)\n",
               magic, TRACK_MAGIC);
        fclose(file);
        return false;
    }

    /* 读取并验证版本号 (2 bytes) */
    if (!read_le16(file, &version) || !read_le16(file, &flags) ||
        !read_le16(file, &point_count) || !read_le16(file, &name_length))
    {
        printf("[TrackLoader] 文件头不完整\n");
        fclose(file);
        return false;
    }

    (void)flags;
    if (version != TRACK_VERSION) {
        printf("[TrackLoader] 不支持的版本 0x%04X (expected: 0x%04X)\n",
               version, TRACK_VERSION);
        fclose(file);
        return false;
    }

    if (fseek(file, name_length, SEEK_CUR) != 0) {
        fclose(file);
        return false;
    }

    for (size_t i = 0; i < sizeof(metadata) / sizeof(metadata[0]); i++) {
        if (!read_le_float(file, &metadata[i])) {
            fclose(file);
            return false;
        }
    }

    /* 读取轨迹点 (每个点 12 bytes: float x 3) */
    for (uint16_t i = 0; i < point_count; i++)
    {
        if (!read_le_float(file, &track->points[i].x) ||
            !read_le_float(file, &track->points[i].y) ||
            !read_le_float(file, &track->points[i].z)) {
            printf("[TrackLoader] 轨迹点数据不完整\n");
            fclose(file);
            track_loader_clear(track);
            return false;
        }
    }

    track->count = point_count;
    track->alt_min = metadata[2];
    track->alt_max = metadata[3];

    fclose(file);

    printf("[TrackLoader] 成功加载 %d 个轨迹点\n", track->count);

    return true;
}

/**
 * @brief 获取轨迹点
 */
const TrackPoint3D_t* track_loader_get_point(const TrackData_t* track,
                                             uint32_t index)
{
    if (!track || index >= track->count)
    {
        return NULL;
    }

    return &track->points[index];
}

/**
 * @brief 获取轨迹点数量
 */
uint32_t track_loader_get_count(const TrackData_t* track)
{
    if (!track)
    {
        return 0;
    }

    return track->count;
}

/**
 * @brief 清空轨迹数据
 */
void track_loader_clear(TrackData_t* track)
{
    if (track)
    {
        memset(track, 0, sizeof(TrackData_t));
    }
}
