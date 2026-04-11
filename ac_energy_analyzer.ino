#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT  64
#define OLED_ADDR     0x3C

#define ZMPT_PIN  A1
#define ACS_PIN   A0

#define ADC_REF   5.0
#define ADC_RES 1023.0
#define MAINS_HZ  50
#define CYCLES    20       // number of full 50Hz cycles to sample = 400ms window

float Vcal     = 644.3;
float ACS_SENS = 0.1225;
float ACS_ZERO = 512.0;    // calibrated at startup

Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

float Vrms = 0, Irms = 0, P = 0, S = 0, Q = 0, PF = 0;
float energyWh = 0;
unsigned long lastEnergyTime = 0;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  delay(250);

  display.begin(OLED_ADDR, true);
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.setTextSize(1);
  display.setCursor(15, 20);
  display.println("AC Energy Meter");
  display.setCursor(25, 36);
  display.println("Calibrating...");
  display.display();

  // ACS712 zero offset — must have NO load connected at power on
  float zeroSum = 0;
  for (int i = 0; i < 1000; i++) {
    zeroSum += analogRead(ACS_PIN);
    delayMicroseconds(100);
  }
  ACS_ZERO = zeroSum / 1000.0;

  Serial.print("ACS_ZERO: ");
  Serial.println(ACS_ZERO);

  lastEnergyTime = millis();
  delay(1500);
}

void loop() {
  // ── Sample exactly CYCLES full 50Hz periods ──────────────────────────────
  // Each cycle = 20ms, total window = 400ms
  // Arduino Nano analogRead ≈ 104us, two reads ≈ 208us
  // So ~96 sample pairs per 20ms cycle, ~1920 total pairs over 20 cycles

  unsigned long samplePeriod = (1000000UL / MAINS_HZ) * CYCLES; // microseconds
  unsigned long startTime    = micros();

  float sumVsq = 0, sumIsq = 0, sumP = 0;
  float vOffsetAcc = 0;
  int   n = 0;

  // First pass: calculate vOffset from this window
  unsigned long t = micros();
  while (micros() - startTime < samplePeriod) {
    vOffsetAcc += analogRead(ZMPT_PIN);
    n++;
    delayMicroseconds(50); // pace to ~96 samples/cycle
  }
  float vOffset = (n > 0) ? vOffsetAcc / n : 512.0;

  // Second pass: actual measurement over same window length
  startTime = micros();
  n = 0;
  while (micros() - startTime < samplePeriod) {
    float vRaw = analogRead(ZMPT_PIN) - vOffset;
    float iRaw = analogRead(ACS_PIN)  - ACS_ZERO;

    float vInst = vRaw * (ADC_REF / ADC_RES) * Vcal;
    float iInst = iRaw * (ADC_REF / ADC_RES) / ACS_SENS;

    sumVsq += vInst * vInst;
    sumIsq += iInst * iInst;
    sumP   += vInst * iInst;
    n++;
    delayMicroseconds(50);
  }

  if (n == 0) return; // safety guard

  // ── Calculate ─────────────────────────────────────────────────────────────
  Vrms = sqrt(sumVsq / n);
  Irms = sqrt(sumIsq / n);

  // Noise floor — ACS712 is noisy at low currents
  if (Irms < 0.05) Irms = 0.0;

  P = sumP / n;
  if (Irms == 0.0) P = 0.0;

  S  = Vrms * Irms;
  Q  = sqrt(fabs(S * S - P * P));
  PF = (S > 1.0) ? constrain(P / S, -1.0, 1.0) : 0.0;

  // ── Energy accumulation ───────────────────────────────────────────────────
  unsigned long now = millis();
  float dtH = (now - lastEnergyTime) / 3600000.0;
  if (P > 0) energyWh += P * dtH;   // only accumulate positive power
  lastEnergyTime = now;

  // ── Serial debug ──────────────────────────────────────────────────────────
  Serial.print("N=");    Serial.print(n);
  Serial.print(" Vrms="); Serial.print(Vrms, 1);
  Serial.print(" Irms="); Serial.print(Irms, 3);
  Serial.print(" P=");   Serial.print(P, 2);
  Serial.print("W S=");  Serial.print(S, 2);
  Serial.print("VA Q="); Serial.print(Q, 2);
  Serial.print("VAR PF="); Serial.print(PF, 3);
  Serial.print(" E=");   Serial.print(energyWh, 4);
  Serial.println("Wh");

  // ── Display ───────────────────────────────────────────────────────────────
  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("V:"); display.print(Vrms, 1);
  display.print("V  I:"); display.print(Irms, 3); display.println("A");

  display.setCursor(0, 12);
  display.print("P :"); display.print(P, 1); display.println(" W");

  display.setCursor(0, 22);
  display.print("S :"); display.print(S, 1); display.println(" VA");

  display.setCursor(0, 32);
  display.print("Q :"); display.print(Q, 1); display.println(" VAR");

  display.setCursor(0, 42);
  display.print("PF:"); display.print(PF, 3);
  // PF quality indicator
  if (S > 1.0) {
    if      (PF > 0.95)  display.println(" GOOD");
    else if (PF > 0.80)  display.println(" FAIR");
    else                 display.println(" POOR");
  }

  display.setCursor(0, 52);
  if (energyWh < 1000.0) {
    display.print("E :"); display.print(energyWh, 2); display.print(" Wh");
  } else {
    display.print("E :"); display.print(energyWh / 1000.0, 3); display.print(" kWh");
  }

  display.display();
}