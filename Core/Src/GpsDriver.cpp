#include "GpsDriver.hpp"
#include <cstring>
#include <stdlib.h>

void GpsDriver::init()
{
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA2);

    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_9, LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_10, LL_GPIO_MODE_ALTERNATE);

    LL_GPIO_SetPinOutputType(GPIOA, LL_GPIO_PIN_9 | LL_GPIO_PIN_10, LL_GPIO_OUTPUT_PUSHPULL);

    LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_9, LL_GPIO_PULL_UP);
    LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_10, LL_GPIO_PULL_UP);

    LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_9, LL_GPIO_SPEED_FREQ_VERY_HIGH);
	LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_10, LL_GPIO_SPEED_FREQ_VERY_HIGH);

    LL_GPIO_SetAFPin_8_15(GPIOA, LL_GPIO_PIN_9, LL_GPIO_AF_7);
    LL_GPIO_SetAFPin_8_15(GPIOA, LL_GPIO_PIN_10, LL_GPIO_AF_7);

	LL_DMA_SetChannelSelection(DMA2, LL_DMA_STREAM_2, LL_DMA_CHANNEL_4);

	LL_DMA_SetStreamPriorityLevel(DMA2, LL_DMA_STREAM_2, LL_DMA_PRIORITY_MEDIUM);

	LL_DMA_SetDataTransferDirection(DMA2, LL_DMA_STREAM_2, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
	LL_DMA_SetMode(DMA2, LL_DMA_STREAM_2, LL_DMA_MODE_CIRCULAR);

	LL_DMA_SetPeriphIncMode(DMA2, LL_DMA_STREAM_2, LL_DMA_PERIPH_NOINCREMENT);
	LL_DMA_SetMemoryIncMode(DMA2, LL_DMA_STREAM_2, LL_DMA_MEMORY_INCREMENT);

	LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_2, BUFFER_SIZE);
	LL_DMA_SetPeriphSize(DMA2, LL_DMA_STREAM_2, LL_DMA_PDATAALIGN_BYTE);
	LL_DMA_SetMemorySize(DMA2, LL_DMA_STREAM_2, LL_DMA_MDATAALIGN_BYTE);

	LL_DMA_SetMemoryAddress(DMA2, LL_DMA_STREAM_2, (uint32_t)buffer.data());
	LL_DMA_SetPeriphAddress(DMA2, LL_DMA_STREAM_2, LL_USART_DMA_GetRegAddr(USART1));

	LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_2);

    LL_USART_SetBaudRate(USART1, SystemCoreClock, LL_USART_OVERSAMPLING_16, 9600);
    LL_USART_SetDataWidth(USART1, LL_USART_DATAWIDTH_8B);
    LL_USART_SetParity(USART1, LL_USART_PARITY_NONE);
    LL_USART_SetStopBitsLength(USART1, LL_USART_STOPBITS_1);
    LL_USART_SetTransferDirection(USART1, LL_USART_DIRECTION_TX_RX);

	LL_USART_EnableDMAReq_RX(USART1);
	LL_USART_Enable(USART1);
}

bool GpsDriver::parse()
{
	uint32_t bytes_remaining{LL_DMA_GetDataLength(DMA2, LL_DMA_STREAM_2)};
	size_t currPosition{BUFFER_SIZE - bytes_remaining};
	size_t newDataLength{ 0 };

	if (currPosition >= this->_lastReadPosition)
		newDataLength = currPosition - this->_lastReadPosition;
	else
		newDataLength = BUFFER_SIZE - this->_lastReadPosition + currPosition;

	if (newDataLength == 0)
	{
		this->_lastReadPosition = currPosition;
		return false;
	}

    size_t chunk1Len{BUFFER_SIZE - _lastReadPosition};
    size_t bytesToCopyFromChunk1 = std::min(newDataLength, chunk1Len);

    std::memcpy(this->_tempSentenceBuffer.data(), buffer.data() + this->_lastReadPosition, bytesToCopyFromChunk1);

    if (newDataLength > chunk1Len)
        std::memcpy(this->_tempSentenceBuffer.data() + chunk1Len, buffer.data(), newDataLength - chunk1Len);

    this->_tempSentenceBuffer.data()[newDataLength] = '\0';

    char *start_sentence = strstr(reinterpret_cast<char*>(this->_tempSentenceBuffer.data()), "$GPRMC");
    if (start_sentence != nullptr)
	{
		char *save_pointer = nullptr;

		char *token = strtok_r(start_sentence, ",", &save_pointer);

		token = strtok_r(NULL, ",", &save_pointer);
		if (token != nullptr)
		{
			long raw_time = std::strtol(token, nullptr, 10);
			this->_currentData.time_sec = static_cast<uint32_t>(raw_time / 10000) * 3600;
			this->_currentData.time_sec += static_cast<uint32_t>((raw_time % 10000) / 100) * 60;
			this->_currentData.time_sec += static_cast<uint32_t>(raw_time % 100);
		}

		token = strtok_r(NULL, ",", &save_pointer);

		token = strtok_r(NULL, ",", &save_pointer);
		if (token != nullptr)
		{
			float raw_lat = std::strtof(token, nullptr);
			int degrees = static_cast<int>(raw_lat / 100);
			float minutes = raw_lat - (degrees * 100);

			this->_currentData.latitude = degrees + (minutes / 60.0f);
		}

		token = strtok_r(NULL, ",", &save_pointer);
		if (token != nullptr && *token == 'S')
			this->_currentData.latitude *= -1.0f;

		token = strtok_r(NULL, ",", &save_pointer);
		if (token != nullptr)
		{
			float raw_lon = std::strtof(token, nullptr);
			int degrees = static_cast<int>(raw_lon / 100);
			float minutes = raw_lon - (degrees * 100);

			this->_currentData.longitude = degrees + (minutes / 60.0f);
		}

		token = strtok_r(NULL, ",", &save_pointer);
		if (token != nullptr && *token == 'W')
			this->_currentData.longitude *= -1.0f;

	}

	this->_lastReadPosition = currPosition;
	return true;
}
