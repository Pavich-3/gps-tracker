#pragma once

#include <cstddef>
#include <cstdint>
#include <array>
#include "main.h"

class GpsDriver {
private:
	static constexpr size_t BUFFER_SIZE = 256;
	std::array<uint8_t, BUFFER_SIZE> buffer = { 0 };

public:
	GpsDriver() = default;

	void init();
	const uint8_t* getRawBuffer();
};
