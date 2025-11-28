#pragma once

#include <main.hpp>
#include <cstddef>
#include <cstdint>
#include <array>

#define LED_PIN LL_GPIO_PIN_13

struct Acceleration
{
	float x_g = 0.0f;
	float y_g = 0.0f;
	float z_g = 0.0f;
};

class Adxl345
{
public:
	void init(void);

	bool configure(void);

	Acceleration getAcceleration(void) { return processRawData(); }

private:
    static constexpr size_t DATA_SIZE = 6;
	std::array<uint8_t, DATA_SIZE> _raw_buffer = {};

	bool I2C_Write(uint8_t reg_addr, uint8_t data);
	bool I2C_Read(uint8_t reg_addr, uint8_t *buffer);
	bool I2C_ReadBuffer(uint8_t reg_addr, uint8_t *buffer, size_t length);

	Acceleration processRawData(void);
};
