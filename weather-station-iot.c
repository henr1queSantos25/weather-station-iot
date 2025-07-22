#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "lwip/tcp.h"
#include <stdio.h>
#include <limits.h>

// === BIBLIOTECAS DA PASTA LIB ===
#include "web.h"
#include "aht20.h"
#include "bmp280.h"
#include "ssd1306.h"
#include "button.h"
#include "rgb.h"
#include "ws2812.h"
#include "buzzer.h"

// === DEFINIÇÕES DE PINOS E CONSTANTES ===
#define I2C_PORT_SENSORS i2c0
#define I2C_SDA_SENSORS 0
#define I2C_SCL_SENSORS 1
#define I2C_PORT_DISP i2c1
#define I2C_SDA_DISP 14
#define I2C_SCL_DISP 15
#define BUTTON_A 5
#define LED_GREEN 11
#define LED_BLUE 12
#define LED_RED 13
#define BUZZER 10
ssd1306_t ssd; // Estrutura do display
bool volatile modo_display = true;  // Modo de exibição
uint volatile last_time = 0; // Debounce

// === CONFIGURAÇÕES DE WIFI ===
#define WIFI_SSID "SSID_WIFI"
#define WIFI_PASS "PASSWORD_WIFI"

// === DADOS DOS SENSORES ===
float temperatura;
int32_t pressao;
float umidade;

// === LIMITES DAS MEDIDAS ===
int limiteMAX_temp = INT_MAX;
int limiteMAX_umi = INT_MAX;
int limiteMAX_pressao = INT_MAX;
int limiteMIN_temp = INT_MIN;
int limiteMIN_umi = INT_MIN;
int limiteMIN_pressao = INT_MIN;

// === OFFSETS DE CALIBRAÇÃO ===
int offset_temp = 0;
int offset_umi = 0;
int offset_pressao = 0;


// ========================================================================
// PROTÓTIPOS DAS FUNÇÕES
// ========================================================================
void setup();
void gpio_irq_handler(uint gpio, uint32_t events);
void info_display(int32_t temperatura_bmp, float temperatura_aht, int32_t pressao, float umidade, char *ip_str);
void verificar_limites();

// ========================================================================
// FUNÇÃO PRINCIPAL
// ========================================================================
int main() {

    // Inicialização do Wi-Fi e do sistema
    setup();

    char ip_str[16];
    if (inicializar_wifi(ip_str, WIFI_SSID, WIFI_PASS) != 0) {
        sleep_ms(5000); // Espera 5 segundos antes de encerrar para dar tempo de conectar ao serial monitor
        printf("Falha ao inicializar Wi-Fi. Encerrando...\n");
        return -1;
    }

    init_web();

    // Configuração da interrupção do botão
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    // Inicialização das estruturas de dados para os sensores
    struct bmp280_calib_param params;
    bmp280_get_calib_params(I2C_PORT_SENSORS, &params);

    AHT20_Data data;
    int32_t raw_temp_bmp;
    int32_t raw_pressure;

    while (1) {
        // === LEITURA DO SENSOR BMP280 ===
        bmp280_read_raw(I2C_PORT_SENSORS, &raw_temp_bmp, &raw_pressure);
        int32_t temperature_bmp = bmp280_convert_temp(raw_temp_bmp, &params); 
        pressao = (bmp280_convert_pressure(raw_pressure, raw_temp_bmp, &params) / 1000) + offset_pressao; // kPa

        // === LEITURA DO SENSOR AHT20 ===
        if (aht20_read(I2C_PORT_SENSORS, &data)) {
            temperatura = ((data.temperature + (temperature_bmp / 100.0)) / 2.0) + offset_temp; // Média das temperaturas
            umidade = (data.humidity + offset_umi) > 100 ? 100 : (data.humidity + offset_umi); // Limita a umidade a 100%
        }
        else {
            printf("Erro ao ler AHT20\n");
            temperatura = 0.0;
            umidade = 0.0;
        }

        // === VERIFICAÇÃO DE LIMITES E ALARMES ===
        verificar_limites(); 
        
        // === ATUALIZAÇÃO DO DISPLAY ===
        info_display(temperature_bmp, temperatura, pressao, umidade, ip_str);

        // === ATUALIZAÇÃO DO SERVIDOR WEB ===
        cyw43_arch_poll();
        sleep_ms(200);
    }

    cyw43_arch_deinit();
    return 0;
}


// ========================================================================
// FUNÇÕES DE INICIALIZAÇÃO
// ========================================================================

/**
 * @brief Inicializa todos os periféricos e sensores do sistema
 */
