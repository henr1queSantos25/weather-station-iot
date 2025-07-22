#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "lwip/tcp.h"
#include <stdio.h>

//
#include "web.h"
#include "aht20.h"
#include "bmp280.h"
#include "ssd1306.h"
#include "button.h"

// Definições dos pinos
#define I2C_PORT_SENSORS i2c0
#define I2C_SDA_SENSORS 0
#define I2C_SCL_SENSORS 1
#define I2C_PORT_DISP i2c1
#define I2C_SDA_DISP 14
#define I2C_SCL_DISP 15
#define BUTTON_A 5
ssd1306_t ssd; // Estrutura do display
bool volatile modo_display = true;  // Modo de exibição
uint volatile last_time = 0; 


// Definições de Wi-Fi
#define WIFI_SSID "SSID_WIFI"
#define WIFI_PASS "PASSWORD_WIFI"

// Variáveis globais
float temperatura;
int32_t pressao;
float umidade;

int limiteMAX_temp = 35;
int limiteMAX_umi = 85;
int limiteMAX_pressao = 1013;
int limiteMIN_temp = 20;
int limiteMIN_umi = 30;
int limiteMIN_pressao = 950;
int offset_temp = 0;
int offset_umi = 0;
int offset_pressao = 0;

// Protótipos das funções
void setup();
void gpio_irq_handler(uint gpio, uint32_t events);
void info_display(int32_t temperatura_bmp, float temperatura_aht, int32_t pressao, float umidade, char *ip_str);

int main() {
    setup();

    char ip_str[16];
    if (inicializar_wifi(ip_str, WIFI_SSID, WIFI_PASS) != 0) {
        sleep_ms(5000); // Espera 5 segundos antes de encerrar para dar tempo de conectar ao serial monitor
        printf("Falha ao inicializar Wi-Fi. Encerrando...\n");
        return -1;
    }

    init_web();

    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    // Inicialização das estruturas de dados para os sensores
    struct bmp280_calib_param params;
    bmp280_get_calib_params(I2C_PORT_SENSORS, &params);

    AHT20_Data data;
    int32_t raw_temp_bmp;
    int32_t raw_pressure;

    while (1) {

        bmp280_read_raw(I2C_PORT_SENSORS, &raw_temp_bmp, &raw_pressure);
        int32_t temperature_bmp = bmp280_convert_temp(raw_temp_bmp, &params); 
        pressao = (bmp280_convert_pressure(raw_pressure, raw_temp_bmp, &params) / 1000) + offset_pressao; // kPa

        if (aht20_read(I2C_PORT_SENSORS, &data)) {
            temperatura = ((data.temperature + (temperature_bmp / 100.0)) / 2.0) + offset_temp; // Média das temperaturas
            umidade = data.humidity + offset_umi;
        }
        else {
            printf("Erro ao ler AHT20\n");
            temperatura = 0.0;
            umidade = 0.0;
        }

        // Exibe as informações no display
        info_display(temperature_bmp, temperatura, pressao, umidade, ip_str);

        cyw43_arch_poll();
        sleep_ms(200);
    }

    cyw43_arch_deinit();
    return 0;
}

void setup() {
    stdio_init_all();
    setup_I2C(I2C_PORT_DISP, I2C_SDA_DISP, I2C_SCL_DISP, 400 * 1000);
    setup_ssd1306(&ssd, I2C_PORT_DISP);
    setup_I2C_aht20(I2C_PORT_SENSORS, I2C_SDA_SENSORS, I2C_SCL_SENSORS, 400 * 1000);
    aht20_reset(I2C_PORT_SENSORS);
    aht20_init(I2C_PORT_SENSORS);
    bmp280_init(I2C_PORT_SENSORS);
    setup_button(BUTTON_A);
}

void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t time = to_us_since_boot(get_absolute_time());

    if (absolute_time_diff_us(last_time, time) > 500000){
        last_time = time; // Atualiza o tempo do último evento
        if (gpio == BUTTON_A) {
            modo_display = !modo_display; // Alterna o modo de exibição
        }
    }
}

void info_display(int32_t temperatura_bmp, float temperatura_aht, int32_t pressao, float umidade, char *ip_str) {

    bool cor = true;
    ssd1306_fill(&ssd, false); // Limpa o display

    ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor);       // Desenha um retângulo
    ssd1306_line(&ssd, 3, 25, 123, 25, cor);            // Desenha uma linha
    ssd1306_line(&ssd, 3, 37, 123, 37, cor);            // Desenha uma linha
    ssd1306_draw_string(&ssd, "CEPEDI   TIC37", 8, 6);  // Desenha uma string
    ssd1306_draw_string(&ssd, "EMBARCATECH", 20, 16);   // Desenha uma string

    if (!modo_display) {
        // Buffers para strings
        char str_tmp1[5];
        char str_tmp2[5];
        char str_pressao[5];
        char str_umi[5];

        sprintf(str_tmp1, "%.1fC", temperatura_bmp / 100.0);
        sprintf(str_tmp2, "%.1fC", temperatura_aht);
        sprintf(str_pressao, "%dkPa", pressao);
        sprintf(str_umi, "%.1f%%", umidade);

        ssd1306_draw_string(&ssd, "BMP280  AHT20", 10, 28); // Desenha uma string
        ssd1306_line(&ssd, 63, 25, 63, 60, cor);            // Desenha uma linha vertical
        ssd1306_draw_string(&ssd, str_tmp1, 14, 41);        // Desenha uma string
        ssd1306_draw_string(&ssd, str_pressao, 12, 52);     // Desenha uma string
        ssd1306_draw_string(&ssd, str_tmp2, 73, 41);        // Desenha uma string
        ssd1306_draw_string(&ssd, str_umi, 73, 52);         // Desenha uma string
    } else {

        char buffer[32]; // Buffer para armazenar as strings que serão desenhadas no display
        ssd1306_draw_string(&ssd, "WIFI - ON - IP", 7, 28);
        snprintf(buffer, sizeof(buffer), "%s", ip_str);
        ssd1306_draw_string(&ssd, buffer, 11, 45);

    }

    ssd1306_send_data(&ssd); // Atualiza o display com as alterações
}
