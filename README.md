# 手表应用-3d运动轨迹展示

## 一、作品简介

“手表应用-3d运动轨迹展示”面向运动场景，计划利用手表采集的运动数据，将用户的运动路线以三维轨迹的方式呈现出来，帮助用户直观回顾运动过程和空间变化。

作品重点关注手表端的轻量化交互：在有限的屏幕尺寸和设备资源下，以清晰的轨迹展示、简洁的操作流程和适合运动场景的视觉反馈为核心。后续可结合加速度计、陀螺仪、定位等数据，完善轨迹采集、回放和视角切换能力。

当前仓库提供 openvela 手表快应用的工程入口和基础页面骨架，具体的 3D 轨迹渲染与传感器接入代码应在此基础上继续完善。

## 二、选题方向

**快应用 / 手表应用创新。**

本作品针对智能手表这一小屏、低功耗、便携式设备，探索运动数据的三维可视化方式。相比只展示距离、时长等二维统计数据，三维运动轨迹能够提供更直观的运动路径回顾，也更适合体现手表应用在传感器数据展示和轻量交互方面的特点。

## 三、目录结构

```text
.
├── app/hello_app/                  # openvela 原生应用示例入口
├── app/py_scripts/                 # Pygame 3D轨迹模拟器 - 手表端模拟
├── app/track_file/        	    # 默认内置的3个轨迹文件，kml文件来源：两步路app,.bin文件：kml转bin,手表端使用的轨迹文件
├── logs/                           # AI Coding 对话日志
├── contest2026_357_Velatrix.xml    # repo 工程清单和目录映射
└── README.md                       # 作品说明
```


## 四、运行方式

### 1. 拉取完整工程

在 openvela 工作区中执行：

```bash
repo init -u https://github.com/open-vela/contest2026_357_Velatrix \
  -b dev-ai-contest-2026 -m contest2026_357_Velatrix.xml
repo sync -c -j8
```

同步完成后，本仓库位于工作区中的 `contest2026_357_Velatrix/` 目录，openvela 的完整源码位于其外层目录。

### 2. 编译

进入 openvela 工作区根目录，使用 `build.sh` 编译：

```bash
cd ..
./build.sh vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap -j8
```

如需重新配置或清理构建目录，可使用：

```bash
 ./build.sh vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap menuconfig
./build.sh vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap distclean -j8
```


### 3. 运行模拟器
```bash
./emulator.sh vela
```
### 4. 推送轨迹资源
在contest2026_357_Velatrix/app/track_file/ 路径下打开终端

adb push ./*.bin /data/

在模拟器的终端环境 openvela-ap> 中输入如下命令：hello_app


## 五、AI Coding 使用说明

本作品使用 AI 辅助完成以下开发工作：

- **需求拆解**：将“手表端展示 3D 运动轨迹”的目标拆分为页面交互、数据采集、轨迹数据处理和三维渲染等子任务。
- **方案设计**：结合 openvela 快应用目录结构和手表设备的屏幕、功耗限制，讨论适合小屏设备的展示方式和交互流程。
- **编码与调试**：辅助生成和检查快应用页面、manifest 配置及 openvela 工程映射，定位构建和运行过程中的配置问题。
- **文档整理**：辅助补充目录说明、构建运行步骤和作品介绍，确保评委能够按照 README 复现工程。 
