#ifndef WEB_H
#define WEB_H

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "lwip/tcp.h"
#include "lwip/pbuf.h" 
#include "lwip/err.h" 

// Estrutura para manter o estado da conexão HTTP
struct http_state {
    char response[16384];
    size_t len;
    size_t sent;
};

// --- ASSINATURA DAS FUNÇÕES ---
static void start_http_server(void);
void init_web(void);
static err_t http_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
static err_t http_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static err_t connection_callback(void *arg, struct tcp_pcb *newpcb, err_t err);
int inicializar_wifi(char *ip_str, char *WIFI_SSID, char *WIFI_PASS);


#endif // WEB_H
