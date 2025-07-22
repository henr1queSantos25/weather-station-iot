#include "pico/stdlib.h"


void setupLED(uint led);
void set_rgb(uint led_red, uint led_green, uint led_blue, bool r, bool g, bool b);
void setup_pwm_led(uint led);
void piscar_led(uint led);
void piscar_dois_leds(uint led1, uint led2);
void atualizar_fade_led(uint led);  