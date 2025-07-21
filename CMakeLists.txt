cmake_minimum_required(VERSION 3.13)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PICO_BOARD pico_w CACHE STRING "Board type")
include(pico_sdk_import.cmake)
project(weather-station-iot C CXX ASM)
pico_sdk_init()

include_directories(${CMAKE_SOURCE_DIR}/lib)

add_executable(${PROJECT_NAME}  
        weather-station-iot.c
        lib/web/web.c
        lib/aht20/aht20.c
        lib/bmp280/bmp280.c
        lib/ssd1306/ssd1306.c
        lib/push_button/button.c
        )

#pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/lib/matriz/matriz.pio)

target_link_libraries(${PROJECT_NAME} 
        pico_stdlib       
        hardware_i2c
        pico_cyw43_arch_lwip_threadsafe_background
        )

target_include_directories(${PROJECT_NAME}  PRIVATE   ${CMAKE_CURRENT_LIST_DIR} 
        lib/web
        lib/aht20
        lib/bmp280
        lib/ssd1306
        lib/push_button
        )

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 1)


pico_add_extra_outputs(${PROJECT_NAME})