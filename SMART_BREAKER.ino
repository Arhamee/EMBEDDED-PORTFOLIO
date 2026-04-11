#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

const int RELAY_PIN = 3;
const int BUTTON_PIN = 4;
Adafruit_SH1106G display(128, 64, &Wire, -1);

bool isLocked = false;

void setup() {
  Wire.begin();
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  digitalWrite(RELAY_PIN, HIGH); // Start ON
  delay(250);
  display.begin(0x3C, true);
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
}

void loop() {
  float vSum = 0;
  for (int i = 0; i < 500; i++) vSum += analogRead(A1);
  float vOffset = vSum / 500.0;

  float sumVsq = 0, sumIsq = 0;
  for (int i = 0; i < 500; i++) {
    float v = (analogRead(A1) - vOffset) * (5.0 / 1023.0) * 603.0;
    float c = -((analogRead(A2) - 506.4) * (5.0 / 1023.0) / 0.1225);
    sumVsq += v * v;
    sumIsq += c * c;
  }

  float Vrms = sqrt(sumVsq / 500.0);
  float Irms = sqrt(sumIsq / 500.0);

  bool fault = (Vrms > 260 || Vrms < 180 || Irms > 0.3);
  if (fault) isLocked = true;

  if (digitalRead(BUTTON_PIN) == HIGH) {
    delay(50); 
    if (!fault) isLocked = false; // Only reset if safe
  }

  bool relayOn = !isLocked && !fault;
  digitalWrite(RELAY_PIN, relayOn ? HIGH : LOW);

  const char* vStat = (Vrms > 260) ? "OVER" : (Vrms < 180) ? "UNDER" : "GOOD";
  const char* iStat = (Irms > 0.3) ? "OVER" : "GOOD";
  const char* sysStat = isLocked ? "LOCKED" : (relayOn ? "ON" : "OFF");

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);  display.print("V:"); display.print(Vrms, 1); display.println("V");
  display.setCursor(0, 12); display.print("I:"); display.print(Irms, 3); display.println("A");
  display.setCursor(0, 24); display.print("V: "); display.println(vStat);
  display.setCursor(0, 36); display.print("I: "); display.println(iStat);
  display.setCursor(0, 48); display.print("Sys: "); display.println(sysStat);
  display.display();
}