#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <stdexcept>

#include "parse_key.hpp"
#include "rw_file.hpp"
#include "magma.hpp"
#include "test_vectors.hpp"
#include "test_correctness.hpp"
#include "benchmark.hpp"
#include "crypto_resistance.hpp"


// --- Parsing key ---
bool parse_key(const std::string& hex_key, uint32_t key[8])
{
    if (hex_key.size() != 64){
        std::cerr << "Err: key must be 64 hex characters (bytes)\n";
        return false;
    }

    uint8_t key_bytes[32];
    for (size_t i = 0; i < 32; ++i){
        std::string byte_str = hex_key.substr(i * 2, 2);
        char* end;
        long val = std::strtol(byte_str.c_str(), &end, 16);
        
        if (end != byte_str.c_str() + 2){
            std::cerr << "Err: invalid hex character in key\n";
            return false;
        }
        key_bytes[i] = static_cast<uint8_t>(val);
    }

    for (size_t i = 0; i < 8; ++i){
        key[i] = (static_cast<uint32_t>(key_bytes[i * 4]) << 24) |
        (static_cast<uint32_t>(key_bytes[i * 4 + 1]) << 16) |
        (static_cast<uint32_t>(key_bytes[i * 4 + 2]) << 8) |
        static_cast<uint32_t>(key_bytes[i * 4 + 3]);
    }

    return true;
}


// --- Main function ---
int main_app(int argc, char* argv[])
{
    if (argc != 5) {
        std::cout << "Err: number of arguments != 5" << std::endl;
        std::cerr << "Usage: " << argv[0] << " <encrypt | decrypt> <input_file> <output_file> <hex_key>\n";
        
        return 1;
    }

    std::string mode = argv[1];
    std::string in_path = argv[2];
    std::string out_path = argv[3];
    std::string hex_key = argv[4];

    bool encrypt_mode;

    if (mode == "encrypt") { encrypt_mode = true; }
    else if (mode == "decrypt") { encrypt_mode = false; }
    else {
        std::cerr << "Err: mode must be 'encrypt' or 'decrypt'\n";
        return 1;
    }

    uint32_t key[8];
    if (!parse_key(hex_key, key)) { return 1; }

    try {
        std::ifstream in(in_path, std::iostream::binary);
        if (!in) throw std::runtime_error("Cannot open input file: " + in_path);

        std::ofstream out(out_path, std::iostream::binary);
        if (!out) throw std::runtime_error("Cannot open output file: " + out_path);

        const size_t BLOCK_SIZE = 8;
        uint8_t block[BLOCK_SIZE];

        if (encrypt_mode) {
            bool first_block = true;
            
            while (true) {
                size_t bytes = read_block(in, block, BLOCK_SIZE);
                
                if (bytes == 0) {
                    apply_padding(block, 0, BLOCK_SIZE);
                    encrypt_block(block, block, key);
                    write_block(out, block, BLOCK_SIZE);
                    break;
                }

                first_block = false;

                if (bytes < BLOCK_SIZE) {
                    apply_padding(block, bytes, BLOCK_SIZE);
                    encrypt_block(block, block, key);
                    write_block(out, block, BLOCK_SIZE);
                    break;
                }

                encrypt_block(block, block, key);
                write_block(out, block, BLOCK_SIZE);
            }
        }
        else {
            uint8_t prev_block[BLOCK_SIZE];
            bool has_prev = false;

            while (true) {
                size_t bytes = read_block(in, block, BLOCK_SIZE);
                
                if (bytes == 0) { break; }
                if (bytes != BLOCK_SIZE) {
                    throw std::runtime_error("Input file size isn't a multiple of block size (corrupted)");
                }

                decrypt_block(block, block, key);

                if (has_prev) { write_block(out, prev_block, BLOCK_SIZE); }

                std::copy(block, block + BLOCK_SIZE, prev_block);
                has_prev = true;
            }

            if (has_prev) {
                size_t valid = remove_padding(prev_block, BLOCK_SIZE);
                out.write(reinterpret_cast<const char*>(prev_block), valid);
                
                if (!out) throw std::runtime_error("write error");
            }
        }

        in.close();
        out.close();
        std::cout << " operation completed successfully!\n";
    }
    catch (const std::exception& e){
        std::cerr << "Err: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}


// --- main ---
int main(int argc, char* argv[], char* envp[])
{
    if (std::string(argv[1]) == "test") {
        bool is_ok = run_known_answer_tests();
        run_boundary_tests();
        return is_ok ? 0 : 1;
    }

    else if (std::string(argv[1]) == "benchmark") {
        run_benchmark();
        return 0;        
    }

    else if (std::string(argv[1]) == "resistance") {
        run_avalanche_test();
        run_statistical_tests();
        return 0;
    }

    return main_app(argc, argv);
}
