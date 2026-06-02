#include "magma.hpp"
#include "parse_key.hpp"
#include <iostream>
#include <iomanip>
#include <vector>
#include <bitset>
#include <cmath>


//  Hamming distance between two blocks (8 bytes)
/*
    The function calculates the number of 
    different bits between two blocks.
*/
int hamming_distance(const uint8_t* a, const uint8_t* b)
{
    int dist = 0;
    for (int i = 0; i < 8; ++i) {
        uint8_t xor_val = a[i] ^ b[i];

        while (xor_val) {
            dist += xor_val & 1;
            xor_val >>= 1;
        }
    }   
    return dist;
}

//  Convert 64-bit integer (little-endian) to byte array
void uint64_to_bytes(uint64_t val, uint8_t out[8]) 
{
    for (int i = 0; i < 8; ++i) {
        out[i] = static_cast<uint8_t>(val & 0xFF);
        val >>= 8;
    }
}

//  Convert a byte array to a 64-bit integer (little-endian)
uint64_t bytes_to_uint64(const uint8_t in[8]) 
{
    uint64_t val = 0;
    for (int i = 7; i >= 0; --i) { val = (val << 8) | in[i]; }
    return val;
}

//  Avalanche test
void run_avalanche_test() 
{
    std::cout << "\n=== Avalanche Test ===\n";

    const std::string key_hex = "FFEEDDCCBBAA99887766554433221100F0F1F2F3F4F5F6F7F8F9FAFBFCFDFEFF";
    uint32_t key[8];
    if (!parse_key(key_hex, key)) {
        std::cerr << "Failed to parse key\n";
        return;
    }

    uint8_t plain[8] = {0x10, 0x32, 0x54, 0x76, 0x98, 0xBA, 0xDC, 0xFE};
    uint8_t cipher_ref[8];
    encrypt_block(plain, cipher_ref, key);

    // Test 1: Inversion of one byte in plaintext 
    std::cout << "Plaintext bit flip (0..63):\n";
    int total_dist_plain = 0;
    for (int bit = 0; bit < 64; ++bit) {
        uint8_t mod_plain[8];
        uint64_t val = bytes_to_uint64(plain);
        val ^= (1ULL << bit);
        uint64_to_bytes(val, mod_plain);

        uint8_t cipher_mod[8];
        encrypt_block(mod_plain, cipher_mod, key);
        int dist = hamming_distance(cipher_ref, cipher_mod);
        total_dist_plain += dist;
        
        if (bit < 10 || bit % 8 == 0) {
            std::cout << "  bit " << std::setw(2) << bit << ": " << dist << " / 64\n";
        }
    }
    double avg_plain = total_dist_plain / 64.0;
    std::cout << "Average changed bits (plaintext): " << std::fixed << std::setprecision(2)
              << avg_plain << " / 64 (" << avg_plain/64.0*100 << "%)\n";

    // Test 2: Inversion one byte in hex_key
    std::cout << "\nKey bit flip (0..255):\n";
    int total_dist_key = 0;
    for (int bit = 0; bit < 256; ++bit) {
        uint32_t mod_key[8];
        memcpy(mod_key, key, sizeof(mod_key));

        uint8_t key_bytes[32];
        for (int i = 0; i < 8; ++i) {
            key_bytes[i*4]   = (key[i] >> 24) & 0xFF;
            key_bytes[i*4+1] = (key[i] >> 16) & 0xFF;
            key_bytes[i*4+2] = (key[i] >> 8) & 0xFF;
            key_bytes[i*4+3] = key[i] & 0xFF;
        }
        int byte_idx = bit / 8;
        int bit_idx = 7 - (bit % 8);
        key_bytes[byte_idx] ^= (1 << bit_idx);

        for (int i = 0; i < 8; ++i) {
            mod_key[i] = (static_cast<uint32_t>(key_bytes[i*4]) << 24) |
                         (static_cast<uint32_t>(key_bytes[i*4+1]) << 16) |
                         (static_cast<uint32_t>(key_bytes[i*4+2]) << 8) |
                         static_cast<uint32_t>(key_bytes[i*4+3]);
        }

        uint8_t cipher_mod[8];
        encrypt_block(plain, cipher_mod, mod_key);

        int dist = hamming_distance(cipher_ref, cipher_mod);
        total_dist_key += dist;

        if (bit < 10 || bit % 32 == 0) { std::cout << "  bit " << std::setw(3) << bit << ": " << dist << " / 64\n"; }
    }

    double avg_key = total_dist_key / 256.0;
    std::cout << "Average changed bits (key): " << std::fixed << std::setprecision(2)
              << avg_key << " / 64 (" << avg_key/64.0*100 << "%)\n"
              << "Avalanche test complete. Ideal average is 50%.\n\n";
}

