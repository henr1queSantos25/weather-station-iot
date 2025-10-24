#include "bh1750.h"

const uint8_t _POWER_ON_C = 0x01;   // Power on command
const uint8_t _CONT_HRES_C = 0x10;  // Modo de alta resolução (1 lux)
const uint8_t _CONT_HRES2_C = 0x11; // Modo de alta resolução 2 (0.5 lux)
const uint8_t _CONT_LRES_C = 0x13;  // Modo de baixa resolução (4 lux)

// Instância do barramento I2C
static i2c_inst_t *i2c_port;


// Função para configurar o I2C do BH1750
void setup_I2C_bh1750(i2c_inst_t *I2C_PORT, uint I2C_SDA, uint I2C_SCL, uint clock) {
    i2c_init(I2C_PORT, clock);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
}

void _i2c_write_byte(i2c_inst_t* i2c, uint8_t byte) {
    i2c_write_blocking(i2c, BH1750_I2C_ADDR, &byte, 1, false);
}

void bh1750_power_on(i2c_inst_t* I2C_PORT) {
    i2c_port = I2C_PORT;
    _i2c_write_byte(i2c_port, _POWER_ON_C);
}

uint16_t bh1750_read_measurement() {
    // Send "Continuously H-resolution mode" instruction
    _i2c_write_byte(i2c_port, _CONT_HRES_C);

    // Wait at least 180 ms to complete measurement
    sleep_ms(200);

    uint8_t buff[2];

    i2c_read_blocking(i2c_port, BH1750_I2C_ADDR, buff, 2, false);

    return (((uint16_t)buff[0] << 8) | buff[1]) / 1.2;
    // Obs. quando utilizar _CONT_HRES2_C dividir por 2.4
    // Quando utilizar _CONT_HRES_C dividir por 1.2
}