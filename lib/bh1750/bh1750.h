#ifndef BH1750_H
#define BH1750_H

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"

// Endereço do BH1750 no barramento I2C
#define BH1750_I2C_ADDR 0x23 

// === ASSINATURA DAS FUNÇÕES ===
void setup_I2C_bh1750(i2c_inst_t *I2C_PORT, uint I2C_SDA, uint I2C_SCL, uint clock);
void bh1750_power_on(i2c_inst_t* i2c);
void _i2c_write_byte(i2c_inst_t* i2c, uint8_t byte); 
uint16_t bh1750_read_measurement();

#endif