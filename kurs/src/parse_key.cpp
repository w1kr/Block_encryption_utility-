#include "parse_key.hpp"
#include <iostream>
#include <cstdlib>

// transformation string to key
bool parse_key(const std::string& hex_key, uint32_t key[8]) 
{
    if (hex_key.size() != 64) {
        std::cerr << "Err: key must be 64 hex characters (bytes)\n";
        return false;
    }

    uint8_t key_bytes[32];
    for (size_t i = 0; i < 32; ++i) {
        std::string byte_str = hex_key.substr(i * 2, 2);
        char* end;
        long val = std::strtol(byte_str.c_str(), &end, 16);
        if (end != byte_str.c_str() + 2) {
            std::cerr << "Err: invalid hex character in key\n";
            
            return false;
        }

        key_bytes[i] = static_cast<uint8_t>(val);
    }

    for (int i = 0; i < 8; ++i) {
        key[i] = (static_cast<uint32_t>(key_bytes[i*4]) << 24) |
                 (static_cast<uint32_t>(key_bytes[i*4+1]) << 16) |
                 (static_cast<uint32_t>(key_bytes[i*4+2]) << 8) |
                 static_cast<uint32_t>(key_bytes[i*4+3]);
    }
    
    return true;
}
