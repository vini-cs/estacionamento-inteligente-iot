# Conexões do Circuito - Estacionamento Inteligente

## Pinagem

| Componente              | Pino Arduino | Observação          |
|-------------------------|--------------|---------------------|
| Sensor Entrada TRIG     | D4           | Saída               |
| Sensor Entrada ECHO     | D3           | Entrada             |
| Sensor Saída TRIG       | D11          | Saída               |
| Sensor Saída ECHO       | D10          | Entrada             |
| Servo (sinal)           | D9           | PWM (fio laranja/amarelo) |
| Buzzer                  | D2           | Digital             |
| LCD I2C SDA             | A4           | I2C                 |
| LCD I2C SCL             | A5           | I2C                 |
| VCC (sensores, LCD, servo, buzzer) | 5V | Alimentação    |
| GND (todos os componentes) | GND       | Terra comum         |

## Observações

- O LCD I2C está no endereço **0x27**.
- Cancela inicia fechada no ângulo **90°** e abre no ângulo **0°**.
- Distância de detecção configurada para **20 cm**.
- Capacidade total: **6 vagas**.
- Buzzer do tipo **ativo**.
- O servo está conectado no pino digital 9 (PWM).

## Arquivos relacionados

- Esquemático e montagem: pasta `imagens/`
- Código: `codigo/estacionamento.ino`
- Documentação técnica: pasta `documentacao/`