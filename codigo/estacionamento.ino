/*
 * Sistema de Estacionamento Inteligente - IoT
 * SENAI - TDESS 2025 1 V1
 * Integrantes: Vinícius Philippi Lessa e Gabriel Henrique
 * 
 * Funcionalidades:
 * - Detecção de entrada e saída com sensores HC-SR04
 * - Contagem de vagas e controle de capacidade
 * - Barreira com micro servo
 * - Indicação no LCD 16x2 I2C
 * - Alerta sonoro com buzzer
 * 
 * Bibliotecas necessárias:
 * - Wire (nativa)
 * - Servo (nativa)
 * - LiquidCrystal_I2C
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

// Pinos do sensor ultrassônico de ENTRADA
#define TRIG1 4
#define ECHO1 3

// Pinos do sensor ultrassônico de SAÍDA
#define TRIG2 11
#define ECHO2 10

// Pino de sinal do servo (cancela)
#define SERVO_PIN 9

// Pino do buzzer
#define BUZZER_PIN 2

// Objeto do display LCD I2C (endereço 0x27)
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo gateServo;

// Capacidade total e vagas disponíveis
const int TOTAL_SPOTS = 6;
int availableSpots = TOTAL_SPOTS;

// Ângulos do servo para cancela fechada e aberta
const int GATE_CLOSE_ANGLE = 90;
const int GATE_OPEN_ANGLE  = 0;

// Velocidade do movimento do servo (ms por grau)
const int SERVO_STEP_DELAY_MS = 15;

// Tempo de espera antes de abrir a cancela após detecção
const int GATE_PREOPEN_DELAY_MS = 100;

// Tempo de espera após o carro passar antes de fechar a cancela
const int GATE_PRECLOSE_DELAY_MS = 500;

// Distância (em cm) para considerar que um veículo foi detectado
const int DETECT_CM = 20;

// Quantidade de leituras consecutivas necessárias para confirmar detecção
const int HIT_REQUIRED = 2;

// Tempo máximo de espera pelo eco do sensor ultrassônico
const unsigned long PULSE_TIMEOUT_US = 30000;

// Intervalo entre medições dos sensores
const unsigned long ULTRA_INTERVAL_MS = 60;

// Estados da máquina de estados do sistema
enum State {
  IDLE,                 // Aguardando veículo
  ENTER_PREPARE,        // Preparando abertura para entrada
  ENTER_OPEN,           // Abrindo cancela para entrada
  ENTER_WAIT_PASS,      // Aguardando carro passar pela entrada
  EXIT_OPEN,            // Abrindo cancela para saída
  EXIT_WAIT_PASS,       // Aguardando carro passar pela saída
  WAIT_BEFORE_CLOSE,    // Esperando antes de fechar a cancela
  WAIT_CLEAR            // Esperando sensores ficarem livres
};

State state = IDLE;

// Controle de tempo das medições ultrassônicas
unsigned long lastUltraTime = 0;
bool measureFirst = true;

// Tempo de início do estado atual
unsigned long stateStartTime = 0;

// Distâncias medidas pelos sensores
int d1 = 999;
int d2 = 999;

// Contadores de estabilidade de detecção
int hit1 = 0;
int hit2 = 0;

// Resultado final da detecção (true = veículo presente)
bool present1 = false;
bool present2 = false;

// Delta pendente: -1 = entrada (ocupa vaga), +1 = saída (libera vaga)
int pendingDelta = 0;

// Envia pulso e retorna a distância medida em centímetros
int readDistanceCm(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  unsigned long duration = pulseIn(echoPin, HIGH, PULSE_TIMEOUT_US);

  // Se não receber eco, considera distância grande
  if (duration == 0) return 999;

  int cm = duration / 58;
  if (cm <= 0) cm = 999;
  return cm;
}

// Converte as leituras de distância em detecção estável de veículo
void updatePresence() {
  bool raw1 = (d1 < DETECT_CM);
  bool raw2 = (d2 < DETECT_CM);

  // Exige várias leituras consecutivas para evitar falsos positivos
  hit1 = raw1 ? min(hit1 + 1, HIT_REQUIRED) : 0;
  hit2 = raw2 ? min(hit2 + 1, HIT_REQUIRED) : 0;

  present1 = (hit1 >= HIT_REQUIRED);
  present2 = (hit2 >= HIT_REQUIRED);
}

// Imprime as distâncias dos sensores no Serial Monitor (debug)
void debugSensores() {
  Serial.print("Sensor entrada: ");
  Serial.print(d1);
  Serial.print(" cm");

  Serial.print(" | Sensor saida: ");
  Serial.print(d2);
  Serial.println(" cm");
}

// Move o servo gradualmente para um movimento suave da cancela
void moveGateSlow(int targetAngle) {
  int currentAngle = gateServo.read();

  if (currentAngle < targetAngle) {
    for (int pos = currentAngle; pos <= targetAngle; pos++) {
      gateServo.write(pos);
      delay(SERVO_STEP_DELAY_MS);
    }
  } else {
    for (int pos = currentAngle; pos >= targetAngle; pos--) {
      gateServo.write(pos);
      delay(SERVO_STEP_DELAY_MS);
    }
  }
}

// Toca o buzzer por um tempo determinado
void tocarBuzzer(int duracaoMs) {
  // Se for Buzzer ATIVO, basta ligar e desligar:
  digitalWrite(BUZZER_PIN, HIGH);
  delay(duracaoMs);
  digitalWrite(BUZZER_PIN, LOW);

  /* 
  // CASO SEU BUZZER SEJA PASSIVO (apenas faz um "clique" com o código acima),
  // apague as 3 linhas acima e descomente o bloco abaixo:
  unsigned long fim = millis() + duracaoMs;
  while (millis() < fim) {
    digitalWrite(BUZZER_PIN, HIGH);
    delayMicroseconds(500); // ~1000 Hz
    digitalWrite(BUZZER_PIN, LOW);
    delayMicroseconds(500);
  }
  */
}

