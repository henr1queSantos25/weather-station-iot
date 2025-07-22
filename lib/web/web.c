#include "web.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ========================================================================
// VARIÁVEIS EXTERNAS
// ========================================================================
// Acesso às variáveis globais do arquivo principal weather-station-iot.c
extern float temperatura;
extern int32_t pressao;
extern float umidade;
extern int limiteMAX_temp;
extern int limiteMAX_umi;
extern int limiteMAX_pressao;
extern int limiteMIN_temp;
extern int limiteMIN_umi;
extern int limiteMIN_pressao;
extern int offset_temp;
extern int offset_umi;
extern int offset_pressao;

// ========================================================================
// PROTÓTIPOS DE FUNÇÕES ESTÁTICAS (PRIVADAS)
// ========================================================================
static void start_http_server(void);
static err_t connection_callback(void *arg, struct tcp_pcb *newpcb, err_t err);
static err_t http_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static err_t http_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);

// ========================================================================
// CONTEÚDO HTML DA INTERFACE WEB
// ========================================================================
// Página completa com estilos CSS e JavaScript integrados
// Contém dashboard com gráficos, configuração de limites e calibração
const char HTML_BODY[] =
    "<!DOCTYPE html><html lang='pt-BR'><head>"
    "<meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1.0'>"
    "<title>Weather Station IoT</title><script src='https://cdn.jsdelivr.net/npm/chart.js'></script>"
    "<style>*{margin:0;padding:0;box-sizing:border-box}"
    "body{font-family:sans-serif;background:linear-gradient(135deg,#667eea,#764ba2);color:#333;min-height:100vh}"
    ".ctn{max-width:1200px;margin:auto;padding:20px}.hdr{text-align:center;margin-bottom:30px;color:#fff}"
    ".hdr h1{font-size:2.5rem;margin-bottom:10px;text-shadow:2px 2px 4px rgba(0,0,0,0.3)}"
    ".hdr p{font-size:1.1rem;opacity:0.9}.dash{display:grid;grid-template-columns:repeat(auto-fit,minmax(350px,1fr));gap:20px;margin-bottom:30px}"
    ".cd{background:#fff;border-radius:15px;padding:20px;box-shadow:0 10px 30px rgba(0,0,0,0.2);transition:transform .3s}.cd:hover{transform:translateY(-5px)}"
    ".cd h3{text-align:center;font-size:1.3rem;margin-bottom:15px;color:#4a5568}"
    ".val{display:flex;align-items:center;justify-content:center;margin-bottom:20px}"
    ".v{font-size:2.5rem;font-weight:bold;margin-right:10px}.u{font-size:1.2rem;color:#718096}"
    ".t .v{color:#e53e3e}.h .v{color:#3182ce}.p .v{color:#38a169}.ch{height:200px;position:relative}"
    ".alert{margin-bottom:15px;padding:10px;border-radius:8px;font-weight:bold;text-align:center;display:none}"
    ".alert.show{display:block}.alert.danger{background:#fed7d7;color:#c53030;border:1px solid #feb2b2}"
    ".alert.success{background:#c6f6d5;color:#2f855a;border:1px solid #9ae6b4}"
    ".status{font-size:0.9rem;color:#666;margin-bottom:10px;text-align:center}"
    ".lim{background:#fff;border-radius:15px;padding:20px;margin-top:20px;box-shadow:0 10px 30px rgba(0,0,0,0.2)}"
    ".cal{background:#fff;border-radius:15px;padding:20px;margin-top:20px;box-shadow:0 10px 30px rgba(0,0,0,0.2)}"
    ".lg{display:grid;grid-template-columns:repeat(auto-fit,minmax(300px,1fr));gap:20px}"
    ".g{border:1px solid #e2e8f0;border-radius:10px;padding:15px}"
    ".g h4{text-align:center;margin-bottom:15px;color:#4a5568}"
    ".inp{display:flex;align-items:center;margin-bottom:10px}"
    ".inp label{min-width:80px;color:#718096;font-weight:500}"
    ".inp input{flex:1;padding:8px;border:1px solid #cbd5e0;border-radius:5px;margin:0 10px}"
    ".inp span{color:#718096}.btns{text-align:center;margin-top:15px}"
    ".btn{background:#4299e1;color:#fff;border:none;padding:10px 20px;border-radius:5px;cursor:pointer;transition:background 0.3s}"
    ".btn:hover{background:#3182ce}.bt-t{background:#e53e3e}.bt-t:hover{background:#c53030}"
    ".bt-h{background:#3182ce}.bt-h:hover{background:#2c5282}.bt-p{background:#38a169}.bt-p:hover{background:#2f855a}"
    ".bt-cal{background:#805ad5}.bt-cal:hover{background:#6b46c1}"
    "@media(max-width:768px){.ctn{padding:10px}.hdr h1{font-size:2rem}.v{font-size:2rem}.lg{grid-template-columns:1fr}}"
    "</style></head><body>"
    "<div class='ctn'><div class='hdr'><h1>🌤️ Weather Station IoT</h1><p>Monitoramento em Tempo Real</p></div>"
    "<div class='dash'>"
    "<div class='cd t'><h3>🌡️ Temperatura</h3>"
    "<div class='alert' id='t-alert'></div>"
    "<div class='status' id='t-status'>Limites: -- a -- °C</div>"
    "<div class='val'><span class='v' id='tv'>--</span><span class='u'>°C</span></div><div class='ch'><canvas id='tc'></canvas></div></div>"
    "<div class='cd h'><h3>💧 Umidade</h3>"
    "<div class='alert' id='h-alert'></div>"
    "<div class='status' id='h-status'>Limites: -- a -- %</div>"
    "<div class='val'><span class='v' id='hv'>--</span><span class='u'>%</span></div><div class='ch'><canvas id='hc'></canvas></div></div>"
    "<div class='cd p'><h3>🌪️ Pressão</h3>"
    "<div class='alert' id='p-alert'></div>"
    "<div class='status' id='p-status'>Limites: -- a -- kPa</div>"
    "<div class='val'><span class='v' id='pv'>--</span><span class='u'>kPa</span></div><div class='ch'><canvas id='pc'></canvas></div></div>"
    "</div>"
    "<div class='cal'><h3>🔧 Calibração dos Sensores (Offset)</h3>"
    "<div class='lg'>"
    "<div class='g'><h4>🌡️ Temperatura</h4>"
    "<div class='inp'><label>Offset:</label><input type='number' id='toff' step='0.1' placeholder='Ex: -3.0'><span>°C</span></div>"
    "<div class='btns'><button class='btn bt-cal' onclick='sto()'>Aplicar Offset</button></div></div>"
    "<div class='g'><h4>💧 Umidade</h4>"
    "<div class='inp'><label>Offset:</label><input type='number' id='hoff' step='0.1' placeholder='Ex: 2.5'><span>%</span></div>"
    "<div class='btns'><button class='btn bt-cal' onclick='sho()'>Aplicar Offset</button></div></div>"
    "<div class='g'><h4>🌪️ Pressão</h4>"
    "<div class='inp'><label>Offset:</label><input type='number' id='poff' step='0.1' placeholder='Ex: -1.2'><span>kPa</span></div>"
    "<div class='btns'><button class='btn bt-cal' onclick='spo()'>Aplicar Offset</button></div></div>"
    "</div></div>"
    "<div class='lim'><h3>⚙️ Configuração de Limites</h3><div class='lg'>"
    "<div class='g'><h4>🌡️ Temperatura (°C)</h4>"
    "<div class='inp'><label>Mínimo:</label><input type='number' id='tmin' step='0.1' placeholder='Ex: 18.0'><span>°C</span></div>"
    "<div class='inp'><label>Máximo:</label><input type='number' id='tmax' step='0.1' placeholder='Ex: 35.0'><span>°C</span></div>"
    "<div class='btns'><button class='btn bt-t' onclick='stm()'>Definir Min</button> <button class='btn bt-t' onclick='stx()'>Definir Max</button></div></div>"
    "<div class='g'><h4>💧 Umidade (%)</h4>"
    "<div class='inp'><label>Mínimo:</label><input type='number' id='hmin' step='0.1' min='0' max='100' placeholder='Ex: 30.0'><span>%</span></div>"
    "<div class='inp'><label>Máximo:</label><input type='number' id='hmax' step='0.1' min='0' max='100' placeholder='Ex: 80.0'><span>%</span></div>"
    "<div class='btns'><button class='btn bt-h' onclick='shm()'>Definir Min</button> <button class='btn bt-h' onclick='shx()'>Definir Max</button></div></div>"
    "<div class='g'><h4>🌪️ Pressão (kPa)</h4>"
    "<div class='inp'><label>Mínimo:</label><input type='number' id='pmin' step='1' placeholder='Ex: 95'><span>kPa</span></div>"
    "<div class='inp'><label>Máximo:</label><input type='number' id='pmax' step='1' placeholder='Ex: 105'><span>kPa</span></div>"
    "<div class='btns'><button class='btn bt-p' onclick='spm()'>Definir Min</button> <button class='btn bt-p' onclick='spx()'>Definir Max</button></div></div>"
    "</div></div></div>"
    "<script>"
    "let td=[],hd=[],pd=[],tl=[],max=20,lm={t:{mi:null,ma:null},h:{mi:null,ma:null},p:{mi:null,ma:null}};"
    "const tc=new Chart(document.getElementById('tc'),{type:'line',data:{labels:tl,datasets:[{label:'Temperatura (°C)',data:td,borderColor:'#e53e3e',backgroundColor:'rgba(229,62,62,0.1)',borderWidth:2,fill:true,tension:0.4}]},options:{responsive:true,maintainAspectRatio:false,scales:{y:{beginAtZero:false},x:{}},plugins:{legend:{display:false}}}});"
    "const hc=new Chart(document.getElementById('hc'),{type:'line',data:{labels:tl,datasets:[{label:'Umidade (%)',data:hd,borderColor:'#3182ce',backgroundColor:'rgba(49,130,206,0.1)',borderWidth:2,fill:true,tension:0.4}]},options:{responsive:true,maintainAspectRatio:false,scales:{y:{beginAtZero:true,max:100},x:{}},plugins:{legend:{display:false}}}});"
    "const pc=new Chart(document.getElementById('pc'),{type:'line',data:{labels:tl,datasets:[{label:'Pressão (kPa)',data:pd,borderColor:'#38a169',backgroundColor:'rgba(56,161,105,0.1)',borderWidth:2,fill:true,tension:0.4}]},options:{responsive:true,maintainAspectRatio:false,scales:{y:{beginAtZero:false},x:{}},plugins:{legend:{display:false}}}});"
    "function chkLim(v,mi,ma,type,unit){"
    "  const alert=document.getElementById(type+'-alert');"
    "  const status=document.getElementById(type+'-status');"
    "  const minTxt=mi!==null?mi:'--';"
    "  const maxTxt=ma!==null?ma:'--';"
    "  status.textContent='Limites: '+minTxt+' a '+maxTxt+' '+unit;"
    "  if(mi!==null&&v<mi){"
    "    alert.textContent='⚠️ Valor ABAIXO do limite mínimo ('+mi+unit+')';"
    "    alert.className='alert danger show';"
    "  }else if(ma!==null&&v>ma){"
    "    alert.textContent='⚠️ Valor ACIMA do limite máximo ('+ma+unit+')';"
    "    alert.className='alert danger show';"
    "  }else if(mi!==null||ma!==null){"
    "    alert.textContent='✅ Valor dentro dos limites';"
    "    alert.className='alert success show';"
    "  }else{"
    "    alert.className='alert';"
    "  }"
    "}"
    "function sto(){"
    "  let v=parseFloat(document.getElementById('toff').value);"
    "  if(isNaN(v)||v<-50||v>50){alert('Offset de temperatura inválido (-50 a +50 °C)');return;}"
    "  fetch('/config/offset_temp/'+v.toFixed(1)).then(()=>alert('Offset temperatura aplicado: '+v+'°C')).catch(()=>alert('Erro ao aplicar offset'));"
    "}"
    "function sho(){"
    "  let v=parseFloat(document.getElementById('hoff').value);"
    "  if(isNaN(v)||v<-50||v>50){alert('Offset de umidade inválido (-50 a +50 %)');return;}"
    "  let atual = parseFloat(document.getElementById('hv').textContent);"
    "  if(!isNaN(atual) && (atual+v<0 || atual+v>100)){alert('Offset resultaria em um valor de umidade fora do intervalo 0% a 100%');return;}"
    "  fetch('/config/offset_umi/'+v.toFixed(1)).then(()=>alert('Offset umidade aplicado: '+v+'%')).catch(()=>alert('Erro ao aplicar offset'));"
    "}"
    "function spo(){"
    "  let v=parseFloat(document.getElementById('poff').value);"
    "  if(isNaN(v)||v<-50||v>50){alert('Offset de pressão inválido (-50 a +50 kPa)');return;}"
    "  let atual = parseFloat(document.getElementById('pv').textContent);"
    "  if(!isNaN(atual) && (atual+v<50 || atual+v>150)){alert('Offset resultaria em um valor de pressão fora do intervalo 50 a 150 kPa');return;}"
    "  fetch('/config/offset_pressao/'+v.toFixed(1)).then(()=>alert('Offset pressão aplicado: '+v+'kPa')).catch(()=>alert('Erro ao aplicar offset'));"
    "}"
    "function stm(){let v=parseFloat(document.getElementById('tmin').value);if(isNaN(v)||v<-50||v>100||lm.t.ma!==null&&v>=lm.t.ma){alert('Valor inválido');return;}fetch('/config/limiteMIN_temp/'+Math.round(v)).then(()=>{lm.t.mi=Math.round(v);alert('Min temperatura: '+v);}).catch(()=>alert('Erro'));}"
    "function stx(){let v=parseFloat(document.getElementById('tmax').value);if(isNaN(v)||v<-50||v>100||lm.t.mi!==null&&v<=lm.t.mi){alert('Valor inválido');return;}fetch('/config/limiteMAX_temp/'+Math.round(v)).then(()=>{lm.t.ma=Math.round(v);alert('Max temperatura: '+v);}).catch(()=>alert('Erro'));}"
    "function shm(){let v=parseFloat(document.getElementById('hmin').value);if(isNaN(v)||v<0||v>100||lm.h.ma!==null&&v>=lm.h.ma){alert('Valor inválido');return;}fetch('/config/limiteMIN_umi/'+Math.round(v)).then(()=>{lm.h.mi=Math.round(v);alert('Min umidade: '+v);}).catch(()=>alert('Erro'));}"
    "function shx(){let v=parseFloat(document.getElementById('hmax').value);if(isNaN(v)||v<0||v>100||lm.h.mi!==null&&v<=lm.h.mi){alert('Valor inválido');return;}fetch('/config/limiteMAX_umi/'+Math.round(v)).then(()=>{lm.h.ma=Math.round(v);alert('Max umidade: '+v);}).catch(()=>alert('Erro'));}"
    "function spm(){let v=parseFloat(document.getElementById('pmin').value);if(isNaN(v)||v<50||v>150||lm.p.ma!==null&&v>=lm.p.ma){alert('Valor inválido');return;}fetch('/config/limiteMIN_pressao/'+Math.round(v)).then(()=>{lm.p.mi=Math.round(v);alert('Min pressão: '+v);}).catch(()=>alert('Erro'));}"
    "function spx(){let v=parseFloat(document.getElementById('pmax').value);if(isNaN(v)||v<50||v>150||lm.p.mi!==null&&v<=lm.p.mi){alert('Valor inválido');return;}fetch('/config/limiteMAX_pressao/'+Math.round(v)).then(()=>{lm.p.ma=Math.round(v);alert('Max pressão: '+v);}).catch(()=>alert('Erro'));}"
    "function upd(){fetch('/dados').then(r=>r.json()).then(d=>{"
    "  const t=new Date().toLocaleTimeString();"
    "  document.getElementById('tv').textContent=d.temperatura.toFixed(1);"
    "  document.getElementById('hv').textContent=d.umidade.toFixed(1);"
    "  document.getElementById('pv').textContent=d.pressao;"
    "  "
    "  if(d.limiteMIN_temp !== undefined && d.limiteMIN_temp !== -2147483648) lm.t.mi = d.limiteMIN_temp; else lm.t.mi = null;"
    "  if(d.limiteMAX_temp !== undefined && d.limiteMAX_temp !== 2147483647) lm.t.ma = d.limiteMAX_temp; else lm.t.ma = null;"
    "  if(d.limiteMIN_umi !== undefined && d.limiteMIN_umi !== -2147483648) lm.h.mi = d.limiteMIN_umi; else lm.h.mi = null;"
    "  if(d.limiteMAX_umi !== undefined && d.limiteMAX_umi !== 2147483647) lm.h.ma = d.limiteMAX_umi; else lm.h.ma = null;"
    "  if(d.limiteMIN_pressao !== undefined && d.limiteMIN_pressao !== -2147483648) lm.p.mi = d.limiteMIN_pressao; else lm.p.mi = null;"
    "  if(d.limiteMAX_pressao !== undefined && d.limiteMAX_pressao !== 2147483647) lm.p.ma = d.limiteMAX_pressao; else lm.p.ma = null;"
    "  "
    "  chkLim(d.temperatura,lm.t.mi,lm.t.ma,'t','°C');"
    "  chkLim(d.umidade,lm.h.mi,lm.h.ma,'h','%');"
    "  chkLim(d.pressao,lm.p.mi,lm.p.ma,'p','kPa');"
    "  tl.push(t);td.push(d.temperatura);hd.push(d.umidade);pd.push(d.pressao);"
    "  if(tl.length>max){tl.shift();td.shift();hd.shift();pd.shift();}"
    "  tc.update();hc.update();pc.update();"
    "}).catch(e=>console.error('Erro:',e));}"
    "upd();setInterval(upd,1000);"
    "</script></body></html>";

