#include "web.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// Variáveis externas (para acesso das variáveis do arquivo weather-station-iot.c)
extern float temperatura;
extern int32_t pressao;
extern float umidade;

int inicializar_wifi(char *ip_str ,char *WIFI_SSID, char *WIFI_PASS) {
    // Inicialização e configuração do Wi-Fi
    if (cyw43_arch_init()) {
        printf("WiFi => FALHA\n");
        sleep_ms(100);
        return -1;
    }

    cyw43_arch_enable_sta_mode();

    // Conexão à rede Wi-Fi
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        printf("WiFi => ERRO\n");
        sleep_ms(100);
        return -1;
    }

    // Exibe o endereço IP atribuído
    uint8_t *ip = (uint8_t *)&(cyw43_state.netif[0].ip_addr.addr);
    snprintf(ip_str, 16, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);

    printf("WiFi => Conectado com sucesso!\n IP: %s\n", ip_str);

    return 0; // Retorna 0 para indicar sucesso
}


// HTML da interface web - Página completa com estilos CSS e JavaScript

const char HTML_BODY[] =
"<!DOCTYPE html>"
"<html lang='pt-BR'>"
"<head>"
"<meta charset='UTF-8'>"
"<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
"<title>Weather Station IoT</title>"
"<script src='https://cdn.jsdelivr.net/npm/chart.js'></script>"
"<style>"
"*{margin:0;padding:0;box-sizing:border-box;}"
"body{font-family:'Segoe UI',Tahoma,Geneva,Verdana,sans-serif;background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);color:#333;min-height:100vh;}"
".container{max-width:1200px;margin:0 auto;padding:20px;}"
".header{text-align:center;margin-bottom:30px;color:white;}"
".header h1{font-size:2.5rem;margin-bottom:10px;text-shadow:2px 2px 4px rgba(0,0,0,0.3);}"
".header p{font-size:1.1rem;opacity:0.9;}"
".dashboard{display:grid;grid-template-columns:repeat(auto-fit,minmax(350px,1fr));gap:20px;margin-bottom:30px;}"
".card{background:white;border-radius:15px;padding:20px;box-shadow:0 10px 30px rgba(0,0,0,0.2);transition:transform 0.3s ease;}"
".card:hover{transform:translateY(-5px);}"
".card h3{color:#4a5568;margin-bottom:15px;font-size:1.3rem;text-align:center;}"
".sensor-value{display:flex;align-items:center;justify-content:center;margin-bottom:20px;}"
".value{font-size:2.5rem;font-weight:bold;margin-right:10px;}"
".unit{font-size:1.2rem;color:#718096;}"
".temp .value{color:#e53e3e;}"
".humidity .value{color:#3182ce;}"
".pressure .value{color:#38a169;}"
".chart-container{height:200px;position:relative;}"
".status{background:white;border-radius:15px;padding:20px;margin-top:20px;box-shadow:0 10px 30px rgba(0,0,0,0.2);}"
".status-item{display:flex;justify-content:space-between;align-items:center;margin-bottom:10px;}"
".status-label{font-weight:bold;color:#4a5568;}"
".status-online{color:#38a169;font-weight:bold;}"
".last-update{text-align:center;color:#718096;font-size:0.9rem;margin-top:15px;}"
"@media(max-width:768px){.container{padding:10px;}.header h1{font-size:2rem;}.value{font-size:2rem;}}"
"</style>"
"</head>"
"<body>"
"<div class='container'>"
"<div class='header'><h1>🌤️ Weather Station IoT</h1><p>Monitoramento em Tempo Real</p></div>"
"<div class='dashboard'>"
"<div class='card temp'><h3>🌡️ Temperatura</h3><div class='sensor-value'><span class='value' id='temp-value'>--</span><span class='unit'>°C</span></div><div class='chart-container'><canvas id='tempChart'></canvas></div></div>"
"<div class='card humidity'><h3>💧 Umidade</h3><div class='sensor-value'><span class='value' id='humidity-value'>--</span><span class='unit'>%</span></div><div class='chart-container'><canvas id='humidityChart'></canvas></div></div>"
"<div class='card pressure'><h3>🌪️ Pressão</h3><div class='sensor-value'><span class='value' id='pressure-value'>--</span><span class='unit'>kPa</span></div><div class='chart-container'><canvas id='pressureChart'></canvas></div></div>"
"</div>"
"<div class='status'><h3>📊 Status do Sistema</h3>"
"<div class='status-item'><span class='status-label'>Status:</span><span class='status-online'>🟢 Online</span></div>"
"<div class='status-item'><span class='status-label'>Última Atualização:</span><span id='last-update'>--</span></div>"
"<div class='status-item'><span class='status-label'>Intervalo de Atualização:</span><span>2 segundos</span></div>"
"</div>"
"</div>"
"<script>"
"let tempData=[],humidityData=[],pressureData=[],timeLabels=[];"
"const maxDataPoints=20;"
"const tempChart=new Chart(document.getElementById('tempChart'),{type:'line',data:{labels:timeLabels,datasets:[{label:'Temperatura (°C)',data:tempData,borderColor:'#e53e3e',backgroundColor:'rgba(229,62,62,0.1)',borderWidth:2,fill:true,tension:0.4}]},options:{responsive:true,maintainAspectRatio:false,scales:{y:{beginAtZero:false,grid:{color:'rgba(0,0,0,0.1)'}},x:{grid:{color:'rgba(0,0,0,0.1)'}}},plugins:{legend:{display:false}}}});"
"const humidityChart=new Chart(document.getElementById('humidityChart'),{type:'line',data:{labels:timeLabels,datasets:[{label:'Umidade (%)',data:humidityData,borderColor:'#3182ce',backgroundColor:'rgba(49,130,206,0.1)',borderWidth:2,fill:true,tension:0.4}]},options:{responsive:true,maintainAspectRatio:false,scales:{y:{beginAtZero:true,max:100,grid:{color:'rgba(0,0,0,0.1)'}},x:{grid:{color:'rgba(0,0,0,0.1)'}}},plugins:{legend:{display:false}}}});"
"const pressureChart=new Chart(document.getElementById('pressureChart'),{type:'line',data:{labels:timeLabels,datasets:[{label:'Pressão (kPa)',data:pressureData,borderColor:'#38a169',backgroundColor:'rgba(56,161,105,0.1)',borderWidth:2,fill:true,tension:0.4}]},options:{responsive:true,maintainAspectRatio:false,scales:{y:{beginAtZero:false,grid:{color:'rgba(0,0,0,0.1)'}},x:{grid:{color:'rgba(0,0,0,0.1)'}}},plugins:{legend:{display:false}}}});"
"function updateData(){fetch('/dados').then(response=>response.json()).then(data=>{const now=new Date().toLocaleTimeString();document.getElementById('temp-value').textContent=data.temperatura.toFixed(1);document.getElementById('humidity-value').textContent=data.umidade.toFixed(1);document.getElementById('pressure-value').textContent=data.pressao;document.getElementById('last-update').textContent=now;timeLabels.push(now);tempData.push(data.temperatura);humidityData.push(data.umidade);pressureData.push(data.pressao);if(timeLabels.length>maxDataPoints){timeLabels.shift();tempData.shift();humidityData.shift();pressureData.shift();}tempChart.update('none');humidityChart.update('none');pressureChart.update('none');}).catch(error=>{console.error('Erro ao buscar dados:',error);document.getElementById('last-update').textContent='Erro de conexão';});}"
"updateData();setInterval(updateData,1000);"
"</script>"
"</body></html>";


