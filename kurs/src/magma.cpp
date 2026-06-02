#include "magma.hpp"

// id-tc26-gost-28147-param-z
static const uint8_t sbox[8][16] = 
{
    {12,4,6,2,10,5,11,9,14,8,13,7,0,3,15,1},
    {6,8,2,3,9,10,5,12,1,14,4,7,11,13,0,15},
    {11,3,5,8,2,15,10,13,14,1,7,4,12,9,6,0},
    {12,8,2,1,13,4,15,6,7,0,10,5,3,14,9,11},
    {7,15,5,10,8,1,6,13,0,9,3,14,11,4,2,12},
    {5,13,15,6,9,2,12,10,11,7,8,1,4,3,14,0},
    {8,14,2,5,6,9,1,12,15,4,11,0,13,10,3,7},
    {1,7,14,13,0,5,8,3,4,15,10,6,9,12,11,2}
};

// --- Main step of encryption f ---
static uint32_t f(uint32_t block, uint32_t key)
{
    uint32_t temp = block + key;
    uint32_t result = 0;

    for (int i = 0; i < 8; ++i) {
        uint8_t nibble = (temp >> (4 * i)) & 0x0F;
        nibble = sbox[i][nibble];
        result |= static_cast<uint32_t>(nibble) << (4 * i);
    }

    result = (result << 11) | (result >> (32 - 11));
    
    return result;
}

// --- loading 32-bit words in Little-Endian ---
static uint32_t load_le(const uint8_t* p)
{
    return static_cast<uint32_t>(p[0]) |
          (static_cast<uint32_t>(p[1]) << 8) |
          (static_cast<uint32_t>(p[2]) << 16) |
          (static_cast<uint32_t>(p[3]) << 24);
}

// --- Saving 32-bit words in Little-Endian ---
static void store_le(uint32_t v, uint8_t* p)
{
    p[0] =  v & 0xFF;
    p[1] = (v >> 8) & 0xFF;
    p[2] = (v >> 16) & 0xFF;
    p[3] = (v >> 24) & 0xFF;
}

// --- Encryption of block ---
void encrypt_block(const uint8_t* in, uint8_t* out, const uint32_t* key) 
{
    uint32_t n1 = load_le(in);
    uint32_t n2 = load_le(in + 4);

    for (int round = 0; round < 32; ++round) {
        uint32_t rk;

        if (round < 24) { rk = key[round % 8]; }
        else            { rk = key[7 - (round % 8)]; }

        uint32_t temp = n2 ^ f(n1, rk);
        if (round < 31) {
            n2 = n1;
            n1 = temp;
        }
        else { n2 = temp; }
    }

    store_le(n1, out);
    store_le(n2, out + 4);
}

// --- Decryption of block ---
void decrypt_block(const uint8_t* in, uint8_t* out, const uint32_t* key) 
{
    uint32_t n1 = load_le(in);
    uint32_t n2 = load_le(in + 4);

    for (int round = 0; round < 32; ++round) {
        uint32_t rk;
        
        if (round < 8) { rk = key[round % 8]; }
        else           { rk = key[7 - (round % 8)]; }

        uint32_t temp = n2 ^ f(n1, rk);
        if (round < 31) {
            n2 = n1;
            n1 = temp;
        } 
        else { n2 = temp; }
    }

    store_le(n1, out);
    store_le(n2, out + 4);
}
