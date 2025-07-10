#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <algorithm>

using namespace std;

// Simple AES-CTR implementation for demonstration
// Note: This is a simplified version for educational purposes
class SimpleAES {
private:
    static const uint8_t sbox[256];
    uint8_t key[16];
    
public:
    SimpleAES(const vector<uint8_t>& keyData) {
        memcpy(key, keyData.data(), min(16, (int)keyData.size()));
    }
    
    void xorWithKey(uint8_t* data, size_t len) {
        for (size_t i = 0; i < len; i++) {
            data[i] ^= key[i % 16];
        }
    }
    
    // Simplified CTR mode - just XOR with key for demonstration
    void decryptCTR(const uint8_t* input, uint8_t* output, size_t len, uint64_t nonce) {
        memcpy(output, input, len);
        
        // Simple XOR with key (not real AES, but demonstrates the concept)
        for (size_t i = 0; i < len; i++) {
            uint8_t keyByte = key[i % 16];
            uint8_t nonceByte = (uint8_t)((nonce >> (i % 8)) & 0xFF);
            output[i] ^= keyByte ^ nonceByte;
        }
    }
};

const uint8_t SimpleAES::sbox[256] = {
    // Simplified S-box for demonstration
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76
    // ... (truncated for brevity)
};

vector<uint8_t> hexToBytes(string hex) {
    vector<uint8_t> result;
    hex.erase(remove_if(hex.begin(), hex.end(), [](char c) { return isspace(c); }), hex.end());
    
    for (size_t i = 0; i < hex.length(); i += 2) {
        if (i + 1 < hex.length()) {
            string byteString = hex.substr(i, 2);
            uint8_t byte = (uint8_t)strtol(byteString.c_str(), nullptr, 16);
            result.push_back(byte);
        }
    }
    return result;
}

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

struct ServiceEnvelope {
    vector<uint8_t> packetData;
    string channelId;
    string gatewayId;
    bool valid = false;
};

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

ServiceEnvelope parseServiceEnvelope(const uint8_t* data, size_t length) {
    ServiceEnvelope envelope;
    const uint8_t* ptr = data;
    size_t remaining = length;
    
    cout << "\n=== ServiceEnvelope Parsing ===" << endl;
    
    while (remaining > 0) {
        uint64_t tag = decodeVarint(ptr, remaining);
        uint32_t fieldNumber = (uint32_t)(tag >> 3);
        uint32_t wireType = tag & 0x7;
        
        switch (fieldNumber) {
            case 1:
                if (wireType == 2) {
                    uint64_t length = decodeVarint(ptr, remaining);
                    if (remaining >= length) {
                        envelope.packetData.assign(ptr, ptr + length);
                        cout << "SUCCESS: Found MeshPacket (" << length << " bytes)" << endl;
                        ptr += length;
                        remaining -= length;
                    }
                }
                break;
                
            case 2:
                if (wireType == 2) {
                    uint64_t length = decodeVarint(ptr, remaining);
                    if (remaining >= length) {
                        envelope.channelId.assign((char*)ptr, length);
                        cout << "SUCCESS: Channel ID: " << envelope.channelId << endl;
                        ptr += length;
                        remaining -= length;
                    }
                }
                break;
                
            case 3:
                if (wireType == 2) {
                    uint64_t length = decodeVarint(ptr, remaining);
                    if (remaining >= length) {
                        envelope.gatewayId.assign((char*)ptr, length);
                        cout << "SUCCESS: Gateway ID: " << envelope.gatewayId << endl;
                        ptr += length;
                        remaining -= length;
                    }
                }
                break;
                
            default:
                if (wireType == 0) {
                    decodeVarint(ptr, remaining);
                } else if (wireType == 2) {
                    uint64_t length = decodeVarint(ptr, remaining);
                    if (remaining >= length) {
                        ptr += length;
                        remaining -= length;
                    }
                }
                break;
        }
    }
    
    envelope.valid = !envelope.packetData.empty();
    return envelope;
}

