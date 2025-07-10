# 📡 Meshtastic MQTT解码器 - 完整指南

## 🎯 项目概述

这是一个**完全独立**的Meshtastic MQTT消息解码工具，无需依赖任何Meshtastic库、protobuf库或PlatformIO环境。采用手动实现的protobuf解析器，支持ServiceEnvelope和MeshPacket格式解析。

### ✨ 核心特点
- ✅ **零依赖** - 无需安装任何库或环境
- ✅ **完全独立** - 脱离Meshtastic项目运行
- ✅ **单文件运行** - 2.7MB静态编译exe文件
- ✅ **跨平台移植** - 可复制到任何Windows系统
- ✅ **双语支持** - 中英文界面可选
- ✅ **源码开放** - 完整源码可修改扩展

---

## 📁 项目结构说明

本项目包含两个独立的解码器版本：

### 🔧 `decoder_portable/` - 独立版本 (推荐)
**完全脱离Meshtastic项目的独立解码器**
- ✅ 零外部依赖
- ✅ 可复制到任何电脑使用
- ✅ 无乱码版本可用

### 🏗️ `meshtastic_decoder/` - 库依赖版本
**基于Meshtastic项目库的解码器**
- ⚠️ 需要Meshtastic项目环境
- ⚠️ 可能有编码问题

---

## 🚀 推荐使用方式

### **最佳选择: 独立版本**
```
进入 decoder_portable/ 文件夹
双击: start_clean_decoder.bat
```

**或者：**
```
进入 decoder_portable/ 文件夹
双击: mqtt_decoder_clean.exe
```

### **重新编译 (如需修改)**
```
进入 decoder_portable/ 文件夹
运行: build.bat
选择: 1 (Clean English version)
```

---

## 📝 使用方法

### 基本使用流程
1. **进入独立版本文件夹** - `decoder_portable/`
2. **启动程序** - 双击 `start_clean_decoder.bat`
3. **输入MQTT数据** - 粘贴十六进制消息数据
4. **输入预期内容** - 帮助验证解析结果
5. **输入PSK信息** - 使用PSK #1或自定义PSK
6. **查看解析结果** - 程序显示详细解析信息

### 示例操作

#### 启动程序后的界面：
```
=================================
    Meshtastic MQTT Decoder      
=================================
Enter MQTT message hex data for decoding
Supports ServiceEnvelope format protobuf messages
=================================

Enter MQTT message hex data (type 'quit' to exit):
>
```

#### 输入数据：
```
MQTT数据: 0a25 0dc0 579c 8415 ffff ffff 2205 0801 1201 3135 4b9f de24 3d95 846f 6848 0358 6478 0398 01c0 0112 0953 686f 7274 536c 6f77 1a09 2138 3439 6335 3763 30

预期内容: 1
PSK: AQ==
```

#### 程序输出：
```
=== ServiceEnvelope Parsing ===
SUCCESS: Found MeshPacket (37 bytes)
SUCCESS: Channel ID: ShortSlow
SUCCESS: Gateway ID: !849c57c0

=== MeshPacket Parsing ===
SUCCESS: From: 0x849c57c0 (!849c57c0)
SUCCESS: To: 0xffffffff (broadcast)
SUCCESS: Encrypted data: 5 bytes

=== Parsing Results Summary ===
Source node: !849c57c0
Target: broadcast message
Channel: ShortSlow
Gateway: !849c57c0
```

### PSK输入说明
- **`AQ==`** - 使用PSK #1 (默认共享密钥)
- **`d4f1bb3a2029075...`** - 输入16字节自定义PSK的hex字符串

---

## 🔧 技术实现原理

### 独立实现 vs 传统方式

#### ❌ 传统方式（需要依赖）
```cpp
// 需要大量依赖库
#include "src/mesh/generated/meshtastic/mesh.pb.h"
#include "src/mesh/generated/meshtastic/mqtt.pb.h"  
#include <pb_decode.h>

// 复杂的构建系统
meshtastic_ServiceEnvelope envelope;
pb_decode(&stream, &meshtastic_ServiceEnvelope_msg, &envelope);
```

#### ✅ 我们的方式（完全独立）
```cpp
// 只使用标准C++库
#include <iostream>
#include <vector>
#include <string>

// 手动实现protobuf解析
uint64_t decodeVarint(const uint8_t*& data, size_t& remaining);
ServiceEnvelope parseServiceEnvelope(const uint8_t* data, size_t length);
```

### 核心技术组件

#### 1. 手动Protobuf解析器
```cpp
// varint解码实现
uint64_t decodeVarint(const uint8_t*& data, size_t& remaining) {
    uint64_t result = 0;
    int shift = 0;
    
    while (remaining > 0 && shift < 64) {
        uint8_t byte = *data++;
        remaining--;
        result |= ((uint64_t)(byte & 0x7F)) << shift;
        if ((byte & 0x80) == 0) break;
        shift += 7;
    }
    return result;
}
```

#### 2. 自定义结构体定义
```cpp
// 手动定义，无需.proto文件
struct ServiceEnvelope {
    vector<uint8_t> packetData;  // field 1
    string channelId;            // field 2  
    string gatewayId;            // field 3
    bool valid = false;
};

struct MeshPacket {
    uint32_t from = 0;           // field 1
    uint32_t to = 0;             // field 2
    vector<uint8_t> encryptedData; // field 4
    uint64_t id = 0;             // field 6
};
```

