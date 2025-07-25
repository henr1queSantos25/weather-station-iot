# Weather Station IoT: Estação Meteorológica Inteligente

O **Weather Station IoT** é uma estação meteorológica embarcada e conectada desenvolvida para a plataforma **BitDogLab** com o microcontrolador **Raspberry Pi Pico W (RP2040)**. O sistema realiza medição contínua e precisa de **temperatura**, **umidade relativa do ar** e **pressão atmosférica**, oferecendo dupla interface de visualização: local através de display OLED e remota via interface web responsiva acessível por Wi-Fi. O projeto integra recursos de alerta visual e sonoro, controle de limites personalizáveis e calibração de sensores, criando uma solução completa para monitoramento ambiental em tempo real.

---

## Vídeo de Demonstração

[Assista à demonstração do Weather Station IoT](https://drive.google.com/file/d/1c2ily7UwW7wz88DoD5sRtNpfBQYznibB/view?usp=sharing)

---

## Funcionalidades Principais

- **Monitoramento Ambiental Contínuo**: Medição precisa de temperatura, umidade e pressão atmosférica utilizando sensores digitais de alta qualidade.
- **Dupla Interface de Visualização**:
  - **Local**: Display OLED SSD1306 com alternância entre modo sensores e modo Wi-Fi
  - **Remota**: Interface web responsiva acessível via Wi-Fi com gráficos dinâmicos em tempo real
- **Interface Web Completa**:
  - Dashboard visual com gráficos em tempo real usando Chart.js
  - Configuração de limites mínimos e máximos personalizáveis para cada variável
  - Sistema de calibração com offsets ajustáveis para correção de desvios dos sensores
  - Atualização automática via AJAX a cada segundo
- **Sistema de Alertas Inteligente**:
  - Alertas visuais através de LED RGB e matriz de LEDs 5x5 com padrões coloridos
  - Notificações sonoras via buzzer PWM para situações críticas
  - Indicações diferenciadas para limites máximos e mínimos ultrapassados
- **Servidor HTTP Embarcado**: Implementado diretamente no firmware usando stack TCP/IP LWIP
- **Controle por Botão**: Alternância entre modos de exibição do display com debounce por interrupção
- **Arquitetura Não-Bloqueante**: Sistema responsivo com ciclos curtos que garantem estabilidade e sincronização

---

## Tecnologias Utilizadas

- **Linguagem de Programação**: C
- **Microcontrolador**: Raspberry Pi Pico W (RP2040 + CYW43439)
- **Conectividade**: Wi-Fi embutido (modo Station)
- **Protocolo Web**: Servidor HTTP com stack LWIP
- **Sensores Utilizados**:
  - **AHT20**: Sensor digital I2C para temperatura e umidade relativa
  - **BMP280**: Sensor digital I2C para pressão atmosférica e temperatura adicional
- **Componentes de Interface**:
  - Display OLED SSD1306 128x64 (I2C) para visualização local
  - LED RGB individual para indicação de estado
  - Matriz de LEDs 5x5 RGB para alertas visuais
  - Buzzer com controle PWM para alertas sonoros
  - Push button com tratamento por interrupção e debounce
- **Tecnologias Web**:
  - HTML5, CSS3 e JavaScript responsivo
  - Chart.js para gráficos dinâmicos
  - AJAX para atualização em tempo real
  - Design mobile-first com gradientes modernos
- **Bibliotecas**:
  - Pico SDK para acesso ao hardware (GPIO, I2C, PWM)
  - CYW43 para controle do módulo Wi-Fi
  - LWIP para implementação do servidor HTTP
  - Bibliotecas personalizadas para controle dos periféricos

---

## Interface Web - Recursos

### Dashboard Principal
- **Cartões Visuais**: Exibição de temperatura, umidade e pressão com cores diferenciadas
- **Gráficos em Tempo Real**: Histórico visual dos últimos 20 pontos de medição
- **Indicadores de Status**: Alertas coloridos quando valores ultrapassam limites configurados
- **Design Responsivo**: Adaptação automática para desktop, tablet e smartphone

### Sistema de Configuração de Limites
- **Controles Individuais**: Definição de valores mínimos e máximos para cada sensor
- **Validação em Tempo Real**: Verificação de consistência dos limites configurados
- **Feedback Imediato**: Confirmação visual das configurações aplicadas
- **Persistência**: Valores mantidos na memória durante a sessão

### Sistema de Calibração (Offsets)
- **Correção Individual**: Aplicação de offsets para cada sensor independentemente
- **Validação de Limites**: Verificação para evitar valores fora das faixas operacionais
- **Interface Intuitiva**: Campos dedicados com validação de entrada
- **Aplicação Instantânea**: Efeito imediato nas leituras exibidas

### Rotas HTTP Implementadas
- **`/dados`**: Retorna JSON com valores atuais e configurações
- **`/config/limiteMIN_temp/[valor]`**: Define limite mínimo de temperatura
- **`/config/limiteMAX_temp/[valor]`**: Define limite máximo de temperatura
- **`/config/limiteMIN_umi/[valor]`**: Define limite mínimo de umidade
- **`/config/limiteMAX_umi/[valor]`**: Define limite máximo de umidade
- **`/config/limiteMIN_pressao/[valor]`**: Define limite mínimo de pressão
- **`/config/limiteMAX_pressao/[valor]`**: Define limite máximo de pressão
- **`/config/offset_temp/[valor]`**: Aplica offset de temperatura
- **`/config/offset_umi/[valor]`**: Aplica offset de umidade
- **`/config/offset_pressao/[valor]`**: Aplica offset de pressão

---

## Como Funciona

### Leitura e Processamento de Dados
- **Sensores Duplos**: O sistema utiliza dois sensores para maior confiabilidade:
  - **AHT20**: Fornece temperatura e umidade relativa do ar
  - **BMP280**: Oferece pressão atmosférica e segunda leitura de temperatura
- **Média de Temperatura**: Calcula média entre as duas leituras de temperatura para maior precisão
- **Aplicação de Offsets**: Correções de calibração são aplicadas antes da exibição
- **Filtragem**: Umidade é limitada a 100% para evitar valores irreais

### Sistema de Alertas por Estados
- **Estado Normal**:
  - Buzzer: Desligado
  - Matriz LED: Apagada
  - LED RGB: Verde piscando
  - Display: Valores normais

- **Alerta por Limite Máximo Ultrapassado**:
  - Buzzer: Alarme PWM ativo
  - Matriz LED: "X" vermelho
  - LED RGB: Vermelho piscando
  - Interface Web: Alerta "ACIMA do limite máximo"

- **Alerta por Limite Mínimo Ultrapassado**:
  - Buzzer: Alarme PWM ativo
  - Matriz LED: "X" amarelo
  - LED RGB: Vermelho e verde alternados
  - Interface Web: Alerta "ABAIXO do limite mínimo"

### Modos de Exibição do Display OLED
- **Modo Sensores** (botão pressionado):
  - Temperatura do BMP280 e AHT20
  - Valor da umidade relativa
  - Pressão atmosférica em kPa
  
- **Modo Wi-Fi** (padrão):
  - Status da conexão Wi-Fi
  - Endereço IP obtido na rede
  - Indicação de conectividade

### Arquitetura do Servidor Web
- **Servidor HTTP Embarcado**: Implementado diretamente no firmware
- **Processamento Assíncrono**: Não bloqueia o loop principal de medições
- **Rotas RESTful**: URLs semânticas para configuração via GET
- **Resposta JSON**: Dados estruturados para fácil processamento pelo frontend
- **Arquivo HTML Embutido**: Interface completa armazenada no firmware

---

## Configuração do Hardware

| Componente | Interface | Pinos do RP2040 | Função |
|------------|-----------|----------------|--------|
| Sensores AHT20 e BMP280 | I2C0 | GP0 (SDA), GP1 (SCL) | Medição de temperatura, umidade e pressão |
| Display OLED SSD1306 | I2C1 | GP14 (SDA), GP15 (SCL) | Exibição local de dados e status Wi-Fi |
| LED RGB | GPIO | GP11 (Verde), GP12 (Azul), GP13 (Vermelho) | Indicação visual de estado do sistema |
| Matriz de LEDs 5x5 | PIO | Controlada via PIO | Alertas visuais com padrões coloridos |
| Buzzer | PWM | GP10 | Alertas sonoros para situações críticas |
| Push Button | GPIO com IRQ | GP5 | Alternância entre modos do display |
| Wi-Fi (CYW43439) | Integrado ao Pico W | - | Comunicação web via Wi-Fi |

### Especificações dos Sensores
- **AHT20**:
  - Temperatura: -40°C a +85°C (±0.3°C)
  - Umidade: 0% a 100% RH (±2% RH)
  - Interface: I2C (0x38)
  
- **BMP280**:
  - Pressão: 300-1100 hPa (±1 hPa)
  - Temperatura: -40°C a +85°C (±1°C)
  - Interface: I2C (0x76)

---

## Configuração do Sistema

### Credenciais Wi-Fi
No arquivo `weather-station-iot.c`, configure:

```c
#define WIFI_SSID "SEU_SSID_WIFI"
#define WIFI_PASS "SUA_SENHA_WIFI"
```

### Acesso à Interface Web
1. Compile e grave o firmware no Raspberry Pi Pico W
2. Observe o endereço IP exibido no display OLED
3. Acesse `http://[IP_DO_DISPOSITIVO]` em qualquer navegador na mesma rede
4. A interface web será carregada automaticamente

### Configuração Inicial de Limites
Valores padrão do sistema:
- **Temperatura**: -2147483648°C a 2147483647°C (sem limite)
- **Umidade**: -2147483648% a 2147483647% (sem limite)
- **Pressão**: -2147483648 kPa a 2147483647 kPa (sem limite)

### Exemplos de Configuração via Interface Web
- **Ambiente Residencial**:
  - Temperatura: 18°C a 28°C
  - Umidade: 40% a 70%
  - Pressão: 95 kPa a 105 kPa

- **Ambiente Laboratorial**:
  - Temperatura: 20°C a 25°C
  - Umidade: 45% a 65%
  - Pressão: 100 kPa a 102 kPa

---

## Estrutura do Repositório

- **`weather-station-iot.c`**: Código-fonte principal com lógica de controle e inicialização
- **`lib/web/`**: Módulo da interface web
  - **`web.h` e `web.c`**: Implementação do servidor HTTP e rotas
  - **`index.html`**: Interface web responsiva completa embutida
- **`lib/aht20/`**: Biblioteca para sensor AHT20
  - **`aht20.h` e `aht20.c`**: Controle e leitura do sensor de temperatura e umidade
- **`lib/bmp280/`**: Biblioteca para sensor BMP280
  - **`bmp280.h` e `bmp280.c`**: Controle e leitura do sensor de pressão
- **`lib/ssd1306/`**: Biblioteca para display OLED
  - **`ssd1306.h` e `ssd1306.c`**: Controle do display com funções gráficas
- **`lib/button/`**: Sistema de controle de botões
  - **`button.h` e `button.c`**: Tratamento de interrupções e debounce
- **`lib/rgb/`**: Controle do LED RGB
  - **`rgb.h` e `rgb.c`**: Funções para controle de cores e padrões de pisca
- **`lib/ws2812/`**: Controle da matriz de LEDs 5x5
  - **`ws2812.h` e `ws2812.c`**: Implementação via PIO para padrões visuais
- **`lib/buzzer/`**: Sistema de alertas sonoros
  - **`buzzer.h` e `buzzer.c`**: Controle PWM para geração de sons
- **`CMakeLists.txt`**: Configuração do sistema de build
- **`README.md`**: Documentação completa do projeto

---

## Fluxo de Operação

1. **Inicialização do Sistema**:
   - Configuração de todos os periféricos (I2C, GPIO, PWM, PIO)
   - Reset e inicialização dos sensores AHT20 e BMP280
   - Configuração do display OLED e periféricos de alerta
   - Estabelecimento da conexão Wi-Fi

2. **Inicialização do Servidor Web**:
   - Configuração do servidor HTTP na porta 80
   - Registro das rotas de dados e configuração
   - Disponibilização da interface HTML embutida

3. **Loop Principal de Operação**:
   - **Leitura dos Sensores**: Coleta dados do BMP280 e AHT20 a cada 200ms
   - **Processamento**: Cálculo de médias, aplicação de offsets e validações
   - **Verificação de Limites**: Comparação com limites configurados
   - **Ativação de Alertas**: Controle de LED RGB, matriz LED e buzzer conforme estado
   - **Atualização de Interfaces**: 
     - Display OLED com dados locais ou status Wi-Fi
     - Servidor web com dados JSON atualizados
   - **Processamento de Requisições**: Atendimento às configurações via web

4. **Tratamento de Eventos**:
   - **Interrupção do Botão**: Alternância entre modos do display com debounce
   - **Requisições HTTP**: Processamento de comandos de configuração
   - **Polling da Rede**: Manutenção da conectividade Wi-Fi

---

## Conceitos Aplicados

- **Sistemas Embarcados IoT**: Integração de sensores, atuadores e conectividade
- **Servidor HTTP Embarcado**: Implementação de web server diretamente no microcontrolador
- **Interface Web Responsiva**: Design adaptativo para múltiplos dispositivos
- **Comunicação I2C**: Protocolo de comunicação digital com múltiplos dispositivos
- **Programação Orientada a Eventos**: Uso de interrupções para tratamento de botões
- **Processamento de Sinais**: Filtragem e média de leituras de sensores
- **Controle PWM**: Modulação por largura de pulso para buzzer e LEDs
- **Programable IO (PIO)**: Controle dedicado para matriz de LEDs WS2812
- **Stack TCP/IP**: Implementação de protocolos de rede em sistema embarcado
- **AJAX e JSON**: Comunicação assíncrona para atualização em tempo real
- **Arquitetura Modular**: Separação de responsabilidades em bibliotecas
- **Calibração de Sensores**: Técnicas de correção e ajuste de precisão
- **Interface Humano-Máquina (HMI)**: Design de interfaces intuitivas
- **Sistemas de Alerta Multi-Modal**: Combinação de estímulos visuais e auditivos

---

## Objetivos Alcançados

- **Monitoramento Ambiental Profissional**: Sistema completo para medição precisa de parâmetros meteorológicos
- **Interface Web Moderna**: Dashboard responsivo com gráficos dinâmicos e controles intuitivos
- **Sistema de Alertas Inteligente**: Notificações multi-modais para situações críticas
- **Calibração Personalizada**: Sistema flexível de correção e ajuste de sensores
- **Configuração Remota**: Controle completo via interface web sem necessidade de recompilação
- **Visualização Local e Remota**: Dupla interface para máxima versatilidade de uso
- **Arquitetura Escalável**: Código modular preparado para expansões futuras
- **Conectividade IoT**: Integração completa com redes Wi-Fi domésticas
- **Precisão e Confiabilidade**: Uso de sensores duplos com média para maior precisão
- **Experiência de Usuário**: Interface intuitiva com feedback imediato e design moderno
- **Aplicabilidade Real**: Solução prática para monitoramento em residências, laboratórios, estufas e ambientes controlados
- **Sustentabilidade**: Sistema de baixo consumo energético adequado para operação contínua

---

## Desenvolvido por

Henrique Oliveira dos Santos  

[![LinkedIn](https://img.shields.io/badge/LinkedIn-0A66C2?style=for-the-badge&logo=linkedin&logoColor=white)](https://www.linkedin.com/in/dev-henriqueo-santos/)