//  Statistical randomness tests
void run_statistical_tests() 
{
    std::cout << "=== Statistical Randomness Tests ===\n";

    const size_t size = 1024 * 1024;
    std::vector<uint8_t> plain(size);
    std::vector<uint8_t> encrypted(size);

    const std::string key_hex = "FFEEDDCCBBAA99887766554433221100F0F1F2F3F4F5F6F7F8F9FAFBFCFDFEFF";
    uint32_t key[8];
    parse_key(key_hex, key);

    for (size_t i = 0; i < size; i += 8) {
        uint64_t counter = i / 8;
        plain[i]   = counter & 0xFF;
        plain[i+1] = (counter >> 8) & 0xFF;
        plain[i+2] = (counter >> 16) & 0xFF;
        plain[i+3] = (counter >> 24) & 0xFF;
        plain[i+4] = (counter >> 32) & 0xFF;
        plain[i+5] = (counter >> 40) & 0xFF;
        plain[i+6] = (counter >> 48) & 0xFF;
        plain[i+7] = (counter >> 56) & 0xFF;
    }

    for (size_t i = 0; i < size; i += 8) { encrypt_block(plain.data() + i, encrypted.data() + i, key); }

    std::vector<int> bits;
    bits.reserve(size * 8);
    for (uint8_t byte : encrypted) {
        for (int j = 7; j >= 0; --j) { bits.push_back((byte >> j) & 1); }
    }
    size_t n = bits.size();


    // 1. Monobit test
    /*
        Checks that the number of ones is approximately equal to n/2. 
        s_obs is the standardized deviation, the p-value is calculated 
        using the additional error function.
    */
    int ones = 0;
    for (int b : bits) { ones += b; }
    double s_obs = std::abs(2.0 * ones - n) / std::sqrt(n);
    double p_value = std::erfc(s_obs / std::sqrt(2.0));
    std::cout << "Monobit Test: p-value = " << std::fixed << std::setprecision(6) << p_value
              << (p_value > 0.01 ? "  PASS" : "  FAIL") << "\n";


    // 2. Block test (block size 128 bits)
    /*
        We split the bits into blocks of 128 and calculate the proportion of ones in each block. 
        chi_sq sums the squared deviations from the ideal of 0.5. 
        The resulting value is compared with a 3-sigma threshold.
        For large df, the χ² distribution approaches normality with a mean of df and a variance of 2*df. 
        If the value lies within three standard deviations, the test passes.
    */
    int M = 128;
    int N_blocks = n / M;
    double chi_sq = 0;
    for (int i = 0; i < N_blocks; ++i) {
        int block_ones = 0;
        for (int j = 0; j < M; ++j) { block_ones += bits[i * M + j]; }
        double pi = block_ones / (double)M;
        chi_sq += (pi - 0.5) * (pi - 0.5);
    }
    chi_sq *= 4 * M;

    double df = N_blocks;
    double mean_chi = df;
    double std_chi  = std::sqrt(2 * df);
    double upper_bound = mean_chi + 3 * std_chi;
    std::cout << "Block Frequency (M=128): chi^2 = " << chi_sq
              << " (expected around " << mean_chi << ", 3-sigma upper bound = " << upper_bound << ")\n";
    if (chi_sq < upper_bound) { std::cout << "  PASS (within bounds)\n"; }
    else { std::cout << "  FAIL (excessive deviation)\n"; }

    std::cout << "Block Frequency (M=128): chi^2 = " << chi_sq << " (critical ~ " 
              << N_blocks + 3*std::sqrt(2*N_blocks) << ")\n";


    // 3. Runs test
    /*
        Counts the number of runs (sequences of identical bits). 
        The expected number of runs and variance depend on the proportion of ones. 
        The p-value is calculated as a normal approximation.
    */
    int runs = 0;
    for (size_t i = 1; i < n; ++i) { 
        if (bits[i] != bits[i-1]) { runs++; } 
    }
    
    double pi_obs = ones / (double)n;
    double runs_mean = 1 + 2 * (n-1) * pi_obs * (1 - pi_obs);
    double runs_std = std::sqrt(2 * (n-1) * pi_obs * (1 - pi_obs) * (1 - 3*pi_obs*(1 - pi_obs)));
    double z = (runs - runs_mean) / runs_std;
    p_value = std::erfc(std::abs(z) / std::sqrt(2.0));

    std::cout << "Runs Test: p-value = " << p_value
              << (p_value > 0.01 ? "  PASS" : "  FAIL") << "\n\n"
              << "Statistical tests complete.\n";
}
