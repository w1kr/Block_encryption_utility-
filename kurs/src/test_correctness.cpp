#include "magma.hpp"
#include "rw_file.hpp"
#include "test_vectors.hpp"
#include "test_correctness.hpp"

#include <iostream>
#include <iomanip>
#include <cstring>

// transformation raw key
static void raw_key_to_u32(const uint8_t raw[32], uint32_t key[8]) 
{
    for (int i = 0; i < 8; ++i) {
        key[i] = (static_cast<uint32_t>(raw[i*4])   << 24) |
                 (static_cast<uint32_t>(raw[i*4+1]) << 16) |
                 (static_cast<uint32_t>(raw[i*4+2]) << 8)  |
                 static_cast<uint32_t>(raw[i*4+3]);
    }
}

// compare function
static bool compare_blocks(const uint8_t* a, const uint8_t* b) { return memcmp(a, b, 8) == 0; }

// known answer tests
bool run_known_answer_tests() {
    std::cout << "=== Known-Answer Tests (Magma) ===\n";
    bool all_passed = true;

    for (size_t i = 0; i < NumVectors; ++i) {
        const auto& vec = KnownVectors[i];
        uint32_t key[8];
        raw_key_to_u32(vec.key, key);

        uint8_t encrypted[8], decrypted[8];
        encrypt_block(vec.plain, encrypted, key);
        decrypt_block(encrypted, decrypted, key);

        bool enc_ok = compare_blocks(encrypted, vec.cipher);
        bool dec_ok = compare_blocks(decrypted, vec.plain);

        std::cout << "[" << vec.description << "]\n";
        std::cout << "  Encrypt: " << (enc_ok ? "PASS" : "FAIL") << '\n';
        std::cout << "  Decrypt: " << (dec_ok ? "PASS" : "FAIL") << '\n';

        if (!enc_ok || !dec_ok) {
            all_passed = false;
            std::cout << "  Expected cipher: ";
            for (int j=0; j<8; ++j) std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)vec.cipher[j];
            std::cout << "\n  Actual cipher:   ";
            for (int j=0; j<8; ++j) std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)encrypted[j];
            std::cout << std::dec << '\n';
        }
    }
    std::cout << "==================================\n";
    return all_passed;
}

// boundary tests (empty, 1 byte, 8 bytes, 17 bytes, 64 - 1 Kbytes)
void run_boundary_tests()
{
    std::cout << "\n=== Boundary Tests ===\n";

    struct TestCase 
    {
        std::string name;
        size_t input_size;
    };

    const TestCase cases[] = {
        {"empty", 0},
        {"1 byte", 1},
        {"exactly 8 bytes", 8},
        {"17 bytes", 17},
        {"1 KB", 1024},
        {"64 KB - 1", 64*1024 - 1}
    };

    const std::string key_hex = "FFEEDDCCBBAA99887766554433221100F0F1F2F3F4F5F6F7F8F9FAFBFCFDFEFF";
    uint32_t key[8];
    extern bool parse_key(const std::string& hex_key, uint32_t key[8]);

    if (!parse_key(key_hex, key)) {
        std::cerr << "Boundary tests: failed to parse key\n";
        return;
    }

    for (const auto& tc : cases) {
        std::string in_name = "boundary_test_" + tc.name + ".tmp";
        std::string enc_name = "boundary_test_" + tc.name + ".enc.tmp";
        std::string dec_name = "boundary_test_" + tc.name + ".dec.tmp";

        {
            std::ofstream f(in_name, std::ios::binary);
            for (size_t i = 0; i < tc.input_size; ++i) {
                uint8_t byte = static_cast<uint8_t>(i & 0xFF);
                f.write(reinterpret_cast<const char*>(&byte), 1);
            }
        }

        {
            std::ifstream in(in_name, std::ios::binary);
            std::ofstream out(enc_name, std::ios::binary);
            if (!in || !out) {
                std::cerr << "  " << tc.name << ": file open error\n";
                continue;
            }
            uint8_t block[8];
            bool first = true;
            while (true) {
                size_t bytes = read_block(in, block, 8);
                if (bytes == 0) {
                    apply_padding(block, 0, 8);
                    encrypt_block(block, block, key);
                    write_block(out, block, 8);
                    break;
                }

                first = false;
                if (bytes < 8) {
                    apply_padding(block, bytes, 8);
                    encrypt_block(block, block, key);
                    write_block(out, block, 8);
                    break;
                }
                
                encrypt_block(block, block, key);
                write_block(out, block, 8);
            }
        }

        {
            std::ifstream in(enc_name, std::ios::binary);
            std::ofstream out(dec_name, std::ios::binary);
            if (!in || !out) {
                std::cerr << "  " << tc.name << ": file open error\n";
                continue;
            }
            uint8_t block[8], prev[8];
            bool has_prev = false;
            while (true) {
                size_t bytes = read_block(in, block, 8);
                if (bytes == 0) break;
                if (bytes != 8) throw std::runtime_error("corrupted");

                decrypt_block(block, block, key);
                
                if (has_prev) {
                    out.write(reinterpret_cast<const char*>(prev), 8);
                }
                memcpy(prev, block, 8);
                has_prev = true;
            }
            if (has_prev) {
                size_t valid = remove_padding(prev, 8);
                out.write(reinterpret_cast<const char*>(prev), valid);
            }
        }

        bool ok = true;
        {
            std::ifstream orig(in_name, std::ios::binary);
            std::ifstream dec(dec_name, std::ios::binary);
            char a, b;

            while (orig.get(a) && dec.get(b)) {
                if (a != b) { ok = false; break; }
            }
            
            if (orig.get(a) || dec.get(b)) ok = false;
        }

        std::cout << "  " << tc.name << ": " << (ok ? "PASS" : "FAIL") << '\n';

        std::remove(in_name.c_str());
        std::remove(enc_name.c_str());
        std::remove(dec_name.c_str());
    }
    std::cout << "======================\n";
}
