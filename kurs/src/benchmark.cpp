#include <iostream>
#include <iomanip>
#include <vector>
#include <chrono>
#include <cstdlib>

#include "magma.hpp"
#include "parse_key.hpp"


// one operation measurement
static double measure_operation(
    const uint8_t* input, uint8_t* output, size_t data_size,
    const uint32_t* key, bool encrypt,
    int iterations = 2)
{
    auto start = std::chrono::high_resolution_clock::now();
    for (int iter = 0; iter < iterations; ++iter) {
        const uint8_t* in = input;
        uint8_t* out = output;
        size_t processed = 0;

        while (processed < data_size) {
            if (encrypt)
                encrypt_block(in, out, key);
            else
                decrypt_block(in, out, key);
            in += 8;
            out += 8;
            processed += 8;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    double seconds = std::chrono::duration<double>(end - start).count();

    return (data_size * iterations) / (seconds * 1024 * 1024);
}

// main benchmark function
void run_benchmark() 
{
    const std::string key_hex = "FFEEDDCCBBAA99887766554433221100F0F1F2F3F4F5F6F7F8F9FAFBFCFDFEFF";
    uint32_t key[8];
    if (!parse_key(key_hex, key)) {
        std::cerr << "Error: parsing key in benchmark\n";
        return;
    }

    // Sizes of tests data: 1 KB, 10 KB, 100 KB, 1 MB, 10 MB.
    std::vector<size_t> sizes = {1024, 10240, 102400, 1048576, 10485760};

    std::cout << "================================================\n";
    std::cout << "      Benchmark 'Magma' (ECB, in memory)\n";
    std::cout << "================================================\n";
    std::cout << std::left << std::setw(14) << "Size"
              << std::right << std::setw(15) << "Encryption (MB/s)"
              << std::right << std::setw(18) << "Decryption (MB/s)\n";
    std::cout << std::string(14 + 15 + 18, '-') << "\n";

    for (auto data_size : sizes) {
        std::vector<uint8_t> data(data_size);
        for (size_t i = 0; i < data_size; ++i)
            data[i] = static_cast<uint8_t>(rand() & 0xFF);

        std::vector<uint8_t>encrypted(data_size);
        std::vector<uint8_t>decrypted(data_size);

        double enc_speed = measure_operation(data.data(), encrypted.data(), data_size, key, true);
        double dec_speed = measure_operation(encrypted.data(), decrypted.data(), data_size, key, false);

        std::cout << std::left << std::setw(14);
        if (data_size < 1048576) {std::cout << std::to_string(data_size / 1024) + " KB"; }
        else { std::cout << std::to_string(data_size / 1048576) + " MB"; }

        std::cout << std::right << std::fixed << std::setprecision(2)
                  << std::setw(15) << enc_speed
                  << std::setw(18) << dec_speed << "\n";
    }
    std::cout << "================================================\n";
}
