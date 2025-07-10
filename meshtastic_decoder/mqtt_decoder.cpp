#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <algorithm>
#include <sstream>

using namespace std;

// 将十六进制字符串转换为字节数组
vector<uint8_t> hexToBytes(string hex) {
    vector<uint8_t> bytes;
    // 移除所有空格、换行等空白字符
    hex.erase(remove_if(hex.begin(), hex.end(), ::isspace), hex.end());
    
    for (size_t i = 0; i < hex.length(); i += 2) {
        if (i + 1 < hex.length()) {
            string byteStr = hex.substr(i, 2);
            bytes.push_back((uint8_t)strtol(byteStr.c_str(), nullptr, 16));
        }
    }
    return bytes;
}

// 打印字节数组
void printHex(const vector<uint8_t>& data, const string& label = "") {
    if (!label.empty()) {
        cout << label << ": ";
    }
    for (size_t i = 0; i < data.size(); i++) {
        cout << hex << setw(2) << setfill('0') << (int)data[i];
        if (i < data.size() - 1) cout << " ";
    }
    cout << dec << endl;
}

// 解析varint
uint64_t decodeVarint(const uint8_t*& data, size_t& remaining) {
    uint64_t result = 0;
    int shift = 0;
    
    while (remaining > 0 && shift < 64) {
        uint8_t byte = *data++;
        remaining--;
        
        result |= ((uint64_t)(byte & 0x7F)) << shift;
        
        if ((byte & 0x80) == 0) {
            break;
        }
        shift += 7;
    }
    
    return result;
}

// ServiceEnvelope结构
struct ServiceEnvelope {
    vector<uint8_t> packetData;
    string channelId;
    string gatewayId;
    bool valid = false;
};

// MeshPacket结构
struct MeshPacket {
    uint32_t from = 0;
    uint32_t to = 0;
    uint64_t id = 0;
    uint32_t channel = 0;
    uint32_t hopLimit = 0;
    uint32_t hopStart = 0;
    bool wantAck = false;
    vector<uint8_t> encryptedData;
    bool valid = false;
};

// 解析ServiceEnvelope
ServiceEnvelope parseServiceEnvelope(const uint8_t* data, size_t length) {
    ServiceEnvelope envelope;
    const uint8_t* ptr = data;
    size_t remaining = length;
    
    cout << "\n=== ServiceEnvelope解析 ===" << endl;
    
    while (remaining > 0) {
        uint64_t fieldInfo = decodeVarint(ptr, remaining);
        int fieldNumber = fieldInfo >> 3;
        int wireType = fieldInfo & 0x7;
        
        if (wireType == 2) { // Length-delimited
            uint64_t length = decodeVarint(ptr, remaining);
            
            if (remaining >= length) {
                if (fieldNumber == 1) { // packet
                    envelope.packetData.assign(ptr, ptr + length);
                    cout << "✓ 找到MeshPacket (" << length << " 字节)" << endl;
                } else if (fieldNumber == 2) { // channel_id
                    envelope.channelId.assign((char*)ptr, length);
                    cout << "✓ Channel ID: " << envelope.channelId << endl;
                } else if (fieldNumber == 3) { // gateway_id
                    envelope.gatewayId.assign((char*)ptr, length);
                    cout << "✓ Gateway ID: " << envelope.gatewayId << endl;
                }
                
                ptr += length;
                remaining -= length;
            } else {
                break;
            }
        } else {
            break;
        }
    }
    
    envelope.valid = !envelope.packetData.empty();
    return envelope;
}

