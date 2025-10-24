#include "web.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ========================================================================
// VARIÁVEIS EXTERNAS
// ========================================================================
// Acesso às variáveis globais do arquivo principal weather-station-iot.c
extern float temperatura;
extern float umidade;
extern uint16_t lux;
extern int32_t pressao;
extern int limiteMAX_temp;
extern int limiteMAX_umi;
extern int limiteMAX_lux;
extern int limiteMAX_pressao;
extern int limiteMIN_temp;
extern int limiteMIN_umi;
extern int limiteMIN_lux;
extern int limiteMIN_pressao;
extern int offset_temp;
extern int offset_umi;
extern int offset_lux;
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
    "<title>Centro de Monitoramento Ambiental da Estufa</title><script src='https://cdn.jsdelivr.net/npm/chart.js'></script>"
    "<style>*{margin:0;padding:0;box-sizing:border-box}"
    "body{font-family:'Segoe UI',sans-serif;background:linear-gradient(135deg,#2d5016,#4a7c23,#6b9e3e);color:#333;min-height:100vh}"
    ".ctn{max-width:1200px;margin:auto;padding:20px}.hdr{text-align:center;margin-bottom:30px;color:#fff}"
    ".hdr h1{font-size:2.8rem;margin-bottom:10px;text-shadow:2px 2px 6px rgba(0,0,0,0.4);display:flex;align-items:center;justify-content:center;gap:15px}"
    ".hdr p{font-size:1.2rem;opacity:0.95;font-weight:300}.dash{display:grid;grid-template-columns:repeat(auto-fit,minmax(350px,1fr));gap:20px;margin-bottom:30px}"
    ".cd{background:linear-gradient(145deg,#fff,#f0f7ed);border-radius:20px;padding:25px;box-shadow:0 15px 40px rgba(0,0,0,0.25);transition:all .3s;border:2px solid rgba(106,158,62,0.2)}"
    ".cd:hover{transform:translateY(-8px);box-shadow:0 20px 50px rgba(0,0,0,0.3)}"
    ".cd h3{text-align:center;font-size:1.4rem;margin-bottom:20px;color:#2d5016;font-weight:600}"
    ".val{display:flex;align-items:center;justify-content:center;margin-bottom:20px}"
    ".v{font-size:3rem;font-weight:700;margin-right:10px}.u{font-size:1.3rem;color:#5a7a42;font-weight:500}"
    ".t .v{color:#d97706}.h .v{color:#0284c7}.l .v{color:#f59e0b}.ch{height:220px;position:relative;padding:10px}"
    ".alert{margin-bottom:15px;padding:12px;border-radius:10px;font-weight:600;text-align:center;display:none;box-shadow:0 4px 12px rgba(0,0,0,0.1)}"
    ".alert.show{display:block}.alert.danger{background:#fee2e2;color:#991b1b;border:2px solid #fca5a5}"
    ".alert.success{background:#d1fae5;color:#065f46;border:2px solid #6ee7b7}"
    ".status{font-size:1rem;color:#4a5568;margin-bottom:15px;text-align:center;font-weight:500;background:rgba(255,255,255,0.5);padding:8px;border-radius:8px}"
    ".lim{background:linear-gradient(145deg,#fff,#f0f7ed);border-radius:20px;padding:25px;margin-top:20px;box-shadow:0 15px 40px rgba(0,0,0,0.25);border:2px solid rgba(106,158,62,0.2)}"
    ".cal{background:linear-gradient(145deg,#fff,#f0f7ed);border-radius:20px;padding:25px;margin-top:20px;box-shadow:0 15px 40px rgba(0,0,0,0.25);border:2px solid rgba(106,158,62,0.2)}"
    ".lim h3,.cal h3{color:#2d5016;font-size:1.5rem;margin-bottom:20px;text-align:center;font-weight:600}"
    ".lg{display:grid;grid-template-columns:repeat(auto-fit,minmax(300px,1fr));gap:20px}"
    ".g{border:2px solid #6b9e3e;border-radius:15px;padding:20px;background:rgba(255,255,255,0.7);transition:all .3s}"
    ".g:hover{background:rgba(255,255,255,0.9);transform:scale(1.02)}"
    ".g h4{text-align:center;margin-bottom:18px;color:#2d5016;font-weight:600;font-size:1.2rem}"
    ".inp{display:grid;grid-template-columns:auto 1fr auto;gap:10px;align-items:center;margin-bottom:12px}"
    ".inp label{color:#2d5016;font-weight:600;font-size:0.95rem;white-space:nowrap}"
    ".inp input{width:100%;padding:10px;border:2px solid #6b9e3e;border-radius:8px;transition:all .3s;font-size:1rem}"
    ".inp input:focus{outline:none;border-color:#4a7c23;box-shadow:0 0 0 3px rgba(106,158,62,0.2)}"
    ".inp span{color:#5a7a42;font-weight:500;font-size:0.95rem;min-width:35px;text-align:left}"
    ".btns{text-align:center;margin-top:18px}"
    ".btn{background:#6b9e3e;color:#fff;border:none;padding:12px 24px;border-radius:8px;cursor:pointer;transition:all .3s;font-weight:600;font-size:1rem;box-shadow:0 4px 12px rgba(107,158,62,0.3)}"
    ".btn:hover{background:#4a7c23;transform:translateY(-2px);box-shadow:0 6px 16px rgba(107,158,62,0.4)}"
    ".bt-t{background:#d97706}.bt-t:hover{background:#b45309}"
    ".bt-h{background:#0284c7}.bt-h:hover{background:#0369a1}"
    ".bt-l{background:#f59e0b}.bt-l:hover{background:#d97706}"
    ".bt-cal{background:#7c3aed}.bt-cal:hover{background:#6d28d9}"
    ".icon{font-size:2.5rem;display:inline-block}"
    "@media(max-width:768px){.ctn{padding:10px}.hdr h1{font-size:2rem;flex-direction:column;gap:10px}.v{font-size:2.2rem}.lg{grid-template-columns:1fr}.inp{grid-template-columns:auto 1fr auto;gap:8px}}"
    "</style></head><body>"
    "<div class='ctn'><div class='hdr'><h1><span class='icon'></span>Centro de Monitoramento Ambiental da Estufa<span class='icon'></span></h1><p>Controle Inteligente do Ambiente para Cultivo Ideal</p></div>"
    "<div class='dash'>"
    "<div class='cd t'><h3>🌡️ Temperatura Ambiente</h3>"
    "<div class='alert' id='t-alert'></div>"
    "<div class='status' id='t-status'>Limites: -- a -- °C</div>"
    "<div class='val'><span class='v' id='tv'>--</span><span class='u'>°C</span></div><div class='ch'><canvas id='tc'></canvas></div></div>"
    "<div class='cd h'><h3>💧 Umidade do Ar</h3>"
    "<div class='alert' id='h-alert'></div>"
    "<div class='status' id='h-status'>Limites: -- a -- %</div>"
    "<div class='val'><span class='v' id='hv'>--</span><span class='u'>%</span></div><div class='ch'><canvas id='hc'></canvas></div></div>"
    "<div class='cd l'><h3>☀️ Luminosidade</h3>"
    "<div class='alert' id='l-alert'></div>"
    "<div class='status' id='l-status'>Limites: -- a -- lux</div>"
    "<div class='val'><span class='v' id='lv'>--</span><span class='u'>lux</span></div><div class='ch'><canvas id='lc'></canvas></div></div>"
    "</div>"
    "<div class='cal'><h3>🔧 Calibração dos Sensores (Offset)</h3>"
    "<p style='text-align:center;color:#5a7a42;margin-bottom:20px;font-style:italic'>Ajuste fino para compensar desvios dos sensores</p>"
    "<div class='lg'>"
    "<div class='g'><h4>🌡️ Temperatura</h4>"
    "<div class='inp'><label>Offset:</label><input type='number' id='toff' step='0.1' placeholder='Ex: -3.0'><span>°C</span></div>"
    "<div class='btns'><button class='btn bt-cal' onclick='sto()'>Aplicar Correção</button></div></div>"
    "<div class='g'><h4>💧 Umidade</h4>"
    "<div class='inp'><label>Offset:</label><input type='number' id='hoff' step='0.1' placeholder='Ex: 2.5'><span>%</span></div>"
    "<div class='btns'><button class='btn bt-cal' onclick='sho()'>Aplicar Correção</button></div></div>"
    "<div class='g'><h4>☀️ Luminosidade</h4>"
    "<div class='inp'><label>Offset:</label><input type='number' id='loff' step='1' placeholder='Ex: -50'><span>lux</span></div>"
    "<div class='btns'><button class='btn bt-cal' onclick='slo()'>Aplicar Correção</button></div></div>"
    "</div></div>"
    "<div class='lim'><h3>⚙️ Configuração de Limites Ideais</h3>"
    "<p style='text-align:center;color:#5a7a42;margin-bottom:20px;font-style:italic'>Defina as condições ideais para suas plantas</p>"
    "<div class='lg'>"
    "<div class='g'><h4>🌡️ Temperatura (°C)</h4>"
    "<div class='inp'><label>Mínimo:</label><input type='number' id='tmin' step='0.1' placeholder='Ex: 18.0'><span>°C</span></div>"
    "<div class='inp'><label>Máximo:</label><input type='number' id='tmax' step='0.1' placeholder='Ex: 30.0'><span>°C</span></div>"
    "<div class='btns'><button class='btn bt-t' onclick='stm()'>Definir Min</button> <button class='btn bt-t' onclick='stx()'>Definir Max</button></div></div>"
    "<div class='g'><h4>💧 Umidade (%)</h4>"
    "<div class='inp'><label>Mínimo:</label><input type='number' id='hmin' step='0.1' min='0' max='100' placeholder='Ex: 50.0'><span>%</span></div>"
    "<div class='inp'><label>Máximo:</label><input type='number' id='hmax' step='0.1' min='0' max='100' placeholder='Ex: 80.0'><span>%</span></div>"
    "<div class='btns'><button class='btn bt-h' onclick='shm()'>Definir Min</button> <button class='btn bt-h' onclick='shx()'>Definir Max</button></div></div>"
    "<div class='g'><h4>☀️ Luminosidade (lux)</h4>"
    "<div class='inp'><label>Mínimo:</label><input type='number' id='lmin' step='100' placeholder='Ex: 1000'><span>lux</span></div>"
    "<div class='inp'><label>Máximo:</label><input type='number' id='lmax' step='100' placeholder='Ex: 10000'><span>lux</span></div>"
    "<div class='btns'><button class='btn bt-l' onclick='slm()'>Definir Min</button> <button class='btn bt-l' onclick='slx()'>Definir Max</button></div></div>"
    "</div></div></div>"
    "<script>"
    "let td=[],hd=[],ld=[],tl=[],max=20,lm={t:{mi:null,ma:null},h:{mi:null,ma:null},l:{mi:null,ma:null}};"
    "const tc=new Chart(document.getElementById('tc'),{type:'line',data:{labels:tl,datasets:[{label:'Temperatura (°C)',data:td,borderColor:'#d97706',backgroundColor:'rgba(217,119,6,0.15)',borderWidth:3,fill:true,tension:0.4,pointRadius:4,pointHoverRadius:6}]},options:{responsive:true,maintainAspectRatio:false,scales:{y:{beginAtZero:false,grid:{color:'rgba(106,158,62,0.1)'}},x:{grid:{color:'rgba(106,158,62,0.1)'}}},plugins:{legend:{display:false}}}});"
    "const hc=new Chart(document.getElementById('hc'),{type:'line',data:{labels:tl,datasets:[{label:'Umidade (%)',data:hd,borderColor:'#0284c7',backgroundColor:'rgba(2,132,199,0.15)',borderWidth:3,fill:true,tension:0.4,pointRadius:4,pointHoverRadius:6}]},options:{responsive:true,maintainAspectRatio:false,scales:{y:{beginAtZero:true,max:100,grid:{color:'rgba(106,158,62,0.1)'}},x:{grid:{color:'rgba(106,158,62,0.1)'}}},plugins:{legend:{display:false}}}});"
    "const lc=new Chart(document.getElementById('lc'),{type:'line',data:{labels:tl,datasets:[{label:'Luminosidade (lux)',data:ld,borderColor:'#f59e0b',backgroundColor:'rgba(245,158,11,0.15)',borderWidth:3,fill:true,tension:0.4,pointRadius:4,pointHoverRadius:6}]},options:{responsive:true,maintainAspectRatio:false,scales:{y:{beginAtZero:true,grid:{color:'rgba(106,158,62,0.1)'}},x:{grid:{color:'rgba(106,158,62,0.1)'}}},plugins:{legend:{display:false}}}});"
    "function chkLim(v,mi,ma,type,unit){"
    "  const alert=document.getElementById(type+'-alert');"
    "  const status=document.getElementById(type+'-status');"
    "  const minTxt=mi!==null?mi:'--';"
    "  const maxTxt=ma!==null?ma:'--';"
    "  status.textContent='Limites: '+minTxt+' a '+maxTxt+' '+unit;"
    "  if(mi!==null&&v<mi){"
    "    alert.textContent='⚠️ ATENÇÃO: Valor abaixo do ideal ('+mi+unit+')';"
    "    alert.className='alert danger show';"
    "  }else if(ma!==null&&v>ma){"
    "    alert.textContent='⚠️ ATENÇÃO: Valor acima do ideal ('+ma+unit+')';"
    "    alert.className='alert danger show';"
    "  }else if(mi!==null||ma!==null){"
    "    alert.textContent='✅ Condições ideais para cultivo';"
    "    alert.className='alert success show';"
    "  }else{"
    "    alert.className='alert';"
    "  }"
    "}"
    "function sto(){"
    "  let v=parseFloat(document.getElementById('toff').value);"
    "  if(isNaN(v)||v<-50||v>50){alert('Offset de temperatura inválido (-50 a +50 °C)');return;}"
    "  fetch('/config/offset_temp/'+v.toFixed(1)).then(()=>alert('✅ Correção de temperatura aplicada: '+v+'°C')).catch(()=>alert('❌ Erro ao aplicar correção'));"
    "}"
    "function sho(){"
    "  let v=parseFloat(document.getElementById('hoff').value);"
    "  if(isNaN(v)||v<-50||v>50){alert('Offset de umidade inválido (-50 a +50 %)');return;}"
    "  let atual = parseFloat(document.getElementById('hv').textContent);"
    "  if(!isNaN(atual) && (atual+v<0 || atual+v>100)){alert('Correção resultaria em valor fora do intervalo 0% a 100%');return;}"
    "  fetch('/config/offset_umi/'+v.toFixed(1)).then(()=>alert('✅ Correção de umidade aplicada: '+v+'%')).catch(()=>alert('❌ Erro ao aplicar correção'));"
    "}"
    "function slo(){"
    "  let v=parseInt(document.getElementById('loff').value);"
    "  if(isNaN(v)||v<-10000||v>10000){alert('Offset de luminosidade inválido (-10000 a +10000 lux)');return;}"
    "  let atual = parseInt(document.getElementById('lv').textContent);"
    "  if(!isNaN(atual) && atual+v<0){alert('Correção resultaria em valor negativo');return;}"
    "  fetch('/config/offset_lux/'+v).then(()=>alert('✅ Correção de luminosidade aplicada: '+v+' lux')).catch(()=>alert('❌ Erro ao aplicar correção'));"
    "}"
    "function stm(){let v=parseFloat(document.getElementById('tmin').value);if(isNaN(v)||v<-50||v>100||lm.t.ma!==null&&v>=lm.t.ma){alert('Valor inválido');return;}fetch('/config/limiteMIN_temp/'+Math.round(v)).then(()=>{lm.t.mi=Math.round(v);alert('✅ Temperatura mínima definida: '+v+'°C');}).catch(()=>alert('❌ Erro'));}"
    "function stx(){let v=parseFloat(document.getElementById('tmax').value);if(isNaN(v)||v<-50||v>100||lm.t.mi!==null&&v<=lm.t.mi){alert('Valor inválido');return;}fetch('/config/limiteMAX_temp/'+Math.round(v)).then(()=>{lm.t.ma=Math.round(v);alert('✅ Temperatura máxima definida: '+v+'°C');}).catch(()=>alert('❌ Erro'));}"
    "function shm(){let v=parseFloat(document.getElementById('hmin').value);if(isNaN(v)||v<0||v>100||lm.h.ma!==null&&v>=lm.h.ma){alert('Valor inválido');return;}fetch('/config/limiteMIN_umi/'+Math.round(v)).then(()=>{lm.h.mi=Math.round(v);alert('✅ Umidade mínima definida: '+v+'%');}).catch(()=>alert('❌ Erro'));}"
    "function shx(){let v=parseFloat(document.getElementById('hmax').value);if(isNaN(v)||v<0||v>100||lm.h.mi!==null&&v<=lm.h.mi){alert('Valor inválido');return;}fetch('/config/limiteMAX_umi/'+Math.round(v)).then(()=>{lm.h.ma=Math.round(v);alert('✅ Umidade máxima definida: '+v+'%');}).catch(()=>alert('❌ Erro'));}"
    "function slm(){let v=parseInt(document.getElementById('lmin').value);if(isNaN(v)||v<0||v>100000||lm.l.ma!==null&&v>=lm.l.ma){alert('Valor inválido');return;}fetch('/config/limiteMIN_lux/'+v).then(()=>{lm.l.mi=v;alert('✅ Luminosidade mínima definida: '+v+' lux');}).catch(()=>alert('❌ Erro'));}"
    "function slx(){let v=parseInt(document.getElementById('lmax').value);if(isNaN(v)||v<0||v>100000||lm.l.mi!==null&&v<=lm.l.mi){alert('Valor inválido');return;}fetch('/config/limiteMAX_lux/'+v).then(()=>{lm.l.ma=v;alert('✅ Luminosidade máxima definida: '+v+' lux');}).catch(()=>alert('❌ Erro'));}"
    "function upd(){fetch('/dados').then(r=>r.json()).then(d=>{"
    "  const t=new Date().toLocaleTimeString();"
    "  document.getElementById('tv').textContent=d.temperatura.toFixed(1);"
    "  document.getElementById('hv').textContent=d.umidade.toFixed(1);"
    "  document.getElementById('lv').textContent=d.luminosidade;"
    "  "
    "  if(d.limiteMIN_temp !== undefined && d.limiteMIN_temp !== -2147483648) lm.t.mi = d.limiteMIN_temp; else lm.t.mi = null;"
    "  if(d.limiteMAX_temp !== undefined && d.limiteMAX_temp !== 2147483647) lm.t.ma = d.limiteMAX_temp; else lm.t.ma = null;"
    "  if(d.limiteMIN_umi !== undefined && d.limiteMIN_umi !== -2147483648) lm.h.mi = d.limiteMIN_umi; else lm.h.mi = null;"
    "  if(d.limiteMAX_umi !== undefined && d.limiteMAX_umi !== 2147483647) lm.h.ma = d.limiteMAX_umi; else lm.h.ma = null;"
    "  if(d.limiteMIN_lux !== undefined && d.limiteMIN_lux !== -2147483648) lm.l.mi = d.limiteMIN_lux; else lm.l.mi = null;"
    "  if(d.limiteMAX_lux !== undefined && d.limiteMAX_lux !== 2147483647) lm.l.ma = d.limiteMAX_lux; else lm.l.ma = null;"
    "  "
    "  chkLim(d.temperatura,lm.t.mi,lm.t.ma,'t','°C');"
    "  chkLim(d.umidade,lm.h.mi,lm.h.ma,'h','%');"
    "  chkLim(d.luminosidade,lm.l.mi,lm.l.ma,'l','lux');"
    "  tl.push(t);td.push(d.temperatura);hd.push(d.umidade);ld.push(d.luminosidade);"
    "  if(tl.length>max){tl.shift();td.shift();hd.shift();ld.shift();}"
    "  tc.update();hc.update();lc.update();"
    "}).catch(e=>console.error('Erro:',e));}"
    "upd();setInterval(upd,1500);"
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
                            "{\"temperatura\":%.2f,\"umidade\":%.2f,\"luminosidade\":%d,"
                            "\"limiteMIN_temp\":%d,\"limiteMAX_temp\":%d,"
                            "\"limiteMIN_umi\":%d,\"limiteMAX_umi\":%d,"
                            "\"limiteMIN_lux\":%d,\"limiteMAX_lux\":%d}\r\n",
                            temperatura, umidade, lux,
                            limiteMIN_temp, limiteMAX_temp,
                            limiteMIN_umi, limiteMAX_umi,
                            limiteMIN_lux, limiteMAX_lux);

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
    else if (strstr(req, "GET /config/limiteMAX_lux/")) {
        char *pos = strstr(req, "/config/limiteMAX_lux/") + strlen("/config/limiteMAX_lux/");
        char valor_str[16] = {0};
        int i = 0;
        
        while (pos[i] != ' ' && pos[i] != '\r' && pos[i] != '\n' && pos[i] != '\0' && i < 15)
        {
            valor_str[i] = pos[i];
            i++;
        }
        valor_str[i] = '\0';

        int novo_limite = atoi(valor_str);
        limiteMAX_lux = novo_limite;

        printf("[DEBUG] Novo limite máximo de luminosidade: %d lux\n", limiteMAX_lux);

        const char *txt = "Limite máximo de luminosidade atualizado";
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
    else if (strstr(req, "GET /config/limiteMIN_lux/")) {
        char *pos = strstr(req, "/config/limiteMIN_lux/") + strlen("/config/limiteMIN_lux/");
        char valor_str[16] = {0};
        int i = 0;
        
        while (pos[i] != ' ' && pos[i] != '\r' && pos[i] != '\n' && pos[i] != '\0' && i < 15)
        {
            valor_str[i] = pos[i];
            i++;
        }
        valor_str[i] = '\0';

        int novo_limite = atoi(valor_str);
        limiteMIN_lux = novo_limite;

        printf("[DEBUG] Novo limite mínimo de luminosidade: %d lux\n", limiteMIN_lux);

        const char *txt = "Limite mínimo de luminosidade atualizado";
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
    else if (strstr(req, "GET /config/offset_lux/")) {
        char *pos = strstr(req, "/config/offset_lux/") + strlen("/config/offset_lux/");
        char valor_str[16] = {0};
        int i = 0;
        
        while (pos[i] != ' ' && pos[i] != '\r' && pos[i] != '\n' && pos[i] != '\0' && i < 15)
        {
            valor_str[i] = pos[i];
            i++;
        }
        valor_str[i] = '\0';

        int novo_offset = atoi(valor_str);
        offset_lux = novo_offset;

        printf("[DEBUG] Novo offset de luminosidade: %d lux\n", offset_lux);

        const char *txt = "Offset de luminosidade atualizado";
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