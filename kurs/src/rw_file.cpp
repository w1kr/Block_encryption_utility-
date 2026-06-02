#include "rw_file.hpp"

// --- Read block ---
size_t read_block(std::ifstream& in, uint8_t* buffer, size_t block_size)
{
    in.read(reinterpret_cast<char*>(buffer), block_size);
    std::streamsize bytes = in.gcount();

    return static_cast<size_t>(bytes);
}

// --- Write block ---
void write_block(std::ofstream& out, const uint8_t* buffer, size_t block_size)
{
    out.write(reinterpret_cast<const char*>(buffer), block_size);
    if (!out) { throw std::runtime_error("write to file"); }
}

// --- Add supplement to block ---
void apply_padding(uint8_t* block, size_t valid_bytes, size_t block_size)
{
    uint8_t pad_value = static_cast<uint8_t>(block_size - valid_bytes);
    for (size_t i = valid_bytes; i < block_size; ++i) {
        block[i] = pad_value;
    }
}

// --- Remove supplement to block ---
size_t remove_padding(const uint8_t* block, size_t block_size)
{
    uint8_t pad_value = block[block_size - 1];
    if (pad_value == 0 || pad_value > block_size) { throw std::runtime_error("invalid PKCS#7 padding"); } 

    for (size_t i = block_size - pad_value; i < block_size - 1; ++i) {
        if (block[i] != pad_value){ throw std::runtime_error("invalid PKCS#7 padding"); }
    }

    return block_size - pad_value;
}
