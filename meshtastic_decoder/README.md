# Meshtastic MQTT解码器 - 开发版本

## 📍 重要提示

**本文件夹为历史开发版本，推荐使用最新的独立版本！**

### 🔥 最新版本位置
**完整功能版本**: `../decoder_portable/`  
**统一文档**: `../Meshtastic_MQTT解码器_完整指南.md`

### 🚀 推荐启动方式
```bash
# 方法1: 根目录一键启动 (推荐)
双击: ../start_mqtt_decoder.bat

# 方法2: 进入完整版本文件夹
cd ../decoder_portable
双击: mqtt_decoder.exe
```

---

## 📂 开发版本说明

本文件夹包含早期开发版本，主要用于：
- 开发历史记录
- 代码演进参考
- 功能实验测试

### 基本功能
- 解析ServiceEnvelope格式的protobuf消息
- 提取MeshPacket中的各个字段
- 分析加密数据结构
- 支持PSK #1和自定义PSK

### 编译使用
```bash
g++ -o mqtt_decoder.exe mqtt_decoder.cpp
./mqtt_decoder.exe
```

---

## ⚠️ 已知问题 (开发版本)

1. **界面乱码** - 中文显示可能有问题
2. **缺少启动脚本** - 需要手动编译和运行
3. **路径依赖** - 需要在特定目录运行
4. **源码分散** - 源码和可执行文件混合

## ✅ 解决方案

**所有问题已在最新版本中解决！**
请使用 `../decoder_portable/` 文件夹中的完整版本。

详细信息请参考: `../Meshtastic_MQTT解码器_完整指南.md`

---

**推荐使用最新版本以获得最佳体验！** 🎯 