// ========================================================================
// FUNÇÕES PÚBLICAS DE INICIALIZAÇÃO
// ========================================================================

/**
 * @brief Inicializa a conexão Wi-Fi
 * @param ip_str Buffer para armazenar o IP como string (mínimo 16 bytes)
 * @param WIFI_SSID Nome da rede Wi-Fi
 * @param WIFI_PASS Senha da rede Wi-Fi
 * @return 0 em caso de sucesso, -1 em caso de erro
 */
int inicializar_wifi(char *ip_str, char *WIFI_SSID, char *WIFI_PASS)
{
    // === INICIALIZAÇÃO DO MÓDULO Wi-Fi ===
    if (cyw43_arch_init())
    {
        printf("WiFi => FALHA na inicialização\n");
        sleep_ms(100);
        return -1;
    }

    // === HABILITAÇÃO DO MODO STATION (CLIENTE) ===
    cyw43_arch_enable_sta_mode();

    // === CONEXÃO À REDE Wi-Fi ===
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 10000))
    {
        printf("WiFi => ERRO na conexão\n");
        sleep_ms(100);
        return -1;
    }

    // === OBTENÇÃO E EXIBIÇÃO DO ENDEREÇO IP ===
    uint8_t *ip = (uint8_t *)&(cyw43_state.netif[0].ip_addr.addr);
    snprintf(ip_str, 16, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);

    printf("WiFi => Conectado com sucesso!\n IP: %s\n", ip_str);

    return 0; // Retorna 0 para indicar sucesso
}

