#include "rgb.h"
#include "hardware/pwm.h"

void setupLED(uint led) {
    gpio_init(led);              // Inicializa o pino do LED
    gpio_set_dir(led, GPIO_OUT); // Define o pino como saída
    gpio_put(led, 0);            // Desliga o LED inicialmente
}

void setup_pwm_led(uint led) {
    gpio_set_function(led, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(led);
    pwm_set_wrap(slice_num, 255);
    pwm_set_enabled(slice_num, true);
}

/* ==========================================================
   FUNÇÕES QUE MANIPULAM O LED
   ========================================================== */

//FUNÇÃO PARA PISCAR O LED
absolute_time_t proximo_piscar; // Variável para armazenar o tempo do próximo piscar do LED
bool led_aceso = false; // Variável para armazenar o estado do LED

void set_rgb(uint led_red, uint led_green, uint led_blue, bool r, bool g, bool b) {
    gpio_put(led_red, r);
    gpio_put(led_green, g);
    gpio_put(led_blue, b);
}

void piscar_led(uint led) {
    if (!time_reached(proximo_piscar))
        return;

    // Inverte o estado atual
    led_aceso = !led_aceso;
    gpio_put(led, led_aceso);

    // Define próximo momento de alternância (ex: a cada 500 ms)
    proximo_piscar = make_timeout_time_ms(500);
}

void piscar_dois_leds(uint led1, uint led2) {
    if (!time_reached(proximo_piscar))
        return;

    // Inverte o estado atual dos LEDs
    led_aceso = !led_aceso;
    gpio_put(led1, led_aceso);
    gpio_put(led2, led_aceso);

    // Define próximo momento de alternância (ex: a cada 500 ms)
    proximo_piscar = make_timeout_time_ms(500);
}

// FUNÇÃO PARA FAZER O LED FADE
int brilho = 0; // Brilho do LED (0-255)
bool subindo = true; // Variável para controlar a direção do fade
absolute_time_t proximo_fade; // Variável para armazenar o tempo do próximo fade

void atualizar_fade_led(uint led) {
    if (!time_reached(proximo_fade)) return;

    pwm_set_gpio_level(led, brilho);

    // Atualiza valor do brilho
    if (subindo) {
        brilho += 5;
        if (brilho >= 255) {
            brilho = 255;
            subindo = false;
        }
    } else {
        brilho -= 5;
        if (brilho <= 0) {
            brilho = 0;
            subindo = true;
        }
    }

    // Define próximo momento do fade
    proximo_fade = make_timeout_time_ms(15); // controla a suavidade da transição
}