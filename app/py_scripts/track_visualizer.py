#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Pygame 3D轨迹模拟器 - 手表端模拟
支持解析 KML 元数据（名称、距离、耗时、海拔）
导出包含元数据的二进制文件 v2.0
"""

import pygame
import math
import xml.etree.ElementTree as ET
from dataclasses import dataclass
from typing import List, Tuple, Optional
import random
import sys
import os
import struct
import re


# ============================================================
# 配置
# ============================================================
SCREEN_SIZE = 480
EPSILON = 0.002
HEIGHT_SCALE = 0.3
STAR_COUNT = 300
CAMERA_RADIUS = 2.0
STAR_RADIUS = 1000.0
MAX_PHI_DEG = 80
TARGET_FPS = 30
TRACK_SCALE = 0.95

# 二进制文件版本
BINARY_VERSION = 0x0200
MAGIC = b'VELX'


# ============================================================
# 数据结构
# ============================================================
@dataclass
class Point3D:
    x: float
    y: float
    z: float


@dataclass
class Point2D:
    x: float
    y: float


@dataclass
class TrackMetadata:
    """轨迹元数据"""
    name: str = "未命名轨迹"
    total_distance_km: float = 0.0
    total_time_min: float = 0.0
    min_altitude: float = 0.0
    max_altitude: float = 0.0
    point_count: int = 0


# ============================================================
# KML解析 - 增强版，支持元数据
# ============================================================
def parse_kml(file_path: str) -> Tuple[List[Tuple[float, float, float]], TrackMetadata]:
    """
    解析KML文件，返回 (轨迹点列表, 元数据)
    支持:
    - 从 <name> 读取轨迹名称
    - 从 <ExtendedData> 或 <description> 读取距离/时间
    """
    if not os.path.exists(file_path):
        raise FileNotFoundError(f"文件不存在: {file_path}")
    
    tree = ET.parse(file_path)
    root = tree.getroot()
    ns = {
        'kml': 'http://www.opengis.net/kml/2.2',
        'gx': 'http://www.google.com/kml/ext/2.2',
    }
    
    coordinates = []
    metadata = TrackMetadata()
    
    def append_coord_tuple(lon, lat, alt):
        try:
            coordinates.append((float(lon), float(lat), float(alt)))
        except ValueError:
            return
    
    # ---- 1. 解析轨迹名称 ----
    name_elem = root.find('.//kml:name', ns)
    if name_elem is not None and name_elem.text:
        metadata.name = name_elem.text.strip()
    
    # ---- 2. 解析轨迹坐标 ----
    for placemark in root.findall('.//kml:Placemark', ns):
        # 尝试从Placemark获取名称（覆盖根名称）
        pm_name = placemark.find('.//kml:name', ns)
        if pm_name is not None and pm_name.text and pm_name.text.strip():
            metadata.name = pm_name.text.strip()

        for linestring in placemark.findall('.//kml:LineString', ns):
            for coord_elem in linestring.findall('.//kml:coordinates', ns):
                if coord_elem.text:
                    coord_text = coord_elem.text.strip()
                    for coord in coord_text.replace('\n', ' ').split():
                        coord = coord.strip()
                        if not coord:
                            continue
                        parts = coord.split(',')
                        if len(parts) >= 2:
                            append_coord_tuple(parts[0], parts[1], parts[2] if len(parts) >= 3 else 0.0)

        # 兼容 Google gx:Track（如 phoenix.kml / squirrel.kml）
        for track in placemark.findall('.//gx:Track', ns):
            for coord_elem in track.findall('.//gx:coord', ns):
                if coord_elem.text:
                    coord_text = coord_elem.text.strip()
                    parts = coord_text.split()
                    if len(parts) >= 3:
                        append_coord_tuple(parts[0], parts[1], parts[2])
                    elif len(parts) >= 2:
                        append_coord_tuple(parts[0], parts[1], 0.0)

    # 兼容根级别/非Placemark的 gx:Track 和 kml:LineString
    for track in root.findall('.//gx:Track', ns):
        for coord_elem in track.findall('.//gx:coord', ns):
            if coord_elem.text:
                coord_text = coord_elem.text.strip()
                parts = coord_text.split()
                if len(parts) >= 3:
                    append_coord_tuple(parts[0], parts[1], parts[2])
                elif len(parts) >= 2:
                    append_coord_tuple(parts[0], parts[1], 0.0)

    for linestring in root.findall('.//kml:LineString', ns):
        for coord_elem in linestring.findall('.//kml:coordinates', ns):
            if coord_elem.text:
                coord_text = coord_elem.text.strip()
                for coord in coord_text.replace('\n', ' ').split():
                    coord = coord.strip()
                    if not coord:
                        continue
                    parts = coord.split(',')
                    if len(parts) >= 2:
                        append_coord_tuple(parts[0], parts[1], parts[2] if len(parts) >= 3 else 0.0)
    
    # ---- 3. 解析元数据 ----
    # 3.1 尝试从 ExtendedData 读取
    ext_data = root.find('.//kml:ExtendedData', ns)
    if ext_data is not None:
        for data in ext_data.findall('.//kml:Data', ns):
            name_attr = data.get('name', '')
            value_elem = data.find('.//kml:value', ns)
            if value_elem is None or not value_elem.text:
                continue
            value = value_elem.text.strip()
            
            # 尝试匹配各种字段名
            if '距离' in name_attr or 'distance' in name_attr.lower() or '里程' in name_attr:
                try:
                    metadata.total_distance_km = float(re.sub(r'[^\d.]', '', value))
                except:
                    pass
            elif '耗时' in name_attr or 'time' in name_attr.lower() or '时长' in name_attr:
                try:
                    metadata.total_time_min = parse_time_string(value)
                except:
                    pass
    
    # 3.2 尝试从 description 解析
    desc_elem = root.find('.//kml:description', ns)
    if desc_elem is not None and desc_elem.text:
        desc = desc_elem.text
        # 尝试匹配 "距离: 13.97km"
        match = re.search(r'距离[:：]\s*([\d.]+)\s*km', desc, re.I)
        if match:
            metadata.total_distance_km = float(match.group(1))
        # 尝试匹配 "耗时: 1h0min"
        match = re.search(r'耗时[:：]\s*([\d.]+)\s*h\s*([\d.]+)\s*min', desc, re.I)
        if match:
            metadata.total_time_min = float(match.group(1)) * 60 + float(match.group(2))
        else:
            match = re.search(r'耗时[:：]\s*([\d.]+)\s*min', desc, re.I)
            if match:
                metadata.total_time_min = float(match.group(1))
    
    # 3.3 如果还没有距离，从轨迹点计算
    if metadata.total_distance_km == 0.0 and len(coordinates) >= 2:
        total_m = 0.0
        for i in range(1, len(coordinates)):
            lon1, lat1, alt1 = coordinates[i-1]
            lon2, lat2, alt2 = coordinates[i]
            dx = (lon2 - lon1) * 111320  # 1度经度 ≈ 111.32km
            dy = (lat2 - lat1) * 111320
            dz = alt2 - alt1
            total_m += math.sqrt(dx*dx + dy*dy + dz*dz)
        metadata.total_distance_km = total_m / 1000.0
    
    # 3.4 计算海拔范围
    if coordinates:
        alts = [p[2] for p in coordinates]
        metadata.min_altitude = min(alts)
        metadata.max_altitude = max(alts)
    
    metadata.point_count = len(coordinates)
    
    return coordinates, metadata


def parse_time_string(s: str) -> float:
    """解析时间字符串，返回分钟数"""
    s = s.strip().lower()
    minutes = 0.0
    
    # 匹配 "1h23min" 或 "1h 23min"
    match = re.match(r'(\d+(?:\.\d+)?)\s*h(?:\s*(\d+(?:\.\d+)?)\s*min)?', s)
    if match:
        hours = float(match.group(1))
        minutes = float(match.group(2)) if match.group(2) else 0
        return hours * 60 + minutes
    
    # 匹配 "83min"
    match = re.match(r'(\d+(?:\.\d+)?)\s*min', s)
    if match:
        return float(match.group(1))
    
    # 匹配 "1:23:45"
    match = re.match(r'(\d+):(\d+):(\d+)', s)
    if match:
        return int(match.group(1)) * 60 + int(match.group(2)) + int(match.group(3)) / 60
    
    return 0.0


# ============================================================
# 道格拉斯-普克抽稀算法
# ============================================================
def perpendicular_distance(point: Point3D, start: Point3D, end: Point3D) -> float:
    if start == end:
        return math.sqrt((point.x - start.x)**2 + (point.y - start.y)**2 + (point.z - start.z)**2)
    
    dx = end.x - start.x
    dy = end.y - start.y
    dz = end.z - start.z
    
    t = ((point.x - start.x) * dx + (point.y - start.y) * dy + (point.z - start.z) * dz) / (dx*dx + dy*dy + dz*dz)
    t = max(0, min(1, t))
    
    proj_x = start.x + t * dx
    proj_y = start.y + t * dy
    proj_z = start.z + t * dz
    
    return math.sqrt((point.x - proj_x)**2 + (point.y - proj_y)**2 + (point.z - proj_z)**2)


def douglas_peucker(points: List[Point3D], epsilon: float) -> List[Point3D]:
    if len(points) <= 2:
        return points
    
    start = points[0]
    end = points[-1]
    
    max_dist = 0
    max_idx = 0
    for i in range(1, len(points) - 1):
        dist = perpendicular_distance(points[i], start, end)
        if dist > max_dist:
            max_dist = dist
            max_idx = i
    
    if max_dist > epsilon:
        left = douglas_peucker(points[:max_idx+1], epsilon)
        right = douglas_peucker(points[max_idx:], epsilon)
        return left[:-1] + right
    else:
        return [start, end]


# ============================================================
# 坐标归一化
# ============================================================
def normalize_coordinates(raw_points: List[Tuple[float, float, float]]) -> Tuple[List[Point3D], float, float]:
    if not raw_points:
        return [], 0, 0
    
    lons = [p[0] for p in raw_points]
    lats = [p[1] for p in raw_points]
    alts = [p[2] for p in raw_points]
    
    lon_min, lon_max = min(lons), max(lons)
    lat_min, lat_max = min(lats), max(lats)
    lon_range = max(lon_max - lon_min, 0.0001)
    lat_range = max(lat_max - lat_min, 0.0001)
    
    center_lon = (lon_min + lon_max) / 2
    center_lat = (lat_min + lat_max) / 2
    
    alt_min = min(alts)
    alt_max = max(alts)
    alt_range = max(alt_max - alt_min, 1.0)
    
    normalized = []
    for lon, lat, alt in raw_points:
        x = (lon - center_lon) / lon_range * TRACK_SCALE
        z = (lat - center_lat) / lat_range * TRACK_SCALE
        y = (alt - alt_min) / alt_range * HEIGHT_SCALE
        normalized.append(Point3D(x, y, z))
    
    # 中心对齐
    cx = sum(p.x for p in normalized) / len(normalized)
    cy = sum(p.y for p in normalized) / len(normalized)
    cz = sum(p.z for p in normalized) / len(normalized)
    for p in normalized:
        p.x -= cx
        p.y -= cy
        p.z -= cz
    
    return normalized, alt_min, alt_max


# ============================================================
# 导出二进制文件 (v2.0 - 包含元数据)
# ============================================================
def export_binary(points: List[Point3D], metadata: TrackMetadata, output_path: str):
    """
    导出归一化坐标点为二进制文件 (v2.0)
    
    文件格式:
    +--------+---------+-------+---------+--------+-------------------+----------+
    | Magic  | Version | Flags | Count   | NameLen| Name              | Distance |
    | 4 bytes| 2 bytes |2 bytes| 2 bytes | 2 bytes| N bytes (UTF-8)   | 4 bytes  |
    +--------+---------+-------+---------+--------+-------------------+----------+
    | Time   | MinAlt  | MaxAlt| Points (float x 3)                         |
    | 4 bytes| 4 bytes |4 bytes| 12 bytes per point                        |
    +--------+---------+-------+-------------------------------------------+
    """
    count = len(points)
    if count > 65535:
        print(f"警告: 点数 {count} 超过65535，截断")
        count = 65535
        points = points[:count]
    
    name_bytes = metadata.name.encode('utf-8')
    name_len = len(name_bytes)
    if name_len > 65535:
        name_len = 65535
        name_bytes = name_bytes[:name_len]
    
    # 限制距离和时间范围
    distance = min(max(metadata.total_distance_km, 0.0), 99999.0)
    time_min = min(max(metadata.total_time_min, 0.0), 99999.0)
    min_alt = min(max(metadata.min_altitude, -9999.0), 9999.0)
    max_alt = min(max(metadata.max_altitude, -9999.0), 9999.0)
    
    with open(output_path, 'wb') as f:
        # Magic: "VELX"
        f.write(MAGIC)
        # Version: 0x0200
        f.write(struct.pack('<H', BINARY_VERSION))
        # Flags (预留)
        f.write(struct.pack('<H', 0))
        # Point Count
        f.write(struct.pack('<H', count))
        # Name Length
        f.write(struct.pack('<H', name_len))
        # Name (UTF-8)
        f.write(name_bytes)
        # Total Distance (km)
        f.write(struct.pack('<f', distance))
        # Total Time (minutes)
        f.write(struct.pack('<f', time_min))
        # Min Altitude (m)
        f.write(struct.pack('<f', min_alt))
        # Max Altitude (m)
        f.write(struct.pack('<f', max_alt))
        # Points
        for p in points:
            f.write(struct.pack('<fff', p.x, p.y, p.z))
    
    print(f"导出成功: {output_path}")
    print(f"  点数: {count}")
    print(f"  名称: {metadata.name}")
    print(f"  距离: {distance:.2f} km")
    print(f"  耗时: {time_min:.1f} min")
    print(f"  海拔: {min_alt:.1f}m ~ {max_alt:.1f}m")
    print(f"  文件大小: {os.path.getsize(output_path)} 字节")


def export_binary_from_file(kml_path: str, output_path: str = None):
    """从KML文件直接导出二进制"""
    if output_path is None:
        base = os.path.splitext(kml_path)[0]
        output_path = base + ".bin"
    
    raw, metadata = parse_kml(kml_path)
    if len(raw) < 2:
        print("错误: 轨迹点数不足")
        return False
    
    normalized, alt_min, alt_max = normalize_coordinates(raw)
    points = douglas_peucker(normalized, EPSILON)
    
    if len(points) < 2:
        print("错误: 抽稀后轨迹点数不足")
        return False
    
    # 更新元数据中的海拔
    if metadata.min_altitude == 0 and metadata.max_altitude == 0:
        alts = [p[2] for p in raw]
        metadata.min_altitude = min(alts)
        metadata.max_altitude = max(alts)
    metadata.point_count = len(points)
    
    export_binary(points, metadata, output_path)
    return True


# ============================================================
# 3D 渲染引擎
# ============================================================
@dataclass
class Camera:
    theta: float
    phi: float
    radius: float = 2.0
    
    def get_position(self) -> Point3D:
        x = self.radius * math.cos(self.phi) * math.sin(self.theta)
        y = self.radius * math.sin(self.phi)
        z = self.radius * math.cos(self.phi) * math.cos(self.theta)
        return Point3D(x, y, z)
    
    def get_view_direction(self) -> Point3D:
        pos = self.get_position()
        dist = math.sqrt(pos.x**2 + pos.y**2 + pos.z**2)
        if dist == 0:
            return Point3D(0, 0, 1)
        return Point3D(-pos.x/dist, -pos.y/dist, -pos.z/dist)
    
    def get_up_direction(self) -> Point3D:
        up = Point3D(0, 1, 0)
        view = self.get_view_direction()
        if abs(view.y) > 0.99:
            up = Point3D(0, 0, 1)
        up_dot_view = up.x * view.x + up.y * view.y + up.z * view.z
        up = Point3D(up.x - up_dot_view * view.x,
                     up.y - up_dot_view * view.y,
                     up.z - up_dot_view * view.z)
        norm = math.sqrt(up.x**2 + up.y**2 + up.z**2)
        if norm > 0:
            up.x /= norm
            up.y /= norm
            up.z /= norm
        return up
    
    def get_right_direction(self) -> Point3D:
        view = self.get_view_direction()
        up = self.get_up_direction()
        right = Point3D(
            up.y * view.z - up.z * view.y,
            up.z * view.x - up.x * view.z,
            up.x * view.y - up.y * view.x
        )
        norm = math.sqrt(right.x**2 + right.y**2 + right.z**2)
        if norm > 0:
            right.x /= norm
            right.y /= norm
            right.z /= norm
        return right


def project_point(point: Point3D, camera: Camera, focal_length: float) -> Point2D:
    cam_pos = camera.get_position()
    view = camera.get_view_direction()
    up = camera.get_up_direction()
    right = camera.get_right_direction()
    
    dx = point.x - cam_pos.x
    dy = point.y - cam_pos.y
    dz = point.z - cam_pos.z
    
    rel_x = dx * right.x + dy * right.y + dz * right.z
    rel_y = dx * up.x + dy * up.y + dz * up.z
    rel_z = dx * view.x + dy * view.y + dz * view.z
    
    if rel_z < 0.01:
        return Point2D(-1000, -1000)
    
    px = (rel_x / rel_z) * focal_length + SCREEN_SIZE // 2
    py = -(rel_y / rel_z) * focal_length + SCREEN_SIZE // 2
    
    return Point2D(px, py)


# ============================================================
# 天空盒星星
# ============================================================
@dataclass
class SkyStar:
    theta: float
    phi: float
    size: int
    phase: float
    speed: float
    base_brightness: float
    color_type: int


def generate_sky_stars(count: int) -> List[SkyStar]:
    stars = []
    color_types = [0, 1, 2, 3, 4, 5]
    color_weights = [30, 25, 15, 15, 10, 5]
    golden_ratio = (1 + math.sqrt(5)) / 2
    for i in range(count):
        theta = 2 * math.pi * i / golden_ratio
        phi = math.asin(1 - 2 * (i + 0.5) / count)
        stars.append(SkyStar(
            theta=theta,
            phi=phi,
            size=random.randint(1, 2),
            phase=random.uniform(0, 2 * math.pi),
            speed=random.uniform(0.3, 1.2),
            base_brightness=random.uniform(0.4, 1.0),
            color_type=random.choices(color_types, weights=color_weights)[0]
        ))
    return stars


def get_star_color(color_type: int, brightness: float) -> Tuple[int, int, int]:
    base_colors = {
        0: (255, 255, 255),
        1: (180, 220, 255),
        2: (255, 180, 180),
        3: (220, 180, 255),
        4: (255, 240, 180),
        5: (180, 255, 240),
    }
    c = base_colors.get(color_type, (255, 255, 255))
    return (int(c[0] * brightness), int(c[1] * brightness), int(c[2] * brightness))


def render_sky_star(star: SkyStar, camera: Camera, focal_length: float, 
                    time: float, screen: pygame.Surface):
    r = 1000.0
    x = r * math.cos(star.phi) * math.sin(star.theta)
    y = r * math.sin(star.phi)
    z = r * math.cos(star.phi) * math.cos(star.theta)
    
    star_pos = Point3D(x, y, z)
    pp = project_point(star_pos, camera, focal_length)
    
    margin = 10
    if pp.x < -margin or pp.x > SCREEN_SIZE + margin or pp.y < -margin or pp.y > SCREEN_SIZE + margin:
        return
    
    brightness = star.base_brightness * (0.3 + 0.7 * abs(math.sin(time * star.speed + star.phase)))
    size = int(star.size * (0.5 + 0.5 * brightness))
    if size <= 0:
        return
    
    color = get_star_color(star.color_type, brightness)
    pygame.draw.circle(screen, color, (int(pp.x), int(pp.y)), size)


# ============================================================
# 星空风格轨迹颜色
# ============================================================
def get_galaxy_color(t: float) -> Tuple[int, int, int]:
    colors = [
        (60, 100, 255),
        (100, 80, 255),
        (200, 80, 255),
        (255, 80, 200),
        (255, 120, 100),
        (100, 200, 255),
    ]
    idx = t * (len(colors) - 1)
    i = int(idx)
    if i >= len(colors) - 1:
        return colors[-1]
    frac = idx - i
    return (
        int(colors[i][0] + (colors[i+1][0] - colors[i][0]) * frac),
        int(colors[i][1] + (colors[i+1][1] - colors[i][1]) * frac),
        int(colors[i][2] + (colors[i+1][2] - colors[i][2]) * frac),
    )


def draw_glow_line(screen: pygame.Surface, p1: Point2D, p2: Point2D, color: Tuple[int, int, int], width: int):
    pygame.draw.line(screen, color, (int(p1.x), int(p1.y)), (int(p2.x), int(p2.y)), width)
    for w in range(width + 2, width + 6, 2):
        alpha = max(0, 20 - (w - width) * 4)
        if alpha > 0:
            glow_color = (color[0], color[1], color[2], alpha)
            pygame.draw.line(screen, glow_color, 
                             (int(p1.x), int(p1.y)), 
                             (int(p2.x), int(p2.y)), w)


# ============================================================
# 打印使用说明
# ============================================================
def print_usage():
    print("""
