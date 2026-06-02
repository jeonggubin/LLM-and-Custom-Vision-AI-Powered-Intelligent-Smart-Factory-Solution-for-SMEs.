#ifndef INC_DHT11_H_
#define INC_DHT11_H_
#include "main.h"

#define DHT11_PORT GPIOA
#define DHT11_PIN  GPIO_PIN_0  // PA0

typedef struct {
    uint8_t temperature;
    uint8_t humidity;
    uint8_t valid;
} DHT11_Data;

void DHT11_Init(void);
DHT11_Data DHT11_Read(void);

#endif
