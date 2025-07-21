#include "pico/stdlib.h"
#include "button.h"

// Variáveis estáticas para controle de debounce
static uint32_t last_press_time = 0;
static bool last_button_state = true; // true = não pressionado (pull-up)

void setup_button(uint gpio) {
    gpio_init(gpio);
    gpio_set_dir(gpio, GPIO_IN);
    gpio_pull_up(gpio);
}

bool debounce_button(uint gpio) {
    bool current_state = gpio_get(gpio);
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    // Verifica se houve mudança de estado (de não pressionado para pressionado)
    if (last_button_state && !current_state) {
        // Verifica se passou tempo suficiente desde a última pressão (debounce)
        if (current_time - last_press_time > 500) { // 200ms de debounce
            last_press_time = current_time;
            last_button_state = current_state;
            return true; // Botão foi pressionado
        }
    }
    
    // Atualiza o estado anterior
    last_button_state = current_state;
    return false; // Botão não foi pressionado ou está em debounce
}