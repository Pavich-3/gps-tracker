#pragma once

#include <main.hpp>
#include <cstddef>
#include <cstdint>
#include <array>

struct Coordinates {
	float latitude { 0.0f };
	float longitude { 0.0f };
	uint32_t time_sec { 0 };
};

class GpsDriver {
public:
	GpsDriver() = default;

	void init();
	const uint8_t* getRawBuffer();

	bool parse();
	Coordinates getCoordinates() const {
		return _currentData;
	}
	;

private:
	static constexpr size_t BUFFER_SIZE = 256;
	std::array<uint8_t, BUFFER_SIZE> buffer{ 0 };

	static constexpr size_t SENTENCE_BUFFER_SIZE = 128;
	std::array<uint8_t, SENTENCE_BUFFER_SIZE> _tempSentenceBuffer{0};

	size_t _lastReadPosition { 0 };
	Coordinates _currentData { };
	bool findAndProcessSentence(const char *sentence, size_t length);
};
