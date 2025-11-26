#include "Adxl345.hpp"

#define ADXL345_ADDR 0x53

#define POWER_CTL 0x2D
#define DATA_FORMAT 0x31

#define SDO LL_GPIO_PIN_4
#define CS LL_GPIO_PIN_5
#define SCL_PIN LL_GPIO_PIN_6
#define SDA_PIN LL_GPIO_PIN_7

bool Adxl345::configure(void)
{
	if (!I2C_Write(DATA_FORMAT, 0x08))
		return false;

	if (!I2C_Write(POWER_CTL, 0x08))
		return false;
}

void Adxl345::init(void)
{
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);

	LL_GPIO_SetPinMode(GPIOB, SCL_PIN, LL_GPIO_MODE_ALTERNATE);
	LL_GPIO_SetPinPull(GPIOB, SCL_PIN, LL_GPIO_PULL_UP);
	LL_GPIO_SetPinSpeed(GPIOB, SCL_PIN, LL_GPIO_SPEED_FREQ_VERY_HIGH);
	LL_GPIO_SetAFPin_0_7(GPIOB, SCL_PIN, LL_GPIO_AF_4);

	LL_GPIO_SetPinMode(GPIOB, SDA_PIN, LL_GPIO_MODE_ALTERNATE);
	LL_GPIO_SetPinPull(GPIOB, SDA_PIN, LL_GPIO_PULL_UP);
	LL_GPIO_SetPinSpeed(GPIOB, SDA_PIN, LL_GPIO_SPEED_FREQ_VERY_HIGH);
	LL_GPIO_SetAFPin_0_7(GPIOB, SDA_PIN, LL_GPIO_AF_4);

	LL_GPIO_SetPinOutputType(GPIOB, SCL_PIN | SDA_PIN, LL_GPIO_OUTPUT_OPENDRAIN);
	LL_GPIO_SetPinOutputType(GPIOB, SDO | CS, LL_GPIO_OUTPUT_PUSHPULL);

	LL_GPIO_SetPinMode(GPIOB, SDO, LL_GPIO_MODE_OUTPUT);
	LL_GPIO_SetPinSpeed(GPIOB, SDO, LL_GPIO_SPEED_FREQ_LOW);
	LL_GPIO_ResetOutputPin(GPIOB, SDO);

	LL_GPIO_SetPinMode(GPIOB, CS, LL_GPIO_MODE_OUTPUT);
	LL_GPIO_SetPinSpeed(GPIOB, CS, LL_GPIO_SPEED_FREQ_LOW);
	LL_GPIO_SetOutputPin(GPIOB, CS);

	LL_I2C_InitTypeDef I2C_InitStruct = {
			.PeripheralMode = LL_I2C_MODE_I2C,
			.ClockSpeed = 100000,
			.DutyCycle = LL_I2C_DUTYCYCLE_2,
			.AnalogFilter = 0,
			.DigitalFilter = 0,
			.OwnAddress1 = 0,
			.TypeAcknowledge = LL_I2C_ACK,
			.OwnAddrSize = LL_I2C_OWNADDRESS1_7BIT
	};
	LL_I2C_Init(I2C1, &I2C_InitStruct);

	LL_I2C_EnableIT_EVT(I2C1);
	LL_I2C_EnableIT_ERR(I2C1);
	LL_I2C_EnableIT_BUF(I2C1);

	NVIC_SetPriority(I2C1_EV_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
	NVIC_EnableIRQ(I2C1_EV_IRQn);

	NVIC_SetPriority(I2C1_ER_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 1));
	NVIC_EnableIRQ(I2C1_ER_IRQn);

	LL_I2C_Enable(I2C1);
}

bool Adxl345::I2C_Write(uint8_t reg_addr, uint8_t data)
{
    uint32_t timeout = 10000;

    while (LL_I2C_IsActiveFlag_BUSY(I2C1))
        if ((--timeout) == 0) return false;
    LL_I2C_GenerateStartCondition(I2C1);

    timeout = 10000;
    while (!LL_I2C_IsActiveFlag_SB(I2C1))
        if ((--timeout) == 0) return false;
    LL_I2C_TransmitData8(I2C1, ADXL345_ADDR << 1);

    timeout = 10000;
    while (!LL_I2C_IsActiveFlag_ADDR(I2C1))
    {
        if ((--timeout) == 0) return false;
        if (LL_I2C_IsActiveFlag_AF(I2C1)) { return false; }
    }
    LL_I2C_ClearFlag_ADDR(I2C1);

    timeout = 10000;
    while (!LL_I2C_IsActiveFlag_TXE(I2C1))
        if ((--timeout) == 0) return false;
    LL_I2C_TransmitData8(I2C1, reg_addr);

    timeout = 10000;
    while (!LL_I2C_IsActiveFlag_TXE(I2C1))
        if ((--timeout) == 0) return false;
    LL_I2C_TransmitData8(I2C1, data);

    timeout = 10000;
    while (!LL_I2C_IsActiveFlag_BTF(I2C1))
        if ((--timeout) == 0) return false;

    LL_I2C_GenerateStopCondition(I2C1);

    return true;
}

