# Sistema de Estacionamento Inteligente - IoT

**SENAI • Automação e Internet das Coisas**  
**Turma TDESS 2025 1 V1**  
**Integrantes:** Vinícius Philippi Lessa e Gabriel Henrique  
**Docente:** Sergio Luiz da Silveira  
**Unidade Curricular:** Internet das Coisas • Avaliação Prática

## Visão Geral

Protótipo de estacionamento automatizado que monitora a entrada e saída de veículos por meio de sensores ultrassônicos, controla a disponibilidade de vagas, aciona uma barreira (servo) e informa o status em tempo real no display LCD.

O sistema integra sensoriamento, lógica de controle e atuadores, atendendo às capacidades C1 e C2 da avaliação prática.

## Funcionamento

1. Sensor de **entrada** detecta a aproximação do veículo.
2. Se houver vagas disponíveis, o servo abre a cancela, o buzzer emite aviso e a contagem é atualizada.
3. Sensor de **saída** detecta a saída do veículo e libera uma vaga.
4. O LCD 16x2 I2C mostra as vagas restantes ou a mensagem "ESTACIONAMENTO CHEIO".
5. Quando a capacidade máxima é atingida, a entrada permanece bloqueada.

## Componentes Principais

- Arduino Uno R3
- 2x Sensor Ultrassônico HC-SR04
- LCD 16x2 I2C (endereço 0x27)
- Micro Servo (barreira)
- Piezo Buzzer
- Protoboard e jumpers

Veja a lista completa em `componentes/lista-componentes.csv` e as conexões em `componentes/conexoes.md`.

## Estrutura do Repositório

```
estacionamento-inteligente-iot/
├── documentacao/
│   ├── documentacao_sa_iot_estacionamento_inteligente.pdf
│   └── esquematico.pdf
│   └── slide_sa_iot_estacionamento_inteligente.pdf
├── codigo/
│   └── estacionamento.ino
├── componentes/
│   ├── lista-componentes.csv
│   └── conexoes.md
├── imagens/
│   ├── prototipo-projeto.png
├── simulacao/
│   └── link-tinkercad.md
├── README.md
└── LICENSE
```

## Como Usar

1. Monte o circuito conforme `componentes/conexoes.md`.
2. Abra `codigo/estacionamento.ino` no Arduino IDE.
3. Instale as bibliotecas: **Wire** (nativa), **Servo** (nativa) e **LiquidCrystal_I2C**.
4. Faça o upload para o Arduino Uno.
5. Ajuste a capacidade máxima (`TOTAL_SPOTS`) se necessário.
6. Teste no simulador Tinkercad (link em `simulacao/`).

## Versionamento

O projeto utiliza Git para controle de versão. Cada alteração relevante deve ser commitada com mensagem descritiva das mudanças.

## Documentação

A documentação técnica completa, motivação, aplicação real e atendimento aos critérios de avaliação estão nos PDFs da pasta `documentacao/`.

## Licença

Projeto educacional. Veja o arquivo LICENSE.
