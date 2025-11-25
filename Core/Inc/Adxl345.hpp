#pragma once

#include <main.hpp>
#include <cstddef>
#include <cstdint>
#include <array>

struct Dimensions
{
	int8_t x = 0;
	int8_t y = 0;
	int8_t z = 0;
};

class Adxl345
{
public:
	void init(void);

	bool configure(void);

	bool I2C_Write(uint8_t reg_addr, uint8_t data);
	bool I2C_Read(uint8_t reg_addr, uint8_t *buffer);

private:
    static constexpr size_t DATA_SIZE = 6;
	std::array<uint8_t, DATA_SIZE> _raw_buffer = {};
};