/**
 * @brief Inicializa o módulo web (servidor HTTP)
 * Esta é a função chamada pelo main() para iniciar o servidor HTTP
 */
void init_web(void)
{
    start_http_server();
}

// ========================================================================
// FUNÇÕES PRIVADAS DO SERVIDOR HTTP
// ========================================================================

/**
 * @brief Inicializa e configura o servidor HTTP
 * - Cria um novo PCB (Protocol Control Block) TCP
 * - Faz o bind para a porta 80 (HTTP padrão)
 * - Coloca o servidor em modo de escuta
 * - Define callback para novas conexões
 */
static void start_http_server(void)
{
    struct tcp_pcb *pcb = tcp_new();
    if (!pcb)
    {
        printf("Erro ao criar PCB TCP\n");
        return;
    }
    
    if (tcp_bind(pcb, IP_ADDR_ANY, 80) != ERR_OK)
    {
        printf("Erro ao fazer bind do servidor na porta 80\n");
        return;
    }
    
    pcb = tcp_listen(pcb);
    tcp_accept(pcb, connection_callback);
    printf("Servidor HTTP rodando na porta 80...\n");
}

/**
 * @brief Callback chamado quando uma nova conexão TCP é estabelecida
 * @param arg Argumento customizado (não utilizado)
 * @param newpcb Novo PCB da conexão estabelecida
 * @param err Status de erro da conexão
 * @return ERR_OK para aceitar a conexão
 */
