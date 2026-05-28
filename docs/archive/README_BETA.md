# DesktopPet

DesktopPet 是一个基于 Qt/C++ 开发的 Windows 桌面宠物程序。  
项目目标是提供一个可配置、可扩展的桌面宠物运行环境，支持宠物管理、动作库管理、动作播放配置、主题个性化、AI 对话和本地日志等功能。

当前版本：`v0.30.5(beta)`

---

## 功能概览

### 已实现

- 桌面宠物透明窗口显示
- GIF / 图片帧动作播放
- 宠物启动、暂停、重新加载
- 托盘菜单
- 宠物管理
  - 新建宠物
  - 导入宠物
  - 编辑宠物
  - 移除宠物
  - 删除宠物
  - 配置异常恢复
- 动作管理
  - 新建动作
  - 导入单个动作
  - 导入动作库
  - 动作重命名
  - 移除动作
  - 删除动作
  - 动作资源缺失提示
- 动作分类配置
  - 日常动作
  - 随机动作
  - 定时动作
  - 情绪动作
- 播放项配置
  - 显示名称
  - 播放速度
  - 重复次数
  - 循环播放
  - 移动开关
  - 移动速度
  - 移动方向
- 桌宠移动
  - 水平移动
  - 垂直移动
  - 随机方向
- 主题系统
  - 浅色主题
  - 深色主题
  - Bloom 主题
  - 卡片渐变
  - 随机渐变
  - 右键菜单主题适配
- 个性化设置
  - 基础移动速度
  - 渐变强度
  - 随机渐变
- 宠物对话面板
  - 绑定在桌宠下方，自动适配屏幕位置
  - 跟随桌宠移动
  - Enter 发送，Shift + Enter 换行
  - 接入 OpenAI 兼容格式大模型对话 API
  - 自定义系统提示词
  - 聊天设置页面（API 配置 + 对话设定）
  - 对话上下文自动裁剪
- 日志页面
  - 按文件加载日志
  - 显示日志内容
- 本地库索引
  - `petlibrary.json`
  - `actionlibrary.json`
- 打包脚本
  - Windows Release 打包
  - `windeployqt` 依赖收集

### TODO

- 完善对话日志自动保存
- 增加更多动作触发方式
- 增加更细粒度的动作库批量导入预览
- 增加资源损坏检测与修复工具
- 优化内存占用
- 优化大量动作帧加载时的性能
- 增加自动化测试
- 增加跨平台适配
- 完善发布流程与版本更新机制

---

## 技术栈

- C++17
- Qt 6
  - Qt Core
  - Qt Widgets
  - Qt Svg
  - Qt Network
- CMake
- Windows 桌面环境

当前主要开发环境：

- Windows
- Qt 6.11.0 MSVC2022 64-bit
- Qt Creator

---

## 项目结构

```text
DesktopPet/
├── core/                 # 全局设置、路径管理、配置读写、版本信息
├── dialogs/              # 新建/导入宠物、动作等弹窗
├── models/               # 宠物、动作、播放列表等数据模型
├── pages/                # 设置窗口中的各个页面
├── resources/            # 图标、主题、资源文件
├── runtime/              # 桌宠运行窗口、动画播放器、托盘管理
├── tools/                # 工具（打包脚本、resize 工具等）
├── services/             # 宠物库、动作库、导入、日志、AI 对话等业务服务
├── theme/                # 主题管理
├── ui/                   # 设置主窗口
├── utils/                # GIF 抽帧、图片处理、更新工具
├── widgets/              # 自定义控件
├── CMakeLists.txt
└── README.md
```

------

## 核心数据目录

程序运行后会在运行目录下维护本地数据。

典型结构如下：

```
pets/
├── petlibrary.json
├── pets/
│   └── <petId>/
│       ├── pet.json
│       └── playlist.json
└── actions/
    ├── actionlibrary.json
    └── <actionId>/
        ├── action.json
        └── frames...

config/
├── api_profiles.json    # API 配置（多配置切换）
└── chat_settings.json   # 对话设定（系统提示词）
```

------

## 核心概念

### petlibrary.json

`petlibrary.json` 是宠物库索引。

从 `v0.28.8` 开始，程序不再用宠物目录是否存在来判断宠物 ID 是否已存在。
 宠物 ID 是否属于当前宠物库，以 `petlibrary.json` 为准。

语义：

- 在 `petlibrary.json` 中存在：宠物属于当前宠物库
- 不在 `petlibrary.json` 中：宠物 ID 可复用
- 移除宠物：只删除 `petlibrary.json` 中的记录，不删除宠物目录
- 删除宠物：删除 `petlibrary.json` 中的记录，并删除宠物目录

### actionlibrary.json

`actionlibrary.json` 是全局动作库索引。

从 `v0.28.9` 开始，程序不再用 `actions/<actionId>/` 目录是否存在来判断动作 ID 是否已存在。
 动作 ID 是否属于当前动作库，以 `actionlibrary.json` 为准。

语义：

- 在 `actionlibrary.json` 中存在：动作属于当前全局动作库
- 不在 `actionlibrary.json` 中：动作 ID 可复用
- 移除动作：只删除 `actionlibrary.json` 中的记录，不删除动作目录
- 删除动作：删除 `actionlibrary.json` 中的记录，并删除动作目录

### playlist.json

`playlist.json` 是某个宠物的播放项配置。