// 解析MeshPacket
MeshPacket parseMeshPacket(const uint8_t* data, size_t length) {
    MeshPacket packet;
    const uint8_t* ptr = data;
    size_t remaining = length;
    
    cout << "\n=== MeshPacket解析 ===" << endl;
    
    while (remaining > 0) {
        uint64_t fieldInfo = decodeVarint(ptr, remaining);
        int fieldNumber = fieldInfo >> 3;
        int wireType = fieldInfo & 0x7;
        
        switch (fieldNumber) {
            case 1: // from
                if (wireType == 0) {
                    packet.from = (uint32_t)decodeVarint(ptr, remaining);
                    cout << "✓ From: 0x" << hex << packet.from << dec << " (!" << hex << packet.from << dec << ")" << endl;
                }
                break;
                
            case 2: // to  
                if (wireType == 0) {
                    packet.to = (uint32_t)decodeVarint(ptr, remaining);
                    cout << "✓ To: 0x" << hex << packet.to << dec;
                    if (packet.to == 0xFFFFFFFF) {
                        cout << " (广播)";
                    }
                    cout << endl;
                }
                break;
                
            case 4: // encrypted
                if (wireType == 2) {
                    uint64_t length = decodeVarint(ptr, remaining);
                    if (remaining >= length) {
                        packet.encryptedData.assign(ptr, ptr + length);
                        cout << "✓ 加密数据: " << length << " 字节" << endl;
                        ptr += length;
                        remaining -= length;
                    }
                }
                break;
                
            case 6: // id
                if (wireType == 1) { // fixed64
                    if (remaining >= 8) {
                        packet.id = *(uint64_t*)ptr;
                        cout << "✓ ID: 0x" << hex << packet.id << dec << endl;
                        ptr += 8;
                        remaining -= 8;
                    }
                } else if (wireType == 0) { // varint
                    packet.id = decodeVarint(ptr, remaining);
                    cout << "✓ ID: 0x" << hex << packet.id << dec << endl;
                }
                break;
                
            case 7: // channel
                if (wireType == 0) {
                    packet.channel = (uint32_t)decodeVarint(ptr, remaining);
                    cout << "✓ Channel: 0x" << hex << packet.channel << dec << endl;
                }
                break;
                
            case 8: // hop_limit
                if (wireType == 0) {
                    packet.hopLimit = (uint32_t)decodeVarint(ptr, remaining);
                    cout << "✓ Hop Limit: " << packet.hopLimit << endl;
                }
                break;
                
            case 9: // hop_start
                if (wireType == 0) {
                    packet.hopStart = (uint32_t)decodeVarint(ptr, remaining);
                    cout << "✓ Hop Start: " << packet.hopStart << endl;
                }
                break;
                
            case 10: // want_ack
                if (wireType == 0) {
                    packet.wantAck = decodeVarint(ptr, remaining) != 0;
                    cout << "✓ Want ACK: " << (packet.wantAck ? "true" : "false") << endl;
                }
                break;
                
            default:
                // 跳过未知字段
                if (wireType == 0) {
                    decodeVarint(ptr, remaining);
                } else if (wireType == 2) {
                    uint64_t length = decodeVarint(ptr, remaining);
                    if (remaining >= length) {
                        ptr += length;
                        remaining -= length;
                    }
                } else if (wireType == 1) {
                    if (remaining >= 8) {
                        ptr += 8;
                        remaining -= 8;
                    }
                } else if (wireType == 5) {
                    if (remaining >= 4) {
                        ptr += 4;
                        remaining -= 4;
                    }
                }
                break;
        }
    }
    
    packet.valid = true;
    return packet;
}

// 分析加密数据内容
void analyzeEncryptedData(const vector<uint8_t>& encryptedData, const string& expectedContent = "") {
    cout << "\n=== 加密数据分析 ===" << endl;
    printHex(encryptedData, "加密数据");
    
    if (encryptedData.size() >= 5) {
        cout << "\n推测的解密结构:" << endl;
        if (encryptedData[0] == 0x08 && encryptedData[1] == 0x01 && encryptedData[2] == 0x12) {
            cout << "08 01 -> portnum = 1 (TEXT_MESSAGE_APP)" << endl;
            cout << "12    -> payload字段 (field 2, length-delimited)" << endl;
            
            if (encryptedData.size() > 3) {
                int textLength = encryptedData[3];
                cout << hex << setw(2) << setfill('0') << (int)encryptedData[3] << dec << " -> 文本长度: " << textLength << " 字节" << endl;
                
                if (!expectedContent.empty()) {
                    cout << "\n如果消息是 \"" << expectedContent << "\":" << endl;
                    for (char c : expectedContent) {
                        cout << hex << setw(2) << setfill('0') << (int)c << " ";
                    }
                    cout << " <- 应该解密为这些字节" << dec << endl;
                }
            }
        }
    }
}