static err_t connection_callback(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    tcp_recv(newpcb, http_recv);
    return ERR_OK;
}

/**
 * @brief Callback chamado quando dados são enviados com sucesso via TCP
 * @param arg Ponteiro para estrutura http_state
 * @param tpcb PCB da conexão TCP
 * @param len Número de bytes enviados com sucesso
 * @return ERR_OK sempre
 */
static err_t http_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
    struct http_state *hs = (struct http_state *)arg;
    hs->sent += len;
    
    // === FECHAMENTO DA CONEXÃO APÓS ENVIO COMPLETO ===
    if (hs->sent >= hs->len)
    {
        tcp_close(tpcb);
        free(hs);
    }
    return ERR_OK;
}

/**
 * @brief Callback principal que processa requisições HTTP recebidas
 * @param arg Argumento customizado (não utilizado)
 * @param tpcb PCB da conexão TCP
 * @param p Buffer com dados recebidos
 * @param err Status de erro
 * @return ERR_OK em caso de sucesso, ERR_MEM em caso de falta de memória
 */
static err_t http_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    // === VERIFICAÇÃO DE DADOS RECEBIDOS ===
    if (!p)
    {
        tcp_close(tpcb);
        return ERR_OK;
    }

    char *req = (char *)p->payload;
    
    // === ALOCAÇÃO DE MEMÓRIA PARA ESTADO HTTP ===
    struct http_state *hs = malloc(sizeof(struct http_state));
    if (!hs)
    {
        pbuf_free(p);
        tcp_close(tpcb);
        return ERR_MEM;
    }
    hs->sent = 0;

    // ========================================================================
    // ROTEAMENTO DE REQUISIÇÕES HTTP
    // ========================================================================

    // === ROTA: /dados (JSON com dados dos sensores) ===
    if (strstr(req, "GET /dados")) {
        char json_payload[256];
        int json_len = snprintf(json_payload, sizeof(json_payload),
                            "{\"temperatura\":%.2f,\"pressao\":%d,\"umidade\":%.2f,"
                            "\"limiteMIN_temp\":%d,\"limiteMAX_temp\":%d,"
                            "\"limiteMIN_umi\":%d,\"limiteMAX_umi\":%d,"
                            "\"limiteMIN_pressao\":%d,\"limiteMAX_pressao\":%d}\r\n",
                            temperatura, pressao, umidade,
                            limiteMIN_temp, limiteMAX_temp,
                            limiteMIN_umi, limiteMAX_umi,
                            limiteMIN_pressao, limiteMAX_pressao);

        hs->len = snprintf(hs->response, sizeof(hs->response),
                           "HTTP/1.1 200 OK\r\n"
                           "Content-Type: application/json\r\n"
                           "Content-Length: %d\r\n"
                           "Connection: close\r\n"
                           "\r\n"
                           "%s",
                           json_len, json_payload);
    } 
    // === ROTAS: Configuração de limites máximos ===
    else if (strstr(req, "GET /config/limiteMAX_temp/")) {
        char *pos = strstr(req, "/config/limiteMAX_temp/") + strlen("/config/limiteMAX_temp/");
        char valor_str[16] = {0};
        int i = 0;
        
        // Extração do valor numérico da URL
        while (pos[i] != ' ' && pos[i] != '\r' && pos[i] != '\n' && pos[i] != '\0' && i < 15)
        {
            valor_str[i] = pos[i];
            i++;
        }
        valor_str[i] = '\0';

        int novo_limite = atoi(valor_str);
        limiteMAX_temp = novo_limite;

        printf("[DEBUG] Novo limite máximo de temperatura: %d°C\n", limiteMAX_temp);

        const char *txt = "Limite máximo de temperatura atualizado";
        hs->len = snprintf(hs->response, sizeof(hs->response),
                           "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/plain\r\n"
                           "Content-Length: %d\r\n"
                           "Connection: close\r\n"
                           "\r\n"
                           "%s",
                           (int)strlen(txt), txt);
    } 
    else if (strstr(req, "GET /config/limiteMAX_umi/")) {
        char *pos = strstr(req, "/config/limiteMAX_umi/") + strlen("/config/limiteMAX_umi/");
        char valor_str[16] = {0};
        int i = 0;
        
        while (pos[i] != ' ' && pos[i] != '\r' && pos[i] != '\n' && pos[i] != '\0' && i < 15)
        {
            valor_str[i] = pos[i];
            i++;
        }
        valor_str[i] = '\0';

        int novo_limite = atoi(valor_str);
        limiteMAX_umi = novo_limite;

        printf("[DEBUG] Novo limite máximo de umidade: %d%%\n", limiteMAX_umi);

        const char *txt = "Limite máximo de umidade atualizado";
        hs->len = snprintf(hs->response, sizeof(hs->response),
                           "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/plain\r\n"
                           "Content-Length: %d\r\n"
                           "Connection: close\r\n"
                           "\r\n"
                           "%s",
                           (int)strlen(txt), txt);
    } 
    else if (strstr(req, "GET /config/limiteMAX_pressao/")) {
        char *pos = strstr(req, "/config/limiteMAX_pressao/") + strlen("/config/limiteMAX_pressao/");
        char valor_str[16] = {0};
        int i = 0;
        
        while (pos[i] != ' ' && pos[i] != '\r' && pos[i] != '\n' && pos[i] != '\0' && i < 15)
        {
            valor_str[i] = pos[i];
            i++;
        }
        valor_str[i] = '\0';

        int novo_limite = atoi(valor_str);
        limiteMAX_pressao = novo_limite;

        printf("[DEBUG] Novo limite máximo de pressão: %dkPa\n", limiteMAX_pressao);

        const char *txt = "Limite máximo de pressão atualizado";
        hs->len = snprintf(hs->response, sizeof(hs->response),
                           "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/plain\r\n"
                           "Content-Length: %d\r\n"
                           "Connection: close\r\n"
                           "\r\n"
                           "%s",
                           (int)strlen(txt), txt);
    } 
    // === ROTAS: Configuração de limites mínimos ===
    else if (strstr(req, "GET /config/limiteMIN_temp/")) {
        char *pos = strstr(req, "/config/limiteMIN_temp/") + strlen("/config/limiteMIN_temp/");
        char valor_str[16] = {0};
        int i = 0;
        
        while (pos[i] != ' ' && pos[i] != '\r' && pos[i] != '\n' && pos[i] != '\0' && i < 15)
        {
            valor_str[i] = pos[i];
            i++;
        }
        valor_str[i] = '\0';

        int novo_limite = atoi(valor_str);
        limiteMIN_temp = novo_limite;

        printf("[DEBUG] Novo limite mínimo de temperatura: %d°C\n", limiteMIN_temp);

        const char *txt = "Limite mínimo de temperatura atualizado";
        hs->len = snprintf(hs->response, sizeof(hs->response),
                           "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/plain\r\n"
                           "Content-Length: %d\r\n"
                           "Connection: close\r\n"
                           "\r\n"
                           "%s",
                           (int)strlen(txt), txt);
    } 
    else if (strstr(req, "GET /config/limiteMIN_umi/")) {
        char *pos = strstr(req, "/config/limiteMIN_umi/") + strlen("/config/limiteMIN_umi/");
        char valor_str[16] = {0};
        int i = 0;
        
        while (pos[i] != ' ' && pos[i] != '\r' && pos[i] != '\n' && pos[i] != '\0' && i < 15)
        {
            valor_str[i] = pos[i];
            i++;
        }
        valor_str[i] = '\0';

        int novo_limite = atoi(valor_str);
        limiteMIN_umi = novo_limite;

        printf("[DEBUG] Novo limite mínimo de umidade: %d%%\n", limiteMIN_umi);

        const char *txt = "Limite mínimo de umidade atualizado";
        hs->len = snprintf(hs->response, sizeof(hs->response),
                           "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/plain\r\n"
                           "Content-Length: %d\r\n"
                           "Connection: close\r\n"
                           "\r\n"
                           "%s",
                           (int)strlen(txt), txt);
    } 
    else if (strstr(req, "GET /config/limiteMIN_pressao/")) {
        char *pos = strstr(req, "/config/limiteMIN_pressao/") + strlen("/config/limiteMIN_pressao/");
        char valor_str[16] = {0};
        int i = 0;
        
        while (pos[i] != ' ' && pos[i] != '\r' && pos[i] != '\n' && pos[i] != '\0' && i < 15)
        {
            valor_str[i] = pos[i];
            i++;
        }
        valor_str[i] = '\0';

        int novo_limite = atoi(valor_str);
        limiteMIN_pressao = novo_limite;

        printf("[DEBUG] Novo limite mínimo de pressão: %dkPa\n", limiteMIN_pressao);

        const char *txt = "Limite mínimo de pressão atualizado";
        hs->len = snprintf(hs->response, sizeof(hs->response),
                           "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/plain\r\n"
                           "Content-Length: %d\r\n"
                           "Connection: close\r\n"
                           "\r\n"
                           "%s",
                           (int)strlen(txt), txt);
    } 
    // === ROTAS: Configuração de offsets de calibração ===
    else if (strstr(req, "GET /config/offset_temp/")) {
        char *pos = strstr(req, "/config/offset_temp/") + strlen("/config/offset_temp/");
        char valor_str[16] = {0};
        int i = 0;
        
        while (pos[i] != ' ' && pos[i] != '\r' && pos[i] != '\n' && pos[i] != '\0' && i < 15)
        {
            valor_str[i] = pos[i];
            i++;
        }
        valor_str[i] = '\0';

        int novo_offset = atoi(valor_str);
        offset_temp = novo_offset;

        printf("[DEBUG] Novo offset de temperatura: %d°C\n", offset_temp);

        const char *txt = "Offset de temperatura atualizado";
        hs->len = snprintf(hs->response, sizeof(hs->response),
                           "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/plain\r\n"
                           "Content-Length: %d\r\n"
                           "Connection: close\r\n"
                           "\r\n"
                           "%s",
                           (int)strlen(txt), txt);
    } 
    else if (strstr(req, "GET /config/offset_umi/")) {
        char *pos = strstr(req, "/config/offset_umi/") + strlen("/config/offset_umi/");
        char valor_str[16] = {0};
        int i = 0;
        
        while (pos[i] != ' ' && pos[i] != '\r' && pos[i] != '\n' && pos[i] != '\0' && i < 15)
        {
            valor_str[i] = pos[i];
            i++;
        }
        valor_str[i] = '\0';

        int novo_offset = atoi(valor_str);
        offset_umi = novo_offset;

        printf("[DEBUG] Novo offset de umidade: %d%%\n", offset_umi);

        const char *txt = "Offset de umidade atualizado";
        hs->len = snprintf(hs->response, sizeof(hs->response),
                           "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/plain\r\n"
                           "Content-Length: %d\r\n"
                           "Connection: close\r\n"
                           "\r\n"
                           "%s",
                           (int)strlen(txt), txt);
    } 
    else if (strstr(req, "GET /config/offset_pressao/")) {
        char *pos = strstr(req, "/config/offset_pressao/") + strlen("/config/offset_pressao/");
        char valor_str[16] = {0};
        int i = 0;
        
        while (pos[i] != ' ' && pos[i] != '\r' && pos[i] != '\n' && pos[i] != '\0' && i < 15)
        {
            valor_str[i] = pos[i];
            i++;
        }
        valor_str[i] = '\0';

        int novo_offset = atoi(valor_str);
        offset_pressao = novo_offset;

        printf("[DEBUG] Novo offset de pressão: %dkPa\n", offset_pressao);

        const char *txt = "Offset de pressão atualizado";
        hs->len = snprintf(hs->response, sizeof(hs->response),
                           "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/plain\r\n"
                           "Content-Length: %d\r\n"
                           "Connection: close\r\n"
                           "\r\n"
                           "%s",
                           (int)strlen(txt), txt);
    } 
    // === ROTA: Página principal (HTML) ===
    else {
        hs->len = snprintf(hs->response, sizeof(hs->response),
                           "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/html\r\n"
                           "Content-Length: %d\r\n"
                           "Connection: close\r\n"
                           "\r\n"
                           "%s",
                           (int)strlen(HTML_BODY), HTML_BODY);
    }

    // === CONFIGURAÇÃO E ENVIO DA RESPOSTA ===
    tcp_arg(tpcb, hs);
    tcp_sent(tpcb, http_sent);

    tcp_write(tpcb, hs->response, hs->len, TCP_WRITE_FLAG_COPY);
    tcp_output(tpcb);

    pbuf_free(p);
    return ERR_OK;
}