它不是 actionId 集合，而是播放序列。
 因此同一个 `actionId` 可以在同一分类中出现多次，每一条播放项可以拥有不同的显示名称、播放速度、循环次数、移动配置等参数。

------

## 移除与删除的区别

### 宠物

| 操作     | 行为                                          |
| -------- | --------------------------------------------- |
| 移除宠物 | 从 `petlibrary.json` 删除记录，不删除宠物目录 |
| 删除宠物 | 从 `petlibrary.json` 删除记录，并删除宠物目录 |

### 动作

| 操作             | 行为                                             |
| ---------------- | ------------------------------------------------ |
| 移除动作         | 从 `actionlibrary.json` 删除记录，不删除动作目录 |
| 删除动作         | 从 `actionlibrary.json` 删除记录，并删除动作目录 |
| 右侧分类中的移除 | 只从当前宠物的 `playlist.json` 删除对应播放项    |

------

## 构建方式

### 1. 安装依赖

需要安装：

- Qt 6.5 或更高版本
- CMake 3.19 或更高版本
- C++17 编译器

Windows 推荐使用：

- Qt 6.11.0 MSVC2022 64-bit
- Qt Creator

### 2. 使用 Qt Creator 构建

1. 打开 Qt Creator
2. 选择 `CMakeLists.txt`
3. 选择 Qt 6 MSVC Kit
4. Configure Project
5. 选择 Debug 或 Release
6. Build
7. Run

### 3. 使用命令行构建

示例：

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

如果 Qt 没有加入默认搜索路径，需要设置 `CMAKE_PREFIX_PATH` 或 `Qt6_DIR`。

------

## 打包发布

项目提供 Windows 打包脚本：

```
tools/packaging/package_release.bat
```

使用前请确认脚本中的 Qt 路径和 Release exe 路径正确。

默认假设：

```
F:\Qt\6.11.0\msvc2022_64
```

打包前需要先在 Qt Creator 中构建 Release 版本。

脚本会调用：

```
windeployqt
```

并生成可发布目录和压缩包。

------

## 使用说明

### 宠物管理

在设置窗口中进入“宠物管理”页面，可以：

- 新建宠物
- 导入宠物
- 编辑宠物
- 移除宠物
- 删除宠物
- 查看当前宠物动作数与可用动作数

如果宠物配置异常，程序会尽量保留恢复入口，允许修复、移除或删除。

### 动作设置

在“动作设置”页面，可以：

- 新建动作
- 导入动作
- 导入动作库
- 将动作添加到当前宠物分类
- 配置播放速度、重复次数、循环、移动等参数
- 保存配置
- 保存并应用

“保存并应用”会刷新当前宠物的动作库和播放列表，无需手动重新加载。

### 动作库导入

导入动作时，程序会自动识别：

- 单个动作目录
- 动作库目录

动作库目录支持两种形式：

1. 包含 `actionlibrary.json`
2. 没有 `actionlibrary.json`，但包含多个有效动作子目录

------

## 资源缺失处理

如果 `playlist.json` 中仍然引用某个动作，但该动作已经不在全局动作库中，或动作资源目录缺失，程序不会自动删除该播放项。

原因是：资源缺失可能只是用户误删或临时移动文件。
 自动删除播放项可能造成配置丢失。

此时界面会显示：

```
[资源缺失]
```

用户可以选择重新导入动作库，或手动移除对应播放项。

------

## 宠物对话

桌宠下方的对话面板，支持接入 OpenAI 兼容格式的大模型 API 进行真实对话。

功能：

- 支持输入与发送（Enter 发送，Shift + Enter 换行）
- 支持跟随桌宠移动，自动适配屏幕位置
- 支持打开时暂停桌宠移动
- 自定义系统提示词（对话设定）
- API 配置管理（支持多配置切换）
- 对话上下文自动裁剪（保留系统消息 + 最近 12 条）

配置文件：

- `config/api_profiles.json` — API 配置
- `config/chat_settings.json` — 对话设定（系统提示词）

------

## 日志

日志页面目前支持手动选择日志文件并显示内容。

TODO：

- 自动保存宠物对话日志
- 按日期检索日志
- 按宠物筛选日志

------

## 开发约定

### 1. 不要用目录是否存在判断 ID 是否存在

宠物 ID 判断：

```
petlibrary.json
```

动作 ID 判断：

```
actionlibrary.json
```

目录只是资源存储位置，不是 ID 注册表。

### 2. 不要把 playlist 当成 actionId 集合

`playlist.json` 是播放项列表。
 同一个 `actionId` 可以出现多次。

### 3. 弹窗不要直接 accept

新建、导入等弹窗采用 `submitRequested` 机制：

- 点击确认后发出 `submitRequested`
- 调用方执行业务逻辑
- 成功后调用 `accept`
- 失败时保留弹窗，用户可以继续修改

### 4. 保存并应用必须刷新动作库和播放列表

只刷新 `playlist.json` 不够。
 如果刚导入了新动作，player 还需要重新读取 `actionlibrary.json` 和动作资源。

------

## 当前状态

本项目仍处于 beta 阶段。
 当前重点是桌宠基础功能、动作系统、主题系统和本地配置稳定性。

后续会继续完善：

- 对话日志自动保存
- 日志系统
- 动作库管理体验
- 性能优化
- 发布流程

---