/**
 * Função privada que inicializa e configura o servidor HTTP
 * - Cria um novo PCB (Protocol Control Block) TCP
 * - Faz o bind para a porta 80 (HTTP padrão)
 * - Coloca o servidor em modo de escuta
 * - Define callback para novas conexões
 */
static void start_http_server(void) {
    struct tcp_pcb *pcb = tcp_new();
    if (!pcb) {
        printf("Erro ao criar PCB TCP\n");
        return;
    }
    if (tcp_bind(pcb, IP_ADDR_ANY, 80) != ERR_OK) {
        printf("Erro ao ligar o servidor na porta 80\n");
        return;
    }
    pcb = tcp_listen(pcb);
    tcp_accept(pcb, connection_callback);
    printf("Servidor HTTP rodando na porta 80...\n");
}

/**
 * Função pública para inicializar o módulo web
 * Esta é a função chamada pelo main() para iniciar o servidor HTTP
 */
void init_web(void) {
    // Inicializa o servidor HTTP
    start_http_server();
}

/**
 * - Callback chamado quando dados são enviados com sucesso via TCP
 * - Atualiza o contador de bytes enviados
 * - Fecha a conexão quando todos os dados foram enviados
 * - Libera a memória alocada para o estado HTTP
 */
static err_t http_sent(void *arg, struct tcp_pcb *tpcb, u16_t len) {
    struct http_state *hs = (struct http_state *)arg;
    hs->sent += len;
    if (hs->sent >= hs->len) {
        tcp_close(tpcb);
        free(hs);
    }
    return ERR_OK;
}

/**
 * Callback principal que processa requisições HTTP recebidas
 * - Analisa o tipo de requisição (GET, POST, etc.)
 * - Gera resposta apropriada (HTML ou JSON)
 * - Envia resposta de volta ao cliente
 * - Gerencia memória e conexão TCP
 */
static err_t http_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    // Se não há dados, fecha a conexão
    if (!p) {
        tcp_close(tpcb);
        return ERR_OK;
    }

    char *req = (char *)p->payload;
    struct http_state *hs = malloc(sizeof(struct http_state));
    if (!hs) {
        pbuf_free(p);
        tcp_close(tpcb);
        return ERR_MEM;
    }
    hs->sent = 0;

    
    // Rota para status em JSON - retorna dados atuais do sistema
    if (strstr(req, "GET /dados")) {
        char json_payload[96];
        int json_len = snprintf(json_payload, sizeof(json_payload),
        "{\"temperatura\":%.2f,\"pressao\":%d,\"umidade\":%.2f}\r\n",
        temperatura, pressao, umidade);

        printf("[DEBUG] JSON: %s\n", json_payload);
        
        hs->len = snprintf(hs->response, sizeof(hs->response),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        json_len, json_payload);
    } else {
        hs->len = snprintf(hs->response, sizeof(hs->response),
                           "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/html\r\n"
                           "Content-Length: %d\r\n"
                           "Connection: close\r\n"
                           "\r\n"
                           "%s",
                           (int)strlen(HTML_BODY), HTML_BODY);
    }

    // Configura callbacks e envia resposta
    tcp_arg(tpcb, hs);
    tcp_sent(tpcb, http_sent);

    tcp_write(tpcb, hs->response, hs->len, TCP_WRITE_FLAG_COPY);
    tcp_output(tpcb);

    pbuf_free(p);
    return ERR_OK;
}

/**
 * Callback chamado quando uma nova conexão TCP é estabelecida
 * - Configura o callback para receber dados HTTP
 * - Retorna ERR_OK para aceitar a conexão
 */
static err_t connection_callback(void *arg, struct tcp_pcb *newpcb, err_t err) {
    tcp_recv(newpcb, http_recv);
    return ERR_OK;
}