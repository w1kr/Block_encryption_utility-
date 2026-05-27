#pragma once

#include <cstdint>
#include <string>

bool parse_key(const std::string& hex_key, uint32_t key[8]);
