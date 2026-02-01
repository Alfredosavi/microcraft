# Microcraft — A Minecraft Server (Probably the Worst One)

Projeto criado **apenas por desafio e diversão**: a ideia é pegar um MCU com recursos extremamente limitados de RAM e transformar em um mini servidor Minecraft, explorando o protocolo do jogo de forma minimalista.  
Não é um produto, não é prático, é um experimento técnico.

O repositório contém a implementação da API do Minecraft **1.12.2 (protocol 340)** em **duas versões**:

- **PC:** usada para testes rápidos, validação do protocolo e depuração.
- **MCU (MSP430G2553 usando Code Composer Studio – CCS):** versão específica para microcontrolador, focada em uso mínimo de RAM.

O servidor é implementado em **C**, suportando conexão básica do cliente e o início do carregamento do mundo.

---

## ✅ Features Implementadas

- Servidor TCP funcional  
- Implementação do Handshake do protocolo  
- Resposta de Status/Ping (lista de servidores)  
- MOTD exibido na lista de servidores  
- Login em modo offline  
- Envio de pacote Login Success  
- Transição para estado PLAY  
- Envio de pacote Join Game  
- Entrada inicial do jogador no mundo  
- Cliente chega à tela de carregamento de terreno  

---

## 🚧 TODO

Falta praticamente **toda a API do Minecraft**.  
A lista abaixo contém apenas alguns exemplos do que ainda precisa ser implementado:

- Implementar KeepAlive  
- Enviar chunk mínimo do mundo  
- Implementar movimentação básica do jogador  
- Chat básico  

---

## 🧠 MCU Utilizado — MSP430G2553

A versão embarcada do servidor roda no **MSP430G2553**, um microcontrolador propositalmente limitado:

- Arquitetura: MSP430 (16-bit)  
- Clock máximo: 16 MHz  
- Flash: **16 KB**  
- RAM: **512 bytes**  
- Alguns periféricos disponíveis:
  - SPI (usado na interface entre o MCU e o W5500)
  - UART (usado para debug/log, se habilitado)

> ⚠️ **Aviso:** Rodar qualquer coisa parecida com um servidor Minecraft nesse chip é **absurdo por definição**, e exatamente por isso faz parte do desafio.

### 🤷 Por que o MSP430G2553?

O MSP430G2553 não foi escolhido por mérito técnico, custo-benefício ou por ser a melhor opção para este projeto.  
Ele foi escolhido porque estava **jogado no fundo da minha gaveta**, acumulando poeira, e até então só tinha servido para projetos do tipo *pisca LED* ou algo igualmente pouco inspirador.

Para que ele finalmente fosse **útil para alguma coisa**, a decisão foi simples — e questionável:  
rodar nele um servidor Minecraft.

Então, se você também tem esse LaunchPad esquecido por aí — talvez comprado em alguma matéria de microcontroladores da faculdade — este pode ser um projeto divertido para rodar, quebrar, mexer e continuar evoluindo.

Talvez não seja útil, nem rápido, nem sensato.  
Mas é uma chance perfeita de usar esse chip para algo **mais interessante do que piscar um LED** (e passar um pouco de raiva no processo).

---

## 🔌 Esquema Elétrico e Interface de Rede

Como o MSP430G2553 **não possui Ethernet**, a comunicação de rede é feita usando o **W5500** (Ethernet SPI da Wiznet).

### Componentes principais

- MSP430G2553  
- W5500 (Ethernet via SPI)  
- Fonte externa 3.3V para o W5500  

### Interface MSP430 ↔ W5500

- SPI (MOSI, MISO, SCK)  
- CS dedicado  
- RESET do W5500 controlado por GPIO  
- INT opcional (dependendo da implementação)  

### 📌 Mapeamento de Pinos — W5500 ↔ MCU (MSP430G2553)
<!-- TODO -->
| W5500 | Função            | MCU (MSP430G2553) | Observação                              |
|------:|-------------------|-------------------|-----------------------------------------|
| MOSI  | SPI MOSI          | USCI_Bx MOSI      | Dados do MCU → W5500                    |
| MISO  | SPI MISO          | USCI_Bx MISO      | Dados do W5500 → MCU                    |
| SCK   | SPI Clock         | USCI_Bx SCLK      | Clock do barramento SPI                 |
| CS    | Chip Select       | GPIO              | Controlado por software                 |
| RST   | Reset             | GPIO              | Reset do W5500                          |
| INT   | Interrupt         | GPIO (opcional)   | Pode ser não utilizado inicialmente     |
| 3V3   | Alimentação       | 3.3V              | Não é tolerante a 5V                    |
| GND   | Terra             | GND               | Referência comum                        |

### ⚠️ Observação Importante

O **W5500 foi escolhido propositalmente** porque ele implementa **toda a stack TCP/IP em hardware**.  
Isso permite que o MCU:

- Não precise implementar TCP/IP em software  
- Economize uma quantidade significativa de RAM  
- Trabalhe apenas com sockets, buffers e estados simples  
- O W5500 **opera exclusivamente em 3.3V**
- Utilize **jumpers de boa qualidade e o mais curtos possível**

> Durante os testes, foram observados **erros de bytes na comunicação SPI** causados por jumpers "longos" e/ou de baixa qualidade.

---

## ▶️ Build e Execução (Versão PC)

A versão para PC existe exclusivamente para **testes, validação do protocolo e depuração**.  
Ela facilita o desenvolvimento antes de portar qualquer funcionalidade para o MCU.

### Requisitos

- GCC
- Make
- Sistema Linux ou WSL

### Build

Na pasta da implementação para PC:

```bash
make
```

O executável será gerado em:

```bash
./build/server
```

### Execução

Execute o servidor com:

```bash
./build/server
```

Por padrão, o servidor escuta na **porta TCP padrão do Minecraft (25565)**, definida diretamente no código.

⚠️ Esta versão não é um servidor Minecraft completo.
Ela implementa apenas partes mínimas do protocolo.

---

## 🔗 Links Úteis
<!-- TODO -->
<!-- A implementação da API do Minecraft (versão **340**, protocolo **1.12.2**) foi baseada na documentação disponível em https://XXXXXX. -->