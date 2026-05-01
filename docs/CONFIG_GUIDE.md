# C-LHF&D-M 配置文件处理

## 概述

C-LHF&D-M 使用 INI 格式的配置文件来管理各种设置。配置文件自动在首次运行时创建，存储在 `~/.clhfm/config.ini`。

## 配置文件位置

```bash
~/.clhfm/config.ini       # 主配置文件（自动创建）
~/.clhfm/                 # 配置目录
├── packages/             # 安装的包
├── cache/                # 包缓存
├── registry/             # 包注册表
└── config.ini            # 配置文件
```

## 配置管理命令

### 显示当前配置

```bash
clhfm config show
```

输出示例：
```
=== Current Configuration ===
Registry URL:        https://saladin510-orangeyouhome-erath.online
Timeout:             30 seconds
Max Concurrent Ops:  32
Config Directory:    /home/user/.clhfm
Packages Directory:  /home/user/.clhfm/packages
Cache Directory:     /home/user/.clhfm/cache
Registry Directory:  /home/user/.clhfm/registry
```

### 获取特定配置参数

```bash
clhfm config get registry_url
clhfm config get connection_timeout
clhfm config get max_concurrent_ops
```

### 设置配置参数

```bash
# 修改 Registry URL
clhfm config set registry_url https://new-registry.example.com

# 修改连接超时（秒）
clhfm config set connection_timeout 60

# 修改最大并发操作数
clhfm config set max_concurrent_ops 64
```

### 初始化默认配置

```bash
clhfm config init
```

这个命令创建或重置配置文件到默认值。

## 配置文件格式

配置文件采用 INI 格式，包含以下部分：

```ini
# Registry 配置
[registry]
registry_url=https://saladin510-orangeyouhome-erath.online
connection_timeout=30

# 性能设置
[performance]
max_concurrent_ops=32

# 目录配置（通常自动设置）
[directories]
packages_dir=/home/user/.clhfm/packages
cache_dir=/home/user/.clhfm/cache
registry_dir=/home/user/.clhfm/registry
```

## 配置参数详解

### Registry 配置

| 参数 | 类型 | 默认值 | 说明 |
|-----|-----|--------|------|
| registry_url | 字符串 | https://saladin510-orangeyouhome-erath.online | 包仓库 URL |
| connection_timeout | 整数 | 30 | HTTP 连接超时（秒） |

### 性能配置

| 参数 | 类型 | 默认值 | 说明 |
|-----|-----|--------|------|
| max_concurrent_ops | 整数 | 32 | 最大并发操作数（利用无锁架构） |

### 目录配置

| 参数 | 类型 | 默认值 | 说明 |
|-----|-----|--------|------|
| packages_dir | 字符串 | ~/.clhfm/packages | 已安装包存储目录 |
| cache_dir | 字符串 | ~/.clhfm/cache | 包缓存目录 |
| registry_dir | 字符串 | ~/.clhfm/registry | 包注册表目录 |

## 常见场景

### 场景 1：切换到自定义 Registry

```bash
# 查看当前 Registry
$ clhfm config get registry_url
https://saladin510-orangeyouhome-erath.online

# 切换到本地 Registry
$ clhfm config set registry_url http://localhost:8080/registry

# 验证改动
$ clhfm config get registry_url
http://localhost:8080/registry
```

### 场景 2：优化高并发性能

```bash
# 当进行大量并发安装时，增加并发限制
$ clhfm config set max_concurrent_ops 128

# 验证
$ clhfm config get max_concurrent_ops
128
```

### 场景 3：调整网络超时

如果网络慢，可以增加超时时间：

```bash
clhfm config set connection_timeout 60
```

### 场景 4：查看所有配置

```bash
$ clhfm config show

=== Current Configuration ===
Registry URL:        https://saladin510-orangeyouhome-erath.online
Timeout:             30 seconds
Max Concurrent Ops:  32
Config Directory:    /home/user/.clhfm
Packages Directory:  /home/user/.clhfm/packages
Cache Directory:     /home/user/.clhfm/cache
Registry Directory:  /home/user/.clhfm/registry
```

## 配置文件示例

查看 `config/config.ini.example` 获得完整示例。

## 配置文件的自动管理

- **首次运行**：自动创建 `~/.clhfm/config.ini`
- **不存在时**：使用内置默认值
- **手动修改**：直接编辑 `~/.clhfm/config.ini` 文件
- **命令行修改**：使用 `clhfm config set` 命令

## 配置文件路径的确定规则

系统按照以下优先级确定配置目录：

1. 环境变量 `$HOME` 下的 `.clhfm` 目录
2. 如果 `$HOME` 不存在，使用 `getpwuid()` 获得用户主目录
3. 最后降级到 `/tmp/.clhfm`

## 配置注意事项

- **Registry URL**：必须是有效的 URL，支持 HTTP 和 HTTPS
- **Timeout**：以秒为单位，建议范围 10-300
- **Max Concurrent Ops**：与系统文件描述符限制相关，建议不超过 1024
- **注释**：以 `#` 或 `;` 开头的行被视为注释

## 配置热加载

配置参数在以下时机加载：

1. **程序启动**：自动加载 `config.ini`
2. **命令行设置**：立即生效并保存到文件
3. **手动编辑**：下次运行时加载

## 调试配置问题

如果配置文件有问题，you can：

1. 删除配置文件：`rm ~/.clhfm/config.ini`
2. 重新初始化：`clhfm config init`
3. 查看当前配置：`clhfm config show`

## 配置优化建议

### 对于开发环境
```bash
clhfm config set max_concurrent_ops 16
clhfm config set connection_timeout 30
```

### 对于 CI/CD 环境
```bash
clhfm config set max_concurrent_ops 64
clhfm config set connection_timeout 10
```

### 对于生产环境
```bash
clhfm config set max_concurrent_ops 32
clhfm config set connection_timeout 60
```

## 未来扩展

计划中的配置功能：

- [ ] 多个 Registry 支持（fallback mechanism）
- [ ] 镜像配置
- [ ] 包签名验证开关
- [ ] 日志级别配置
- [ ] 缓存策略配置
- [ ] 代理设置

