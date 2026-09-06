/**
 * @file track_registry.h
 * @brief 简单的轨迹文件注册表 (key -> path)，运行时扫描目录生成条目
 */

#ifndef __TRACK_REGISTRY_H
#define __TRACK_REGISTRY_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* key;   /* 唯一 key，例如文件名不含扩展 */
    const char* name;  /* 显示名称，默认等于文件名 */
    const char* path;  /* 相对或绝对路径到轨迹文件 */
} TrackEntry_t;

/* 初始化注册表，dir 为要扫描的目录（例如 "./data"）。
 * 调用一次，返回 0 成功，非 0 表示出错（目录不可读等）。
 */
int track_registry_init(const char* dir);

/* 释放注册表占用的资源 */
void track_registry_deinit(void);

/* 返回注册表条目数量 */
int track_registry_count(void);

/* 按索引获取条目，如果越界返回 NULL */
const TrackEntry_t* track_registry_get(int index);

#ifdef __cplusplus
}
#endif

#endif /* __TRACK_REGISTRY_H */
