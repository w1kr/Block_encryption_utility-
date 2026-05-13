#pragma once

#include <cstdint>
#include <cstring>

// --- encryption ---
void encrypt_block(const uint8_t* in, uint8_t* out, const uint32_t* key);

// --- decryption ---
void decrypt_block(const uint8_t* in, uint8_t* out, const uint32_t* key);