================================================================================
                手表端 3D轨迹模拟器 - 使用说明
================================================================================

用法:
    python track_visualizer.py [KML文件路径]

导出模式:
    python track_visualizer.py --export <KML文件路径> [输出文件路径]
    将KML轨迹转换为二进制文件，包含元数据（名称/距离/耗时/海拔）

示例:
    # 正常启动（自动导出二进制到同目录）
    python track_visualizer.py demo_track.kml
    
    # 只导出，不预览
    python track_visualizer.py --export demo_track.kml
    python track_visualizer.py --export demo_track.kml output.bin

交互操作:
    鼠标拖拽         摄像机在球面上移动
    滚轮             缩放视野
    R键             重置视角
    ESC键           退出程序

二进制文件格式 (v2.0):
    Magic: "VELX" (4 bytes)
    Version: 0x0200 (2 bytes)
    Flags: 预留 (2 bytes)
    Point Count: 点数 (2 bytes)
    Name Length: 名称长度 (2 bytes)
    Name: UTF-8编码名称 (N bytes)
    Total Distance: 总里程(km) (4 bytes float)
    Total Time: 总耗时(min) (4 bytes float)
    Min Altitude: 最低海拔(m) (4 bytes float)
    Max Altitude: 最高海拔(m) (4 bytes float)
    Points: float x, y, z 连续存储 (12 bytes per point)
