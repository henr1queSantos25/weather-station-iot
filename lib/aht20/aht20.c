#include "aht20.h"

#define AHT20_I2C_ADDR      0x38
#define AHT20_CMD_INIT      0xBE
#define AHT20_CMD_TRIGGER   0xAC
#define AHT20_CMD_RESET     0xBA
#define AHT20_STATUS_BUSY   0x80  // Bit de status ocupado
#define AHT20_STATUS_CALIBRATED 0x08  // Bit de calibração

void setup_I2C_aht20(i2c_inst_t *I2C_PORT, uint I2C_SDA, uint I2C_SCL, uint clock) {
    // INICIALIZAÇÃO DO I2C COM A FREQUÊNCIA DE CLOCK
    i2c_init(I2C_PORT, clock);
    // CONFIGURAÇÃO DO I2C
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Define a função do pino GPIO para I2C.
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Define a função do pino GPIO para I2C.
    gpio_pull_up(I2C_SDA); // Configura pull-up para a linha de dados
    gpio_pull_up(I2C_SCL); // Configura pull-up para a linha de clock
}

bool aht20_init(i2c_inst_t *i2c) {
    uint8_t init_cmd[3] = {AHT20_CMD_INIT, 0x08, 0x00};
    i2c_write_blocking(i2c, AHT20_I2C_ADDR, init_cmd, 3, false);
    sleep_ms(50);  // Aguarda o sensor inicializar

    // Verifica status até que o sensor esteja pronto
    uint8_t status;
    for (int i = 0; i < 10; i++) {
        i2c_read_blocking(i2c, AHT20_I2C_ADDR, &status, 1, false);
        if ((status & AHT20_STATUS_CALIBRATED) == AHT20_STATUS_CALIBRATED) {
            return true;  // Sensor calibrado e pronto
        }
        sleep_ms(10);
    }

    return false;  // Falhou na calibração
}

bool aht20_read(i2c_inst_t *i2c, AHT20_Data *data) {
    uint8_t trigger_cmd[3] = {AHT20_CMD_TRIGGER, 0x33, 0x00};
    uint8_t buffer[6];

    // Envia comando de medição
    i2c_write_blocking(i2c, AHT20_I2C_ADDR, trigger_cmd, 3, false);
    
    // Aguarda até o sensor estar pronto
    uint8_t status;
    for (int i = 0; i < 10; i++) {
        i2c_read_blocking(i2c, AHT20_I2C_ADDR, &status, 1, false);
        if (!(status & AHT20_STATUS_BUSY)) {
            break;
        }
        sleep_ms(10);
    }
    
    // Se ainda estiver ocupado, falha na leitura
    if (status & AHT20_STATUS_BUSY) {
        return false;
    }

    // Lê os 6 bytes de dados
    if (i2c_read_blocking(i2c, AHT20_I2C_ADDR, buffer, 6, false) != 6) {
        return false;
    }

    // Processa os dados de umidade (20 bits)
    uint32_t raw_humidity = ((uint32_t)buffer[1] << 12) | ((uint32_t)buffer[2] << 4) | (buffer[3] >> 4);
    data->humidity = (float)raw_humidity * 100.0 / 1048576.0;

    // Processa os dados de temperatura (20 bits)
    uint32_t raw_temp = ((uint32_t)(buffer[3] & 0x0F) << 16) | ((uint32_t)buffer[4] << 8) | buffer[5];
    data->temperature = ((float)raw_temp * 200.0 / 1048576.0) - 50.0;

    return true;
}

void aht20_reset(i2c_inst_t *i2c) {
    uint8_t reset_cmd = AHT20_CMD_RESET;
    i2c_write_blocking(i2c, AHT20_I2C_ADDR, &reset_cmd, 1, false);
    sleep_ms(20);
    aht20_init(i2c);
}

bool aht20_check(i2c_inst_t *i2c) {
    uint8_t status;
    return i2c_read_blocking(i2c, AHT20_I2C_ADDR, &status, 1, false) == 1;
}
