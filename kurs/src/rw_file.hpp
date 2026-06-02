#pragma once

#include <fstream>
#include <cstdint>
#include <cstddef>
#include <stdexcept>

// ---   read from file ---
size_t read_block(std::ifstream& in, uint8_t* buffer, size_t block_size);
void apply_padding(uint8_t* block, size_t valid_bytes, size_t block_size);

// ---   write to file  ---
void write_block(std::ofstream& out, const uint8_t* buffer, size_t block_size);
size_t remove_padding(const uint8_t* block, size_t block_size);