================================================================================
""")


# ============================================================
# 主程序
# ============================================================
def batch_convert_kmls(scan_dir: str) -> int:
    """扫描目录下的 .kml 文件并导出同名 .bin。返回导出成功的文件数。"""
    if not os.path.isdir(scan_dir):
        print(f"错误: 目录不存在或不可读: {scan_dir}")
        return 0
    files = sorted([f for f in os.listdir(scan_dir) if f.lower().endswith('.kml')])
    if not files:
        print(f"目录中未发现 .kml 文件: {scan_dir}")
        return 0
    success = 0
    for fn in files:
        kml_path = os.path.join(scan_dir, fn)
        out_bin = os.path.splitext(kml_path)[0] + '.bin'
        print(f"处理: {kml_path} -> {out_bin}")
        try:
            ok = export_binary_from_file(kml_path, out_bin)
            if ok:
                success += 1
        except Exception as e:
            print(f"处理失败: {kml_path} - {e}")
    print(f"已处理 {len(files)} 个KML，成功导出 {success} 个二进制文件")
    return success


def find_track_file_dir() -> str:
    """尝试定位 track_file 目录的常见位置，返回第一个存在的路径或 './track_file'（不一定存在）。"""
    here = os.path.dirname(os.path.abspath(__file__))
    candidates = [
        os.path.join(here, 'track_file'),
        os.path.join(here, '..', 'hello_app', 'track_file'),
        os.path.join(here, '..', 'hello_app', 'track_file'),
        os.path.join(here, '..', 'track_file'),
        os.path.join(here, '..', 'data'),
        os.path.join(here, 'data'),
    ]
    for c in candidates:
        if os.path.isdir(c):
            return os.path.normpath(c)
    # fallback to first candidate
    return os.path.normpath(candidates[0])


def main():
    # ===== 检查导出模式 =====
    if len(sys.argv) >= 2 and sys.argv[1] == "--export":
        if len(sys.argv) < 3:
            print("错误: 请指定KML文件路径")
            print("用法: python track_visualizer.py --export <KML文件> [输出文件]")
            return
        
        kml_file = sys.argv[2]
        output = sys.argv[3] if len(sys.argv) >= 4 else None
        
        if not os.path.exists(kml_file):
            print(f"错误: 文件不存在 - {kml_file}")
            return
        
        print(f"导出模式: {kml_file}")
        export_binary_from_file(kml_file, output)
        return
    
    # ===== 帮助 =====
    if len(sys.argv) > 1 and sys.argv[1] in ["-h", "--help", "/?", "-?"]:
        print_usage()
        return

    # ===== 如果给出目录参数，按目录扫描 =====
    if len(sys.argv) > 1 and os.path.isdir(sys.argv[1]):
        scan_dir = sys.argv[1]
        batch_convert_kmls(scan_dir)
        return

    # ===== 默认行为：扫描 track_file/ 并批量转换 =====
    scan_dir = find_track_file_dir()
    print(f"扫描轨迹目录: {scan_dir}")
    converted = batch_convert_kmls(scan_dir)
    if converted > 0:
        return

    # ===== 若未发现KML则退回到交互预览模式（兼容旧行为） =====
    kml_file = "demo_track.kml"
    if len(sys.argv) > 1:
        kml_file = sys.argv[1]
    
    pygame.init()
    screen = pygame.display.set_mode((SCREEN_SIZE, SCREEN_SIZE))
    pygame.display.set_caption(f"手表端3D轨迹 - {os.path.basename(kml_file)}")
    clock = pygame.time.Clock()
    font = pygame.font.Font(None, 16)
    
    try:
        print(f"\n加载: {kml_file}")
        raw, metadata = parse_kml(kml_file)
        print(f"原始点数: {len(raw)}")
        print(f"轨迹名称: {metadata.name}")
        print(f"总里程: {metadata.total_distance_km:.2f} km")
        print(f"总耗时: {metadata.total_time_min:.1f} min")
    except FileNotFoundError as e:
        print(f"\n错误: {e}")
        pygame.quit()
        return
    except ET.ParseError as e:
        print(f"\n错误: KML解析失败 - {e}")
        pygame.quit()
        return
    
    if len(raw) < 2:
        print("错误: 轨迹点数不足")
        pygame.quit()
        return
    
    normalized, alt_min, alt_max = normalize_coordinates(raw)
    print(f"归一化后点数: {len(normalized)}")
    print(f"海拔范围: {alt_min:.1f}m ~ {alt_max:.1f}m")
    
    # 更新元数据海拔
    if metadata.min_altitude == 0 and metadata.max_altitude == 0:
        metadata.min_altitude = alt_min
        metadata.max_altitude = alt_max
    
    points = douglas_peucker(normalized, EPSILON)
    print(f"抽稀后点数: {len(points)} (阈值: {EPSILON})")
    print(f"压缩率: {(1 - len(points)/len(normalized)) * 100:.1f}%\n")
    
    # ===== 自动导出二进制文件 =====
    binary_path = os.path.splitext(kml_file)[0] + ".bin"
    export_binary(points, metadata, binary_path)
    
    if len(points) < 2:
        print("错误: 抽稀后轨迹点数不足")
        pygame.quit()
        return
    
    # ===== 初始化渲染 =====
    sky_stars = generate_sky_stars(STAR_COUNT)
    
    camera = Camera(theta=0.5, phi=0.3, radius=CAMERA_RADIUS)
    
    dragging = False
    last_mouse_x = 0
    last_mouse_y = 0
    running = True
    focal_length = 180
    max_phi = math.radians(MAX_PHI_DEG)
    
    # 参考网格
    grid_size = 8
    grid_step = 0.3
    y_plane = -0.2
    grid_lines = []
    
    for i in range(-grid_size, grid_size + 1):
        t = i * grid_step
        if -1.2 <= t <= 1.2:
            grid_lines.append((Point3D(-1.2, y_plane, t), Point3D(1.2, y_plane, t)))
            grid_lines.append((Point3D(t, y_plane, -1.2), Point3D(t, y_plane, 1.2)))
    
    print("=== 交互控制 ===")
    print("鼠标拖拽: 摄像机在球面上移动")
    print("滚轮: 缩放视野")
    print("R键: 重置视角")
    print("ESC键: 退出程序")
    print("=" * 40 + "\n")
    
    while running:
        dt = clock.tick(TARGET_FPS) / 1000.0
        
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            elif event.type == pygame.MOUSEBUTTONDOWN:
                if event.button == 1:
                    dragging = True
                    last_mouse_x = event.pos[0]
                    last_mouse_y = event.pos[1]
                elif event.button == 4:
                    focal_length = max(80, min(400, focal_length * 1.05))
                elif event.button == 5:
                    focal_length = max(80, min(400, focal_length * 0.95))
            elif event.type == pygame.MOUSEBUTTONUP:
                if event.button == 1:
                    dragging = False
            elif event.type == pygame.MOUSEMOTION:
                if dragging:
                    dx = event.pos[0] - last_mouse_x
                    dy = event.pos[1] - last_mouse_y
                    camera.theta += dx * 0.008
                    camera.phi = max(-max_phi, min(max_phi, camera.phi + dy * 0.008))
                    last_mouse_x = event.pos[0]
                    last_mouse_y = event.pos[1]
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_ESCAPE:
                    running = False
                elif event.key == pygame.K_r:
                    camera.theta = 0.5
                    camera.phi = 0.3
                    focal_length = 180
        
        screen.fill((10, 10, 26))
        
        # 1. 渲染彩色星星
        time = pygame.time.get_ticks() / 1000.0
        for star in sky_stars:
            render_sky_star(star, camera, focal_length, time, screen)
        
        # 2. 绘制参考网格
        for p1, p2 in grid_lines:
            pp1 = project_point(p1, camera, focal_length)
            pp2 = project_point(p2, camera, focal_length)
            if -50 < pp1.x < SCREEN_SIZE + 50 and -50 < pp1.y < SCREEN_SIZE + 50:
                pygame.draw.line(screen, (30, 40, 70), 
                                 (int(pp1.x), int(pp1.y)), 
                                 (int(pp2.x), int(pp2.y)), 1)
        
        # 3. 绘制星空风格轨迹
        if len(points) >= 2:
            projected = []
            for p in points:
                pp = project_point(p, camera, focal_length)
                projected.append(pp)
            
            for i in range(len(points) - 1):
                p1 = projected[i]
                p2 = projected[i+1]
                
                if (p1.x > -50 and p1.x < SCREEN_SIZE + 50 and p1.y > -50 and p1.y < SCREEN_SIZE + 50) or \
                   (p2.x > -50 and p2.x < SCREEN_SIZE + 50 and p2.y > -50 and p2.y < SCREEN_SIZE + 50):
                    t = i / (len(points) - 1)
                    color = get_galaxy_color(t)
                    draw_glow_line(screen, p1, p2, color, 1)
        
        # 4. UI信息
        info_texts = [
            f"📁 {metadata.name}",
            f"📏 {metadata.total_distance_km:.2f} km  ⏱ {metadata.total_time_min:.1f} min",
            f"⛰ {metadata.min_altitude:.0f}m ~ {metadata.max_altitude:.0f}m",
            f"📍 {len(points)} 点 | {int(math.degrees(camera.theta) % 360)}° {int(math.degrees(camera.phi))}°",
            f"💾 {os.path.basename(binary_path)}",
        ]
        for i, text in enumerate(info_texts):
            surf = font.render(text, True, (150, 160, 200))
            screen.blit(surf, (8, 8 + i * 16))
        
        fps = int(clock.get_fps())
        fps_surf = font.render(f"{fps}FPS", True, (80, 160, 80))
        screen.blit(fps_surf, (SCREEN_SIZE - 60, 8))
        
        if fps < 25:
            warn = font.render("⚠ 性能不足", True, (200, 100, 100))
            screen.blit(warn, (SCREEN_SIZE - 110, 28))
        
        pygame.display.flip()
    
    pygame.quit()
    print("\n程序已退出")


if __name__ == "__main__":
    main()