#### 3. 内置PSK处理
```cpp
// 内置默认PSK，无需外部配置
const uint8_t defaultpsk[] = {
    0xd4, 0xf1, 0xbb, 0x3a, 0x20, 0x29, 0x07, 0x59,
    0xf0, 0xbc, 0xff, 0xab, 0xcf, 0x4e, 0x69, 0x01
};
```

---

## 📁 详细项目结构

```
meshtastic_f/
├── Meshtastic_MQTT解码器_完整指南.md  # 本文档
├── decoder_portable/              # 🔥 独立版本 (推荐使用)
│   ├── start_clean_decoder.bat    # 无乱码启动脚本
│   ├── mqtt_decoder_clean.exe     # 无乱码版本 (推荐)
│   ├── mqtt_decoder_en.exe        # 英文版本
│   ├── mqtt_decoder_cn.exe        # 中文版本
│   ├── build.bat                  # 编译脚本
│   ├── README_Clean.md            # 独立版本说明
│   └── src/                       # 源码目录
│       ├── mqtt_decoder_clean_en.cpp  # 无乱码源码
│       ├── mqtt_decoder_en.cpp    # 英文版源码
│       └── mqtt_decoder_cn.cpp    # 中文版源码
└── meshtastic_decoder/            # 库依赖版本 (开发用)
    ├── mqtt_decoder.cpp           # 原始源码
    └── README.md                  # 库依赖版本说明
```

---

## 🛠 问题解决方案

### ✅ 已解决的问题

| 问题 | 解决方案 | 状态 |
|------|----------|------|
| **乱码显示** | 创建clean版本 | ✅ 完全解决 |
| **入口点错误** | 静态编译链接 | ✅ 完全解决 |
| **路径问题** | 独立文件夹结构 | ✅ 完全解决 |
| **源码保留** | 多版本源码保存 | ✅ 完全解决 |
| **依赖问题** | 完全独立实现 | ✅ 完全解决 |
| **项目污染** | 独立文件夹管理 | ✅ 完全解决 |

### 使用建议
1. **首选独立版本** - 使用 `decoder_portable/` 文件夹
2. **使用clean版本** - 完全无乱码问题
3. **保持文件夹结构** - 不要移动文件到根目录
4. **参考相应README** - 各文件夹有专门的说明文档

---

## 🔬 开发者指南

### 修改源码
1. 进入 `decoder_portable/` 文件夹
2. 编辑 `src/mqtt_decoder_clean_en.cpp` (推荐版本)
3. 运行 `build.bat` 重新编译
4. 选择要设为默认的版本

### 添加新功能
源码采用模块化设计，主要函数：
- `hexToBytes()` - 十六进制字符串转换
- `parseServiceEnvelope()` - ServiceEnvelope解析
- `parseMeshPacket()` - MeshPacket解析
- `analyzeEncryptedData()` - 加密数据分析
- `getPSKFromInput()` - PSK输入处理

### 扩展建议
可以继续添加的功能：
- 真正的AES-CTR解密实现
- JSON格式输出选项
- 批量处理MQTT日志文件
- 更多消息类型解析
- Web界面版本

---

## 📊 技术对比

| 特征 | Meshtastic官方 | 本解码器 |
|------|----------------|----------|
| **protobuf解析** | nanpb库 + .proto文件 | 手动实现 |
| **编译依赖** | PlatformIO + 多个库 | 仅g++ |
| **运行环境** | 完整开发环境 | 单exe文件 |
| **文件大小** | 需外部库 | 2.7MB自包含 |
| **移植性** | 需要环境配置 | 即复制即用 |
| **学习成本** | 高（需了解整个系统） | 低（单文件理解） |

---

## 🎁 独立性验证

### 验证步骤
1. 将 `decoder_portable/` 文件夹复制到任何Windows电脑
2. 双击 `start_clean_decoder.bat` 
3. 无需安装Meshtastic、PlatformIO或任何依赖库
4. 程序正常运行，完全脱离原项目环境

### 技术证明
- ✅ 零外部依赖
- ✅ 静态编译包含所有库
- ✅ 手动实现所有解析逻辑
- ✅ 内置所有必要的常量和数据结构

---

## 💡 使用提示

### 最佳实践
1. **使用独立版本** - 进入 `decoder_portable/` 文件夹
2. **首选clean版本** - 完全无编码问题
3. **保持文件夹完整性** - 不要移动单个文件
4. **参考对应文档** - 各版本有专门的说明

### 常见问题
- **Q: 程序显示乱码？** A: 使用 `decoder_portable/mqtt_decoder_clean.exe`
- **Q: 程序找不到？** A: 确认在正确的文件夹 (`decoder_portable/`)
- **Q: 解析失败？** A: 检查MQTT数据格式和PSK输入
- **Q: 需要修改？** A: 编辑 `decoder_portable/src/` 下源码后重新编译

---

## 🎉 项目成就

### 技术突破
1. **完全独立** - 实现了真正的零依赖解码器
2. **手动实现** - 从零构建protobuf解析能力
3. **用户友好** - 解决了所有已知的使用问题
4. **高可移植** - 单文件分发，即复制即用
5. **项目结构清晰** - 独立文件夹管理，不污染主项目

### 实用价值
- 适合快速分析Meshtastic MQTT流量
- 便于理解Meshtastic消息格式
- 可作为学习protobuf解析的示例
- 为进一步开发提供了独立基础

**🚀 推荐使用独立版本: `decoder_portable/start_clean_decoder.bat`** 