bool Adxl345::I2C_Read(uint8_t reg_addr, uint8_t *buffer)
{
	uint32_t timeout = 10000;

	while(LL_I2C_IsActiveFlag_BUSY(I2C1))
		if (!(--timeout)) return false;

	LL_I2C_GenerateStartCondition(I2C1);
	timeout = 10000;
	while(!LL_I2C_IsActiveFlag_SB(I2C1))
		if (!(--timeout)) return false;

	LL_I2C_TransmitData8(I2C1, ADXL345_ADDR);
	timeout = 10000;
	while(!LL_I2C_IsActiveFlag_ADDR(I2C1))
		if (!(--timeout)) return false;

	LL_I2C_ClearFlag_ADDR(I2C1);

	LL_I2C_TransmitData8(I2C1, reg_addr);
	timeout = 10000;
	while(!LL_I2C_IsActiveFlag_TXE(I2C1))
		if (!(--timeout)) return false;

	LL_I2C_GenerateStartCondition(I2C1);
	timeout = 10000;
	while(!(LL_I2C_IsActiveFlag_SB(I2C1)))
		if (!(--timeout)) return false;

	LL_I2C_TransmitData8(I2C1, ADXL345_ADDR | 1);
	timeout = 10000;
	while(!(LL_I2C_IsActiveFlag_ADDR(I2C1)))
		if (!(--timeout)) return false;

	LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_NACK);
	LL_I2C_ClearFlag_ADDR(I2C1);
	LL_I2C_GenerateStopCondition(I2C1);

	timeout = 10000;
	while(!(LL_I2C_IsActiveFlag_RXNE(I2C1)))
		if (!(--timeout)) return false;

	*buffer = LL_I2C_ReceiveData8(I2C1);

	return true;
}

bool Adxl345::I2C_ReadBuffer(uint8_t reg_addr, uint8_t *buffer, uint16_t length)
{
    // 1. Окремий випадок для 1 байта
    if (length == 1) return I2C_Read(reg_addr, buffer);

    uint32_t timeout = 10000;

    // --- ФАЗА 1: Вибір регістра ---
    while(LL_I2C_IsActiveFlag_BUSY(I2C1)) if (!(--timeout)) return false;

    LL_I2C_GenerateStartCondition(I2C1);
    timeout = 10000;
    while(!LL_I2C_IsActiveFlag_SB(I2C1)) if (!(--timeout)) return false;

    LL_I2C_TransmitData8(I2C1, ADXL345_ADDR);
    timeout = 10000;
    while(!LL_I2C_IsActiveFlag_ADDR(I2C1)) if (!(--timeout)) return false;
    LL_I2C_ClearFlag_ADDR(I2C1);

    LL_I2C_TransmitData8(I2C1, reg_addr);
    timeout = 10000;
    while(!LL_I2C_IsActiveFlag_TXE(I2C1)) if (!(--timeout)) return false;

    // --- ФАЗА 2: Читання масиву ---
    LL_I2C_GenerateStartCondition(I2C1);
    timeout = 10000;
    while(!LL_I2C_IsActiveFlag_SB(I2C1)) if (!(--timeout)) return false;

    LL_I2C_TransmitData8(I2C1, ADXL345_ADDR | 1);
    timeout = 10000;
    while(!LL_I2C_IsActiveFlag_ADDR(I2C1)) if (!(--timeout)) return false;

    // Вмикаємо ACK, бо ми плануємо читати багато байтів
    LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_ACK);
    LL_I2C_ClearFlag_ADDR(I2C1);

    // Читаємо всі байти, КРІМ ОСТАННЬОГО
    while (length > 1)
    {
        timeout = 10000;
        while (!LL_I2C_IsActiveFlag_RXNE(I2C1)) if (!(--timeout)) return false;

        *buffer = LL_I2C_ReceiveData8(I2C1);
        buffer++;
        length--;
    }

    // --- ФАЗА 3: Останній байт (NACK + STOP) ---
    // Ми вийшли з циклу, коли length == 1. Тобто залишився один байт в дорозі.

    // 1. Готуємо NACK (щоб сказати "досить")
    LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_NACK);

    // 2. Готуємо STOP
    LL_I2C_GenerateStopCondition(I2C1);

    // 3. Чекаємо останній байт
    timeout = 10000;
    while (!LL_I2C_IsActiveFlag_RXNE(I2C1)) if (!(--timeout)) return false;

    // 4. Читаємо останній байт
    *buffer = LL_I2C_ReceiveData8(I2C1);

    return true; // <--- НЕ ЗАБУВАЙТЕ ЦЕ!
}