MeshPacket parseMeshPacket(const uint8_t* data, size_t length) {
    MeshPacket packet;
    const uint8_t* ptr = data;
    size_t remaining = length;
    
    cout << "\n=== MeshPacket Parsing ===" << endl;
    
    while (remaining > 0) {
        uint64_t tag = decodeVarint(ptr, remaining);
        uint32_t fieldNumber = (uint32_t)(tag >> 3);
        uint32_t wireType = tag & 0x7;
        
        cout << "DEBUG: Field " << fieldNumber << ", WireType " << wireType << endl;
        
        switch (fieldNumber) {
            case 1:
                if (wireType == 0) {
                    packet.from = (uint32_t)decodeVarint(ptr, remaining);
                    cout << "SUCCESS: From: 0x" << hex << packet.from << dec << " (!" << hex << packet.from << dec << ")" << endl;
                }
                break;
                
            case 2:
                if (wireType == 0) {
                    packet.to = (uint32_t)decodeVarint(ptr, remaining);
                    cout << "SUCCESS: To: 0x" << hex << packet.to << dec;
                    if (packet.to == 0xFFFFFFFF) {
                        cout << " (broadcast)";
                    }
                    cout << endl;
                }
                break;
                
            case 3:
                if (wireType == 2) {
                    uint64_t length = decodeVarint(ptr, remaining);
                    if (remaining >= length) {
                        packet.encryptedData.assign(ptr, ptr + length);
                        cout << "SUCCESS: Encrypted data (field 3): " << length << " bytes" << endl;
                        ptr += length;
                        remaining -= length;
                    }
                }
                break;
                
            case 4:
                if (wireType == 2) {
                    uint64_t length = decodeVarint(ptr, remaining);
                    if (remaining >= length) {
                        packet.encryptedData.assign(ptr, ptr + length);
                        cout << "SUCCESS: Encrypted data (field 4): " << length << " bytes" << endl;
                        ptr += length;
                        remaining -= length;
                    }
                }
                break;
                
            case 5:
                if (wireType == 2) {
                    uint64_t length = decodeVarint(ptr, remaining);
                    if (remaining >= length) {
                        packet.encryptedData.assign(ptr, ptr + length);
                        cout << "SUCCESS: Encrypted data (field 5): " << length << " bytes" << endl;
                        ptr += length;
                        remaining -= length;
                    }
                }
                break;
                
            case 6:
                if (wireType == 1) {
                    if (remaining >= 8) {
                        packet.id = *(uint64_t*)ptr;
                        cout << "SUCCESS: ID: 0x" << hex << packet.id << dec << endl;
                        ptr += 8;
                        remaining -= 8;
                    }
                } else if (wireType == 0) {
                    packet.id = decodeVarint(ptr, remaining);
                    cout << "SUCCESS: ID: 0x" << hex << packet.id << dec << endl;
                }
                break;
                
            default:
                cout << "DEBUG: Skipping unknown field " << fieldNumber << endl;
                if (wireType == 0) {
                    decodeVarint(ptr, remaining);
                } else if (wireType == 2) {
                    uint64_t length = decodeVarint(ptr, remaining);
                    if (remaining >= length) {
                        // 检查是否是可能的加密数据
                        if (length > 0 && length < 1000) {
                            packet.encryptedData.assign(ptr, ptr + length);
                            cout << "INFO: Found potential encrypted data in field " << fieldNumber << ": " << length << " bytes" << endl;
                        }
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
    
    cout << "DEBUG: Final encrypted data size: " << packet.encryptedData.size() << " bytes" << endl;
    
    packet.valid = true;
    return packet;
}

bool attemptDecryption(const vector<uint8_t>& encryptedData, const vector<uint8_t>& psk, uint64_t messageId, uint32_t fromNode, const string& expectedContent = "") {
    cout << "\n=== ATTEMPTING DECRYPTION ===" << endl;
    printHex(encryptedData, "Encrypted data");
    printHex(psk, "PSK");
    
    if (encryptedData.empty() || psk.size() < 16) {
        cout << "ERROR: Invalid data or PSK for decryption!" << endl;
        return false;
    }
    
    // Create AES decryptor
    SimpleAES aes(psk);
    
    // Create nonce from message ID and sender
    uint64_t nonce = messageId ^ ((uint64_t)fromNode << 32);
    cout << "Nonce: 0x" << hex << nonce << dec << endl;
    
    // Decrypt the data
    vector<uint8_t> decrypted(encryptedData.size());
    aes.decryptCTR(encryptedData.data(), decrypted.data(), encryptedData.size(), nonce);
    
    printHex(decrypted, "Decrypted raw");
    
    // Try to interpret as text
    string decryptedText;
    bool validText = true;
    for (uint8_t byte : decrypted) {
        if (byte >= 32 && byte <= 126) {  // Printable ASCII
            decryptedText += (char)byte;
        } else if (byte == 0) {
            break;  // Null terminator
        } else {
            validText = false;
            break;
        }
    }
    
    cout << "\n=== DECRYPTION RESULTS ===" << endl;
    if (validText && !decryptedText.empty()) {
        cout << "SUCCESS: Decrypted text: \"" << decryptedText << "\"" << endl;
        
        if (!expectedContent.empty()) {
            if (decryptedText == expectedContent) {
                cout << "SUCCESS: Matches expected content!" << endl;
                return true;
            } else {
                cout << "WARNING: Does not match expected content (" << expectedContent << ")" << endl;
            }
        }
        return true;
    } else {
        cout << "INFO: Could not interpret as readable text" << endl;
        cout << "Raw decrypted bytes: ";
        for (size_t i = 0; i < min((size_t)20, decrypted.size()); i++) {
            cout << "0x" << hex << setw(2) << setfill('0') << (int)decrypted[i] << " ";
        }
        cout << dec << endl;
        return false;
    }
}

vector<uint8_t> getPSKFromInput(const string& pskInput) {
    const uint8_t defaultpsk[] = {
        0xd4, 0xf1, 0xbb, 0x3a, 0x20, 0x29, 0x07, 0x59,
        0xf0, 0xbc, 0xff, 0xab, 0xcf, 0x4e, 0x69, 0x01
    };
    
    if (pskInput == "AQ==") {
        cout << "Using PSK #1 (default shared key)" << endl;
        return vector<uint8_t>(defaultpsk, defaultpsk + 16);
    } else {
        cout << "Using custom PSK: " << pskInput << endl;
        return hexToBytes(pskInput);
    }
}

int main() {
    cout << "========================================" << endl;
    cout << "   Meshtastic MQTT Decoder v2.0        " << endl;
    cout << "     WITH DECRYPTION SUPPORT            " << endl;
    cout << "========================================" << endl;
    cout << "This version attempts to decrypt message content" << endl;
    cout << "========================================" << endl;
    
    string input;
    while (true) {
        cout << "\nEnter MQTT message hex data (type 'quit' to exit):" << endl;
        cout << "> ";
        
        getline(cin, input);
        
        if (input == "quit" || input == "exit") {
            cout << "Exiting decoder." << endl;
            break;
        }
        
        if (input.empty()) {
            continue;
        }
        
        vector<uint8_t> data = hexToBytes(input);
        
        if (data.empty()) {
            cout << "ERROR: Invalid hex data!" << endl;
            continue;
        }
        
        cout << "\nParsing message length: " << data.size() << " bytes" << endl;
        printHex(data, "Raw data");
        
        ServiceEnvelope envelope = parseServiceEnvelope(data.data(), data.size());
        
        if (!envelope.valid) {
            cout << "ERROR: Failed to parse ServiceEnvelope!" << endl;
            continue;
        }
        
        MeshPacket packet = parseMeshPacket(envelope.packetData.data(), envelope.packetData.size());
        
        if (!packet.valid) {
            cout << "ERROR: Failed to parse MeshPacket!" << endl;
            continue;
        }
        
        string expectedContent;
        cout << "\nEnter expected message content: ";
        getline(cin, expectedContent);
        
        string pskInput;
        cout << "Enter PSK info: ";
        getline(cin, pskInput);
        
        if (!pskInput.empty() && !packet.encryptedData.empty()) {
            vector<uint8_t> psk = getPSKFromInput(pskInput);
            attemptDecryption(packet.encryptedData, psk, packet.id, packet.from, expectedContent);
        }
        
        cout << "\n=== Final Summary ===" << endl;
        cout << "Source node: !" << hex << packet.from << dec << endl;
        if (packet.to == 0xFFFFFFFF) {
            cout << "Target: broadcast message" << endl;
        } else {
            cout << "Target: !" << hex << packet.to << dec << endl;
        }
        cout << "Channel: " << envelope.channelId << endl;
        cout << "Gateway: " << envelope.gatewayId << endl;
    }
    
    return 0;
} 