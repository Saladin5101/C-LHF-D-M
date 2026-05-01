#!/bin/bash

# C-LHF&D-M Configuration Management Examples
# 配置文件处理示例

echo "=== C-LHF&D-M 配置文件处理示例 ==="
echo ""

# 设置项目路径
CLHFM="${1:-.}/bin/clhfm"

# 检查二进制是否存在
if [ ! -f "$CLHFM" ]; then
    echo "Error: clhfm binary not found at $CLHFM"
    echo "Run: make all"
    exit 1
fi

echo "二进制路径: $CLHFM"
echo ""

# 示例 1: 初始化配置
echo "示例 1: 初始化默认配置"
echo "$ clhfm config init"
$CLHFM config init
echo ""

# 示例 2: 查看所有配置
echo "示例 2: 显示当前配置"
echo "$ clhfm config show"
$CLHFM config show
echo ""

# 示例 3: 获取单个配置
echo "示例 3: 获取特定配置参数"
echo "$ clhfm config get registry_url"
$CLHFM config get registry_url
echo ""

echo "$ clhfm config get max_concurrent_ops"
$CLHFM config get max_concurrent_ops
echo ""

echo "$ clhfm config get connection_timeout"
$CLHFM config get connection_timeout
echo ""

# 示例 4: 修改配置
echo "示例 4: 修改配置参数"
echo "$ clhfm config set max_concurrent_ops 64"
$CLHFM config set max_concurrent_ops 64
echo ""

# 示例 5: 验证修改
echo "示例 5: 验证修改已保存"
echo "$ clhfm config get max_concurrent_ops"
$CLHFM config get max_concurrent_ops
echo ""

# 示例 6: 修改 Registry URL
echo "示例 6: 修改 Registry URL（演示）"
echo "$ clhfm config set registry_url https://custom-registry.example.com"
$CLHFM config set registry_url https://custom-registry.example.com
echo ""

# 示例 7: 修改连接超时
echo "示例 7: 修改连接超时"
echo "$ clhfm config set connection_timeout 60"
$CLHFM config set connection_timeout 60
echo ""

# 示例 8: 最终配置状态
echo "示例 8: 最终配置状态"
echo "$ clhfm config show"
$CLHFM config show
echo ""

# 示例 9: 查看配置文件位置
echo "示例 9: 配置文件位置"
CONFIG_FILE=~/.clhfm/config.ini
echo "配置文件: $CONFIG_FILE"
if [ -f "$CONFIG_FILE" ]; then
    echo "配置文件内容:"
    cat "$CONFIG_FILE"
else
    echo "配置文件不存在"
fi
echo ""

echo "=== 配置示例演示完成 ==="
echo ""
echo "常用命令快速参考:"
echo "  clhfm config show                      # 显示所有配置"
echo "  clhfm config get <key>                 # 获取特定值"
echo "  clhfm config set <key> <value>         # 设置值"
echo "  clhfm config init                      # 重置为默认值"