// Abre ou fecha a cancela com movimento suave
void setGate(bool open) {
  if (open) {
    // Toca um bipe duplo curto antes de abrir a cancela
    tocarBuzzer(120);
    delay(80);
    tocarBuzzer(120);
  }

  moveGateSlow(open ? GATE_OPEN_ANGLE : GATE_CLOSE_ANGLE);
}

// Escreve o texto fixo no LCD
void initLCDText() {
  lcd.setCursor(0, 0);
  lcd.print("ESTACIONAMENTO");

  lcd.setCursor(0, 1);
  lcd.print("Vagas: ");
}

// Atualiza o LCD conforme a quantidade de vagas disponíveis
void updateSpotsOnly() {
  lcd.clear();

  if (availableSpots == 0) {
    lcd.setCursor(0, 0);
    lcd.print("ESTACIONAMENTO");

    lcd.setCursor(0, 1);
    lcd.print("CHEIO!");
  }
  else {
    initLCDText();

    lcd.setCursor(7, 1);
    lcd.print(availableSpots);
  }
}

// Aplica a mudança de vagas de forma segura
void applyPendingDelta() {
  if (pendingDelta == -1) {
    if (availableSpots > 0) availableSpots--;
  } else if (pendingDelta == 1) {
    if (availableSpots < TOTAL_SPOTS) availableSpots++;
  }
  pendingDelta = 0;
}

void setup() {
  // Configura os pinos dos sensores
  pinMode(TRIG1, OUTPUT);
  pinMode(ECHO1, INPUT);
  pinMode(TRIG2, OUTPUT);
  pinMode(ECHO2, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Conecta o servo e inicia com a cancela fechada
  gateServo.attach(SERVO_PIN);
  setGate(false);

  // Inicializa o LCD
  lcd.init();
  lcd.backlight();
  initLCDText();
  updateSpotsOnly();
  Serial.begin(9600);
}

void loop() {
  unsigned long now = millis();

  // Alterna a leitura entre os dois sensores para evitar interferência
  if (now - lastUltraTime >= ULTRA_INTERVAL_MS) {
    lastUltraTime = now;

    if (measureFirst) d1 = readDistanceCm(TRIG1, ECHO1);
    else             d2 = readDistanceCm(TRIG2, ECHO2);

    measureFirst = !measureFirst;
    updatePresence();
    debugSensores();
  }

  // Máquina de estados do sistema
  switch (state) {
    // Aguardando veículo
    case IDLE:
      // Disparo de entrada (somente se houver vaga)
      if (present1 && !present2 && availableSpots > 0) {
        state = ENTER_PREPARE;
        stateStartTime = now;
      }
      // Disparo de saída
      else if (present2 && !present1) {
        state = EXIT_OPEN;
      }
      break;

    // Pequeno atraso antes de abrir a cancela para entrada
    case ENTER_PREPARE:
      if (now - stateStartTime >= (unsigned long)GATE_PREOPEN_DELAY_MS) {
        state = present1 ? ENTER_OPEN : IDLE; // Reconfirma presença para evitar abertura falsa
      }
      break;

    // Abre a cancela para o veículo entrar
    case ENTER_OPEN:
      setGate(true);
      state = ENTER_WAIT_PASS;
      break;

    // Aguarda o veículo chegar ao sensor de saída
    case ENTER_WAIT_PASS:
      if (present2) {
        pendingDelta = -1;
        applyPendingDelta();
        updateSpotsOnly();
        state = WAIT_BEFORE_CLOSE;
        stateStartTime = now;
      }
      break;

    // Abre a cancela para o veículo sair
    case EXIT_OPEN:
      setGate(true);
      state = EXIT_WAIT_PASS;
      break;

    // Aguarda o veículo chegar ao sensor de entrada
    case EXIT_WAIT_PASS:
      if (present1) {
        pendingDelta = 1;
        applyPendingDelta();
        updateSpotsOnly();
        state = WAIT_BEFORE_CLOSE;
        stateStartTime = now;
      }
      break;

    // Mantém a cancela aberta por um tempo após a passagem
    case WAIT_BEFORE_CLOSE:
      if (now - stateStartTime >= (unsigned long)GATE_PRECLOSE_DELAY_MS) {
        setGate(false);
        state = WAIT_CLEAR;
      }
      break;

    // Aguarda os dois sensores ficarem livres antes de voltar ao estado inicial
    case WAIT_CLEAR:
      if (!present1 && !present2) state = IDLE;
      break;
  }
}