// 获取PSK信息
vector<uint8_t> getPSKFromInput(const string& pskInput) {
    vector<uint8_t> psk;
    
    if (pskInput == "AQ==" || pskInput == "1") {
        // PSK #1 - 扩展为默认PSK
        const uint8_t defaultpsk[] = {0xd4, 0xf1, 0xbb, 0x3a, 0x20, 0x29, 0x07, 0x59,
                                      0xf0, 0xbc, 0xff, 0xab, 0xcf, 0x4e, 0x69, 0x01};
        psk.assign(defaultpsk, defaultpsk + 16);
        cout << "使用PSK #1 (默认PSK)" << endl;
    } else if (pskInput.length() == 32) {
        // 16字节十六进制
        psk = hexToBytes(pskInput);
        cout << "使用自定义16字节PSK" << endl;
    } else {
        cout << "PSK格式不正确，使用默认PSK #1" << endl;
        const uint8_t defaultpsk[] = {0xd4, 0xf1, 0xbb, 0x3a, 0x20, 0x29, 0x07, 0x59,
                                      0xf0, 0xbc, 0xff, 0xab, 0xcf, 0x4e, 0x69, 0x01};
        psk.assign(defaultpsk, defaultpsk + 16);
    }
    
    return psk;
}

int main() {
    cout << "=================================" << endl;
    cout << "    Meshtastic MQTT解码器" << endl;
    cout << "=================================" << endl;
    cout << "输入MQTT消息的十六进制数据进行解码" << endl;
    cout << "支持ServiceEnvelope格式的protobuf消息" << endl;
    cout << "=================================" << endl;
    
    while (true) {
        cout << "\n请输入MQTT消息的十六进制数据 (输入'quit'退出):" << endl;
        cout << "> ";
        
        string hexInput;
        getline(cin, hexInput);
        
        if (hexInput == "quit" || hexInput == "exit") {
            cout << "退出解码器。" << endl;
            break;
        }
        
        if (hexInput.empty()) {
            cout << "请输入有效的十六进制数据。" << endl;
            continue;
        }
        
        try {
            // 转换十六进制数据
            vector<uint8_t> data = hexToBytes(hexInput);
            
            if (data.empty()) {
                cout << "无效的十六进制数据。" << endl;
                continue;
            }
            
            cout << "\n解析消息长度: " << data.size() << " 字节" << endl;
            printHex(data, "原始数据");
            
            // 解析ServiceEnvelope
            ServiceEnvelope envelope = parseServiceEnvelope(data.data(), data.size());
            
            if (!envelope.valid) {
                cout << "❌ ServiceEnvelope解析失败！" << endl;
                continue;
            }
            
            // 解析MeshPacket
            MeshPacket packet = parseMeshPacket(envelope.packetData.data(), envelope.packetData.size());
            
            if (!packet.valid) {
                cout << "❌ MeshPacket解析失败！" << endl;
                continue;
            }
            
            // 显示解析结果
            cout << "\n=== 解析结果总结 ===" << endl;
            cout << "📍 来源节点: !" << hex << packet.from << dec << endl;
            cout << "📍 目标: ";
            if (packet.to == 0xFFFFFFFF) {
                cout << "广播消息" << endl;
            } else {
                cout << "!" << hex << packet.to << dec << endl;
            }
            cout << "📍 频道: " << envelope.channelId << endl;
            cout << "📍 网关: " << envelope.gatewayId << endl;
            cout << "📍 消息ID: 0x" << hex << packet.id << dec << endl;
            
            // 分析加密数据
            if (!packet.encryptedData.empty()) {
                cout << "\n请输入预期的消息内容 (用于验证，可留空): ";
                string expectedContent;
                getline(cin, expectedContent);
                
                analyzeEncryptedData(packet.encryptedData, expectedContent);
                
                cout << "\n请输入PSK信息 (AQ==表示PSK #1，或输入16字节hex): ";
                string pskInput;
                getline(cin, pskInput);
                
                vector<uint8_t> psk = getPSKFromInput(pskInput);
                printHex(psk, "使用的PSK");
                
                cout << "\n💡 提示: 需要使用CryptoEngine进行AES-CTR解密" << endl;
                cout << "💡 Nonce构成: [包ID][发送节点][额外随机数]" << endl;
            }
            
        } catch (const exception& e) {
            cout << "❌ 解析出错: " << e.what() << endl;
        }
        
        cout << "\n" << string(50, '-') << endl;
    }
    
    return 0;
} 