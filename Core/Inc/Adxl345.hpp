#pragma once

#include <main.hpp>
#include <cstddef>
#include <cstdint>
#include <array>

class Adxl345
{
public:
	void init(void);

private:
    static constexpr size_t DATA_SIZE = 6;
	std::array<uint8_t, DATA_SIZE> _raw_buffer = {};
};
