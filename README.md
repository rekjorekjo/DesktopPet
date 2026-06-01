# DesktopPet

![C++](https://img.shields.io/badge/C++17-00599C?logo=cplusplus&logoColor=white)
![Qt](https://img.shields.io/badge/Qt_6.x-41CD52?logo=qt&logoColor=white)
![CMake](https://img.shields.io/badge/CMake_3.19+-064F8C?logo=cmake&logoColor=white)
![Windows](https://img.shields.io/badge/Windows-supported-0078D6?logo=windows&logoColor=white)

DesktopPet 是一个基于 Qt Widgets 开发的桌面宠物应用，支持自定义宠物资源、动作播放、AI 对话、联网搜索、情绪动作联动、对话日志和自动更新。

DesktopPet 旨在提供一个轻量、可扩展、可个性化的桌面陪伴工具，同时保留清晰的本地资源管理、配置管理和发布更新流程。

项目状态（v1.1.1，统计数据仅供参考）

---

## 功能特性

- 桌面悬浮宠物显示，支持拖拽、右键菜单、托盘菜单
- 宠物管理：新建、导入、启用/禁用、删除、重新加载
- 动作管理：新建动作、导入动作、动作分类配置
- 日常 / 随机 / 定时 / 情绪动作分类播放
- 模型结构化回复（reply + emotion），情绪动作自动触发
- 桌宠移动（水平、垂直、随机方向）
- 主题系统（浅色 / 深色 / Bloom / 卡片渐变 / 随机渐变）
- 基于 OpenAI 兼容 API 的非流式 AI 对话
- 联网搜索增强（支持 Tavily / Brave Search / Exa，关键词自动触发与手动触发）
- 对话日志 JSONL 写入与查看
- 聊天设置与系统提示词自定义（角色设定）
- API Profile 配置管理与 JSON 持久化
- 个性化主题、不透明度、开机自启动、注册表清理
- 关于页检查 GitHub Releases 更新
- Inno Setup 安装包发布
- 配套工具 DesktopPet-resize，用于批量处理动作图片资源

---

## 界面预览

> 截图与演示动图待补充。

---

## 技术栈

| 技术 | 说明 |
|------|------|
| C++17 | 项目语言标准 |
| Qt 6.5+ | 应用框架 |
| Qt Widgets | UI 组件 |
| Qt Network | 网络请求（AI 对话） |
| Qt Svg | SVG 图标支持 |
| Qt Test | 单元测试框架 |
| CMake 3.19+ | 构建系统 |
| JSON | 配置与数据持久化 |

当前主要支持 Windows 平台，未来可扩展跨平台适配。

---

## 项目结构

```text
DesktopPet/
├── core/          # 全局设置、版本、路径管理
├── models/        # 宠物、动作、API 配置等数据模型
├── services/      # 宠物库、动作库、API Profile、聊天设置、聊天请求等服务
├── runtime/       # 桌面宠物运行时窗口与动画播放器
├── pages/         # 设置窗口页面
├── dialogs/       # 配置与创建类对话框
├── widgets/       # 通用 UI 组件
├── theme/         # 主题管理
├── ui/            # 设置主窗口
├── utils/         # GIF 抽帧、图片处理等工具函数
├── platform/      # 平台相关代码
├── tools/         # 配套工具与打包脚本
├── tests/         # 单元测试
└── resources/     # 图标、主题等资源
```

---

## 配置与数据文件

程序运行后会在应用程序目录（`<appDir>`）下的 `pets` 目录作为数据根目录生成配置与数据，即 `<appDir>/pets/`。`<appDir>` 指 DesktopPet 可执行文件所在目录，通过 `QCoreApplication::applicationDirPath()` 获取，与启动时的工作目录无关。

### API 与聊天配置

```
<appDir>/pets/config/
├── api_profiles.json         # API 配置档案（支持多配置切换，当前配置也保存在此文件中）
├── chat_settings.json        # 系统提示词与聊天设置
└── websearch_settings.json   # 联网搜索配置（搜索服务、API Key 等）
```

### 宠物与动作数据

```
<appDir>/pets/petlibrary.json              # 宠物库索引（宠物 ID 唯一依据）
<appDir>/pets/pets/<petId>/                # 宠物配置（pet.json、playlist.json）
<appDir>/pets/actions/<actionId>/          # 动作资源（action.json、图片帧）
<appDir>/pets/logs/chat/<petId>/<yyyyMMdd>.jsonl  # 对话日志（JSONL 格式）
```

宠物目录规则：`pets/<petId>/` 是唯一合法的宠物目录格式，`petId` 必须与 `pet.json` 中的 `id` 字段一致。

**DesktopPet 不内置默认宠物资源。** 安装包和 release 包不会打包默认 pets 资源。用户需要自行创建、导入或恢复宠物资源。

### API Key 存储说明

当前版本的 API Profile 会保存在本地 `<appDir>/pets/config/api_profiles.json` 中，API Key 仍以明文形式存储。DesktopPet 不会主动上传 API Key，但请不要在公共电脑或不可信设备上保存密钥。后续版本计划接入系统级安全存储。

---

## AI 对话能力

DesktopPet 内置聊天面板，支持接入 OpenAI 兼容格式的大模型 API。

### 当前支持

- 仅支持 OpenAI 兼容格式（OpenAI-compatible）的非流式聊天请求
- 联网搜索增强：支持 Tavily、Brave Search、Exa 三种搜索服务
- 从聊天设置页选择当前 API Profile
- 自定义系统提示词（桌宠人格与对话风格设定）
- 聊天窗口中用户消息显示为"我"，宠物回复显示为当前宠物名称
- 对话上下文自动裁剪
- 每次请求自动注入当前本地日期、时间与星期（不写入上下文，不显示）
- 首次对话可根据角色设定自动搜索实时信息（需在搜索设置中启用）
- 助手回复使用结构化 JSON 协议（reply + emotion），聊天窗口只显示 reply 文本
- emotion 支持 neutral、happy、sad、angry、surprised、fear、confused，非 neutral 时自动触发宠物情绪动作（需 playlist.json 配置对应 emotion 动作）

### 当前暂不支持

- Tool Calling / Function Calling
- 流式输出

### 支持的 API 格式与供应商

当前版本仅支持 OpenAI-compatible API 格式。以下供应商预设均按 OpenAI 兼容接口调用：

DeepSeek、Kimi、Qwen、GLM、Doubao、MiniMax、XiaoMiMiMo、OpenAI、Gemini（OpenAI endpoint）、Grok，以及"自定义"配置。

Claude / Anthropic 原生接口（Anthropic-compatible）暂未开放。

### 对话日志

v0.32.1 起支持对话日志自动写入。聊天窗口中可见的用户消息、宠物回复和系统提示会写入本地 JSONL 日志文件。

日志路径：`logs/chat/<petId>/<yyyyMMdd>.jsonl`。没有当前宠物时使用 `logs/chat/_no_pet/<yyyyMMdd>.jsonl`。

日志仅用于查看和检索，不会自动恢复聊天窗口，也不会作为长期记忆重新注入对话。不记录 API Key、临时 system prompt、当前时间 system message 或联网搜索上下文原文。

### 联网搜索说明

v0.31.2 起支持联网搜索增强。用户可在"搜索设置"页面配置搜索服务（Tavily / Brave Search / Exa）及其 API Key。

联网搜索有两种触发方式：
- **关键词自动触发**：消息中包含"新闻"、"天气"、"股价"等复合关键词时自动搜索（"最近怎么样"等闲聊不会触发）
- **手动触发**：消息中包含 `/search`、`#search`、"搜一下"、"查一下"等指令（支持放在消息开头或消息中间），指令后的文本会作为搜索 query

本地时间相关问题（"现在几点"、"今天星期几"等）不会触发联网搜索，直接由 LLM 基于注入的时间上下文回答。

搜索结果会作为临时上下文注入对话，不会写入聊天历史。用户手动触发或关键词触发的搜索失败时，会提示错误信息，不会自动回退为普通聊天回答。

v0.31.6 起支持"首次对话自动搜索"选项：启用后，应用启动后首次对话时会根据角色设定自动搜索实时信息，搜索结果注入对话上下文。

---

## 构建

### 环境要求

- Qt 6.5 或更高版本
- CMake 3.19 或更高版本
- C++17 编译器

Windows 推荐使用 Qt 6 MSVC 64-bit Kit。

### 使用 Qt Creator

1. 打开 Qt Creator，选择 `CMakeLists.txt`
2. 选择 Qt 6 MSVC Kit
3. Configure Project，选择 Debug 或 Release
4. Build & Run

### 使用命令行

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

构建测试版本：

```bash
cmake -S . -B build -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

如果 Qt 未加入默认搜索路径，需设置 `CMAKE_PREFIX_PATH` 或 `Qt6_DIR`。

---

## 运行测试

项目使用 Qt Test 作为测试框架，通过 CTest 运行。

```bash
ctest --test-dir build -C Release --output-on-failure
```

### 测试范围

当前测试主要覆盖：

- API Key 脱敏（SecretStorageService）
- WebSearch 配置 JSON 序列化与边界修正（WebSearchConfig）
- 宠物库磁盘恢复逻辑（PetLibraryIndexService）
- 版本比较逻辑（VersionUtils）
- GitHub Release JSON 解析（UpdateService）
- Manifest JSON 解析与安装包文件名校验（UpdateService）

### 暂不覆盖

- Qt Widgets 图形界面自动化测试
- 真实联网搜索请求
- Inno Setup 安装流程

---

## 配套工具：DesktopPet-resize

`DesktopPet-resize` 是独立的配套图片处理工具，用于批量裁剪和压缩 `pets/actions/<actionId>` 下的动作图片资源。

该工具独立编译，不随主程序自动运行。打包发布时如已构建，会一并包含在发布目录中。

---

## 路线图

- 流式输出
- Tool Calling / Function Calling
- 长期记忆 / 摘要记忆
- 更完善的宠物资源编辑器
- 更多跨平台适配

---

## 打包发布

项目提供 Windows 打包脚本：

```bash
tools/packaging/package_release.bat
tools/packaging/build_installer.bat
```

`package_release.bat` 调用 `windeployqt` 收集 Qt 依赖，并生成可发布目录与压缩包。`build_installer.bat` 调用 Inno Setup 生成安装程序。打包前需先在 Qt Creator 中构建 Release 版本。

---

## 更新机制

DesktopPet 支持在"关于"页检查 GitHub Releases 上的新版本。

- 检查更新时优先读取 GitHub Release asset 中的 `latest.json` manifest 文件。
- 如果 manifest 获取或解析失败，会 fallback 到 GitHub Releases API。
- 关于页只显示版本号信息，不显示 release notes / release body / Markdown 内容。
- 如果有新版本，会提供"下载并安装"和"打开发布页"按钮。
- 下载的是 GitHub release 中的 Inno Setup 安装包（`DesktopPet_Setup*.exe`），保存到系统临时目录。
- 下载完成后自动启动安装向导，然后退出 DesktopPet。
- 安装程序不会直接删除或覆盖 DesktopPet.exe，由 Inno Setup 处理文件替换。
- 更新不会主动删除 `config`、`pets`、`logs` 目录。
- 当前仅支持公开仓库更新，不内置 GitHub token，不支持私有仓库。
- 发布 GitHub Release 时需要上传 `latest.json` 文件（由 `build_installer.bat` 自动生成）。
- 自动后台更新可作为后续计划。

---

## 许可

详见 [LICENSE](LICENSE)。