void setup() {
    stdio_init_all();

    // === CONFIGURAÇÃO DO DISPLAY ===
    setup_I2C(I2C_PORT_DISP, I2C_SDA_DISP, I2C_SCL_DISP, 400 * 1000);
    setup_ssd1306(&ssd, I2C_PORT_DISP);

    // === CONFIGURAÇÃO DOS SENSORES ===
    setup_I2C_aht20(I2C_PORT_SENSORS, I2C_SDA_SENSORS, I2C_SCL_SENSORS, 400 * 1000);
    aht20_reset(I2C_PORT_SENSORS);
    aht20_init(I2C_PORT_SENSORS);
    bmp280_init(I2C_PORT_SENSORS);

    // === CONFIGURAÇÃO DOS PERIFÉRICOS ===
    setup_button(BUTTON_A);
    setupLED(LED_GREEN);
    setupLED(LED_RED);
    setupLED(LED_BLUE);
    setup_PIO();
    init_pwm_buzzer(BUZZER);
}

// ========================================================================
// FUNÇÕES DE CONTROLE
// ========================================================================

/**
 * @brief Handler de interrupção para o botão A
 * @param gpio Pino do GPIO que gerou a interrupção
 * @param events Eventos que causaram a interrupção
 */
void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t time = to_us_since_boot(get_absolute_time());

    if (absolute_time_diff_us(last_time, time) > 500000){
        last_time = time; // Atualiza o tempo do último evento
        if (gpio == BUTTON_A) {
            modo_display = !modo_display; // Alterna o modo de exibição
        }
    }
}

// ========================================================================
// FUNÇÕES DE INTERFACE
// ========================================================================

/**
 * @brief Atualiza o display OLED com informações dos sensores ou WiFi
 * @param temperatura_bmp Temperatura do BMP280 (em centésimos de grau)
 * @param temperatura_aht Temperatura média calculada
 * @param pressao Pressão atmosférica em kPa
 * @param umidade Umidade relativa em %
 * @param ip_str String com o endereço IP
 */
void info_display(int32_t temperatura_bmp, float temperatura_aht, int32_t pressao, float umidade, char *ip_str) {
    bool cor = true;
    
    // === LIMPEZA E LAYOUT BÁSICO ===
    ssd1306_fill(&ssd, false); // Limpa o display
    ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor);       // Desenha um retângulo
    ssd1306_line(&ssd, 3, 25, 123, 25, cor);            // Desenha uma linha
    ssd1306_line(&ssd, 3, 37, 123, 37, cor);            // Desenha uma linha
    ssd1306_draw_string(&ssd, "CEPEDI   TIC37", 8, 6); 
    ssd1306_draw_string(&ssd, "EMBARCATECH", 20, 16);   

    if (!modo_display) {
        // === MODO SENSORES ===
        char str_tmp1[5], str_tmp2[5], str_pressao[5], str_umi[5];

        // Formatação dos valores
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
        // === MODO Wi-Fi ===
        char buffer[32]; // Buffer para armazenar as strings que serão desenhadas no display
        ssd1306_draw_string(&ssd, "WIFI - ON - IP", 7, 28);
        snprintf(buffer, sizeof(buffer), "%s", ip_str);
        ssd1306_draw_string(&ssd, buffer, 11, 45);

    }

    ssd1306_send_data(&ssd); // Atualiza o display com as alterações
}

// ========================================================================
// FUNÇÕES DE MONITORAMENTO
// ========================================================================

/**
 * @brief Verifica se os valores dos sensores estão dentro dos limites
 *        e aciona alarmes visuais e sonoros conforme necessário
 */
void verificar_limites() {
    // === VERIFICAÇÃO DE LIMITES MÁXIMOS ===
    if (temperatura > limiteMAX_temp || umidade > limiteMAX_umi || pressao > limiteMAX_pressao) {
        // Alarme crítico: valores acima do limite máximo
        alarmePWM(BUZZER);
        desenhoX_vermelho();
        gpio_put(LED_GREEN, false); 
        gpio_put(LED_BLUE, false); 
        piscar_led(LED_RED); 
    } 
    // === VERIFICAÇÃO DE LIMITES MÍNIMOS ===
    else if (temperatura < limiteMIN_temp || umidade < limiteMIN_umi || pressao < limiteMIN_pressao) {
        alarmePWM(BUZZER);
        desenhoX_amarelo();
        gpio_put(LED_BLUE, false); // Desliga o LED azul
        piscar_dois_leds(LED_RED, LED_GREEN); // Pisca os LEDs vermelho e verde
    }
    // === VALORES NORMAIS === 
    else {
        buzzer_pwm_off(BUZZER); // Desliga o buzzer
        apagarMatriz();
        gpio_put(LED_RED, false); // Desliga o LED vermelho
        gpio_put(LED_BLUE, false); // Desliga o LED azul
        piscar_led(LED_GREEN); // Pisca o LED verde
    }
}