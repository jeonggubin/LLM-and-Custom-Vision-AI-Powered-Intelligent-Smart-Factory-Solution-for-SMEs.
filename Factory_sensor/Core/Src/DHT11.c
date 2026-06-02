#include "DHT11.h"
#include "string.h"
#include "stdio.h"

extern UART_HandleTypeDef huart2;

extern TIM_HandleTypeDef htim1;

static void delay_us(uint32_t us)
{
    __HAL_TIM_SET_COUNTER(&htim1, 0);
    while (__HAL_TIM_GET_COUNTER(&htim1) < us);
}

static void DHT11_SetOutput(void)
{
    GPIO_InitTypeDef g = {0};
    g.Pin   = DHT11_PIN;
    g.Mode  = GPIO_MODE_OUTPUT_PP;
    g.Pull  = GPIO_NOPULL;
    g.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DHT11_PORT, &g);
}

static void DHT11_SetInput(void)
{
    GPIO_InitTypeDef g = {0};
    g.Pin  = DHT11_PIN;
    g.Mode = GPIO_MODE_INPUT;
    g.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(DHT11_PORT, &g);
}

static uint8_t DHT11_ReadByte(void)
{
	uint8_t data = 0;
	uint32_t t;
	for (int i = 0; i < 8; i++) {
		t = 100000; while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_RESET) if(--t==0) return data;
		delay_us(35);
		data <<= 1;
		if (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_SET)
			data |= 1;
		t = 100000; while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_SET) if(--t==0) return data;
	}
	return data;
}

void DHT11_Init(void)
{
    DHT11_SetOutput();
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET);
    HAL_Delay(2000);
}

DHT11_Data DHT11_Read(void)
{
    DHT11_Data result = {0};
    uint32_t t;
    char dbg[64];

    DHT11_SetOutput();
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_RESET);
    HAL_Delay(20);
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET);
    delay_us(7);

    DHT11_SetInput();

    t = 100000;
    while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_SET)
        if(--t==0) { HAL_UART_Transmit(&huart2, (uint8_t*)"ACK1 FAIL\r\n", 11, 100); return result; }
    HAL_UART_Transmit(&huart2, (uint8_t*)"ACK1 OK\r\n", 9, 100);

    t = 100000;
    while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_RESET)
        if(--t==0) { HAL_UART_Transmit(&huart2, (uint8_t*)"ACK2 FAIL\r\n", 11, 100); return result; }
    HAL_UART_Transmit(&huart2, (uint8_t*)"ACK2 OK\r\n", 9, 100);

    t = 100000;
    while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_SET)
        if(--t==0) { HAL_UART_Transmit(&huart2, (uint8_t*)"ACK3 FAIL\r\n", 11, 100); return result; }
    HAL_UART_Transmit(&huart2, (uint8_t*)"ACK3 OK\r\n", 9, 100);

    uint8_t hum    = DHT11_ReadByte();
    uint8_t hum_d  = DHT11_ReadByte();
    uint8_t temp   = DHT11_ReadByte();
    uint8_t temp_d = DHT11_ReadByte();
    uint8_t chksum = DHT11_ReadByte();

    snprintf(dbg, sizeof(dbg), "H:%d Hd:%d T:%d Td:%d CK:%d SUM:%d\r\n",
             hum, hum_d, temp, temp_d, chksum, (uint8_t)(hum+hum_d+temp+temp_d));
    HAL_UART_Transmit(&huart2, (uint8_t*)dbg, strlen(dbg), 100);

    if (chksum == (uint8_t)(hum + hum_d + temp + temp_d)) {
        result.humidity    = hum;
        result.temperature = temp;
        result.valid       = 1;
    }

    DHT11_SetOutput();
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET);
    return result;
}
