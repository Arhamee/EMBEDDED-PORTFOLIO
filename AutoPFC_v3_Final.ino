// =============================================================================
//  AUTOMATIC POWER FACTOR CORRECTION SYSTEM  — v3 Final
//  Hardware : Arduino Nano + ZMPT101B (4-pin) + ACS712 + 4-Relay Bank
//  Display  : 128x64 OLED (SH1106) via Adafruit_SH110X
//  v2.1     : ACS712 orientation fix + voltage scale corrected
//  v3       : Anti-hunting fix — two-threshold hysteresis + confirm counter
// =============================================================================

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// -----------------------------------------------------------------------------
//  OLED
// -----------------------------------------------------------------------------
#define SCREEN_WIDTH   128
#define SCREEN_HEIGHT   64
#define OLED_RESET      -1
#define OLED_I2C_ADDR   0x3C

Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// -----------------------------------------------------------------------------
//  PINS
// -----------------------------------------------------------------------------
#define VOLTAGE_PIN      A1
#define CURRENT_PIN      A2

#define RELAY1_PIN        5    // 1.2 µF
#define RELAY2_PIN        6    // 2.0 µF
#define RELAY3_PIN        7    // 2.0 µF
#define RELAY4_PIN        8    // 5.0 µF

#define RELAY_ON        HIGH
#define RELAY_OFF        LOW

// -----------------------------------------------------------------------------
//  SYSTEM PARAMETERS
// -----------------------------------------------------------------------------
#define AC_FREQUENCY             50.0f

// ── Two-threshold hysteresis ──────────────────────────────────────────────────
// PF below PF_LOWER → needs correction   (start switching caps in)
// PF above PF_UPPER → over-corrected     (release caps)
// PF between PF_LOWER and PF_UPPER → dead zone (hold current state, do nothing)
// This dead zone prevents caps from hunting on and off near the target
#define PF_UPPER                 0.98f   // Above this → release caps
#define PF_LOWER                 0.92f   // Below this → switch caps in

// Consecutive readings below PF_LOWER before any relay switches
// Prevents a single noisy reading from triggering unnecessary switching
#define CONFIRM_COUNT                3

#define MIN_LOAD_CURRENT         0.05f   // Amps — below this = no load
#define MIN_Q_VAR               15.0f   // VAR  — below this = ignore correction
                                         // Filters resistive loads that read
                                         // slightly low PF due to noise

// 384 samples × 2 reads × 104µs ≈ 80ms = 4 full cycles at 50Hz ✓
#define NUM_SAMPLES              384

// Calibrated sensor scales
#define VOLTAGE_SCALE            2.38f  // Calibrated: meter=226V / serial=209.5V
#define CURRENT_SCALE            0.03474f // Calibrated: meter=0.35A / serial=0.669A

// Timing
#define MEASUREMENT_INTERVAL_MS  1000
#define DISPLAY_INTERVAL_MS       500
#define RELAY_SWITCH_COOLDOWN_MS 2000
#define ZC_TIMEOUT_MS             150

// -----------------------------------------------------------------------------
//  CAPACITOR BANK
// -----------------------------------------------------------------------------
const float CAP_uF[4]    = { 1.2f, 2.0f, 2.0f, 5.0f };
const int   RELAY_PIN[4] = { RELAY1_PIN, RELAY2_PIN,
                              RELAY3_PIN, RELAY4_PIN };
// 16 combinations: 0 → 10.2 µF

// -----------------------------------------------------------------------------
//  ADC OFFSETS  —  auto-measured at startup
// -----------------------------------------------------------------------------
int V_OFFSET = 512;
int I_OFFSET = 512;

// -----------------------------------------------------------------------------
//  GLOBAL STATE
// -----------------------------------------------------------------------------
float    g_Vrms       = 0.0f;
float    g_Irms       = 0.0f;
float    g_PF         = 1.0f;
float    g_Pact       = 0.0f;
float    g_Qreac      = 0.0f;
float    g_Sapp       = 0.0f;

uint8_t  g_relayState       = 0b0000;
uint8_t  g_belowThreshCount = 0;      // Consecutive below-threshold readings
uint32_t g_lastMeasMs       = 0;
uint32_t g_lastDisplayMs    = 0;
uint32_t g_lastSwitchMs     = 0;

// =============================================================================
//  FREE RAM MONITOR
//  Printed in every serial line — keep above 200 bytes on Nano
// =============================================================================
int freeRam() {
  extern int __heap_start, *__brkval;
  int v;
  return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}

// =============================================================================
//  OFFSET CALIBRATION
//  Runs once at startup with AC connected.
//  Mean of a pure sine wave over complete cycles = 0, so
//  mean(analogRead) over many samples = true DC midpoint of sensor.
//  This is the most important fix for wrong PF readings.
// =============================================================================
void calibrateOffsets() {
  Serial.println(F("Calibrating ADC offsets..."));

  const int CAL_SAMPLES = 2000;
  long vSum = 0, iSum = 0;

  for (int i = 0; i < CAL_SAMPLES; i++) {
    vSum += analogRead(VOLTAGE_PIN);
    iSum += analogRead(CURRENT_PIN);
    delayMicroseconds(100);
  }

  V_OFFSET = (int)(vSum / CAL_SAMPLES);
  I_OFFSET = (int)(iSum / CAL_SAMPLES);

  Serial.print(F("V_OFFSET = ")); Serial.println(V_OFFSET);
  Serial.print(F("I_OFFSET = ")); Serial.println(I_OFFSET);

  if (abs(V_OFFSET - 512) > 40)
    Serial.println(F("WARN: V_OFFSET far from 512 — check ZMPT101B"));
  if (abs(I_OFFSET - 512) > 40)
    Serial.println(F("WARN: I_OFFSET far from 512 — check ACS712"));
}

// =============================================================================
//  SOFTWARE ZERO CROSS DETECTION
//  Watches VOLTAGE_PIN for signal to cross V_OFFSET going upward.
//  This is the 0V crossing of the AC sine wave on its positive half.
// =============================================================================
bool waitForZeroCross(uint32_t timeoutMs = ZC_TIMEOUT_MS) {
  uint32_t deadline = millis() + timeoutMs;
  int prev = analogRead(VOLTAGE_PIN);

  while (millis() < deadline) {
    int curr = analogRead(VOLTAGE_PIN);
    if (prev < V_OFFSET && curr >= V_OFFSET) return true;
    prev = curr;
    delayMicroseconds(50);
  }
  return false;
}

// =============================================================================
//  MEASURE  —  Sample V & I, compute Vrms, Irms, PF, P, Q, S
// =============================================================================
void measurePowerFactor() {
  if (!waitForZeroCross()) {
    Serial.println(F("WARN: Zero cross not found"));
    return;
  }

  // Accumulators — long prevents overflow at 384 samples
  // max: 512² × 384 = 100,663,296 — fits in long ✓
  long vSqSum  = 0;
  long iSqSum  = 0;
  long pRawSum = 0;
  long vSum    = 0;   // For DC residual correction
  long iSum    = 0;   // For DC residual correction

  for (int n = 0; n < NUM_SAMPLES; n++) {
    int vRaw = analogRead(VOLTAGE_PIN) - V_OFFSET;
    int iRaw = analogRead(CURRENT_PIN) - I_OFFSET;

    vSqSum  += (long)vRaw * vRaw;
    iSqSum  += (long)iRaw * iRaw;
    pRawSum += (long)vRaw * iRaw;
    vSum    += vRaw;
    iSum    += iRaw;
    // No delay — ADC paces itself at ~104µs per read
    // 384 pairs × 208µs ≈ 80ms = 4 complete cycles ✓
  }

  float vRms_counts = sqrt((float)vSqSum / NUM_SAMPLES);
  float iRms_counts = sqrt((float)iSqSum / NUM_SAMPLES);

  // DC-corrected cross-product:
  //   true mean(v·i) = mean(v·i)_raw - mean(v) × mean(i)
  // Removes any residual DC offset that survived calibrateOffsets()
  float vMean    = (float)vSum    / NUM_SAMPLES;
  float iMean    = (float)iSum    / NUM_SAMPLES;
  float pRaw_avg = ((float)pRawSum / NUM_SAMPLES) - (vMean * iMean);

  g_Vrms = vRms_counts * VOLTAGE_SCALE;
  g_Irms = iRms_counts * CURRENT_SCALE;

  // No load guard
  if (g_Irms < MIN_LOAD_CURRENT || vRms_counts < 5.0f) {
    g_PF   = 1.0f;
    g_Pact = g_Qreac = g_Sapp = 0.0f;
    return;
  }

  g_Sapp = g_Vrms * g_Irms;

  // ── Power Factor ──────────────────────────────────────────────────────────
  // cos(θ) = mean(v·i) / (Vrms_counts × Irms_counts)
  // Negated because ACS712 is installed in reverse orientation —
  // without negation, inductive loads show LEAD instead of LAG
  float denom = vRms_counts * iRms_counts;
  float pfRaw = (denom > 0.01f) ? -(pRaw_avg / denom) : 1.0f;
  pfRaw = constrain(pfRaw, -1.0f, 1.0f);

  g_PF   = pfRaw;
  g_Pact = g_Sapp * abs(g_PF);

  float theta = acos(abs(g_PF));
  g_Qreac = g_Sapp * sin(theta);
  if (g_PF < 0.0f) g_Qreac = -g_Qreac;
}

// =============================================================================
//  CALCULATE REQUIRED CAPACITANCE  (µF)
// =============================================================================
float calculateRequiredCapMicroF() {
  if (g_PF <= 0.0f)             return 0.0f;  // Leading — no correction
  if (g_PF >= PF_UPPER)         return 0.0f;  // Already well within target
  if (g_Pact < 1.0f)            return 0.0f;  // Negligible load
  if (abs(g_Qreac) < MIN_Q_VAR) return 0.0f;  // Reactive power too small
                                                // (filters resistive load noise)

  float pfNow    = constrain(g_PF,    0.01f, 0.9999f);
  float pfTarget = constrain(PF_UPPER, 0.01f, 0.9999f);

  float Q_comp  = g_Pact * (tan(acos(pfNow)) - tan(acos(pfTarget)));
  float omega   = 2.0f * PI * AC_FREQUENCY;
  float C_farad = Q_comp / (g_Vrms * g_Vrms * omega);

  return C_farad * 1.0e6f;
}

// =============================================================================
//  SELECT BEST RELAY COMBINATION
//  Scores all 16 combinations against target capacitance.
//  Over-correction penalised 2× to avoid leading PF.
// =============================================================================
uint8_t findBestRelayState(float targetCap_uF) {
  float   bestScore = 1e9f;
  uint8_t bestState = 0b0000;

  for (uint8_t state = 0; state < 16; state++) {
    float total = 0.0f;
    for (int i = 0; i < 4; i++) {
      if (state & (1 << i)) total += CAP_uF[i];
    }
    float diff  = targetCap_uF - total;
    float score = (diff >= 0.0f) ? diff : (-diff * 2.0f);
    if (score < bestScore) { bestScore = score; bestState = state; }
  }
  return bestState;
}

// =============================================================================
//  APPLY RELAY STATE
//  Always switches at zero crossing to protect relay contacts.
//  Enforces minimum cooldown between switches.
// =============================================================================
void applyRelayState(uint8_t newState) {
  if (newState == g_relayState)                              return;
  if (millis() - g_lastSwitchMs < RELAY_SWITCH_COOLDOWN_MS) return;

  waitForZeroCross();
  delayMicroseconds(300);

  for (int i = 0; i < 4; i++) {
    digitalWrite(RELAY_PIN[i], (newState & (1 << i)) ? RELAY_ON : RELAY_OFF);
  }

  g_relayState   = newState;
  g_lastSwitchMs = millis();
}

// =============================================================================
//  ACTIVE CAPACITANCE  —  sum of currently switched-in capacitors
// =============================================================================
float activeCap_uF() {
  float t = 0.0f;
  for (int i = 0; i < 4; i++) {
    if (g_relayState & (1 << i)) t += CAP_uF[i];
  }
  return t;
}

// =============================================================================
//  DISPLAY
// =============================================================================
void updateDisplay() {
  display.clearDisplay();

  // ── Title bar ─────────────────────────────────────────────────────────────
  display.fillRect(0, 0, 128, 11, SH110X_WHITE);
  display.setTextColor(SH110X_BLACK);
  display.setTextSize(1);
  display.setCursor(14, 2);
  display.print(F("AUTO PFC MONITOR"));
  display.setTextColor(SH110X_WHITE);

  // ── Row 1 — Voltage and Current ───────────────────────────────────────────
  display.setCursor(0, 14);
  display.print(F("V:"));
  display.print(g_Vrms, 1);
  display.print(F("V"));
  display.setCursor(68, 14);
  display.print(F("I:"));
  display.print(g_Irms, 2);
  display.print(F("A"));

  // ── Row 2 — Active and Reactive power ─────────────────────────────────────
  display.setCursor(0, 23);
  display.print(F("P:"));
  display.print(g_Pact, 0);
  display.print(F("W"));
  display.setCursor(68, 23);
  display.print(F("Q:"));
  display.print(abs(g_Qreac), 0);
  display.print(F("VAR"));

  // ── Row 3 — PF large + status label ───────────────────────────────────────
  display.setCursor(0, 33);
  display.print(F("PF:"));
  display.setTextSize(2);
  display.setCursor(20, 31);
  display.print(abs(g_PF), 3);
  display.setTextSize(1);
  display.setCursor(92, 33);
  if      (g_Irms < MIN_LOAD_CURRENT)  display.print(F("IDLE"));
  else if (g_PF < 0.0f)                display.print(F("LEAD"));
  else if (abs(g_PF) >= PF_LOWER)      display.print(F(" OK "));
  else                                  display.print(F(" LAG"));

  // ── Divider ───────────────────────────────────────────────────────────────
  display.drawFastHLine(0, 49, 128, SH110X_WHITE);

  // ── Row 4 — Capacitance + relay bitmap + confirm counter ──────────────────
  display.setCursor(0, 52);
  display.print(F("CAP:"));
  display.print(activeCap_uF(), 1);
  display.print(F("uF ["));
  for (int i = 3; i >= 0; i--) {
    display.print((g_relayState & (1 << i)) ? '1' : '0');
  }
  display.print(F("]"));

  // Show confirm counter on bottom right when actively counting
  // C0/C1/C2 means system is waiting to confirm correction needed
  display.setCursor(92, 52);
  if (g_belowThreshCount > 0) {
    display.print(F("C"));
    display.print(g_belowThreshCount);
    display.print(F("/"));
    display.print(CONFIRM_COUNT);
  } else {
    display.print(F("S:"));
    display.print(g_Sapp, 0);
  }

  display.display();
}

// =============================================================================
//  SETUP
// =============================================================================
void setup() {
  Serial.begin(115200);
  Serial.println(F("=== Auto PFC System v3 Final ==="));

  // All relays OFF at startup
  for (int i = 0; i < 4; i++) {
    pinMode(RELAY_PIN[i], OUTPUT);
    digitalWrite(RELAY_PIN[i], RELAY_OFF);
  }

  // OLED
  if (!display.begin(OLED_I2C_ADDR, true)) {
    Serial.println(F("ERROR: OLED not found!"));
    while (true) delay(1000);
  }

  // Splash
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.setTextSize(1);
  display.setCursor(10, 8);  display.print(F("Auto PFC  v3"));
  display.setCursor(10, 20); display.print(F("Calibrating..."));
  display.setCursor(10, 32); display.print(F("Keep load ON"));
  display.display();

  delay(500);
  calibrateOffsets();

  display.clearDisplay();
  display.setCursor(10, 8);  display.print(F("Auto PFC  v3"));
  display.setCursor(10, 20); display.print(F("V_off: ")); display.print(V_OFFSET);
  display.setCursor(10, 32); display.print(F("I_off: ")); display.print(I_OFFSET);
  display.setCursor(10, 44); display.print(F("PF zone: "));
  display.print(PF_LOWER, 2); display.print(F("-")); display.print(PF_UPPER, 2);
  display.display();

  Serial.print(F("Free RAM: ")); Serial.println(freeRam());
  delay(2000);
}

// =============================================================================
//  MAIN LOOP
// =============================================================================
void loop() {
  uint32_t now = millis();

  // ── Measurement + correction cycle ────────────────────────────────────────
  if (now - g_lastMeasMs >= MEASUREMENT_INTERVAL_MS) {
    g_lastMeasMs = now;

    measurePowerFactor();

    float   C_required = calculateRequiredCapMicroF();
    uint8_t newState;

    // ── Two-threshold hysteresis control logic ─────────────────────────────
    if (g_Irms < MIN_LOAD_CURRENT) {
      // No load — release all caps immediately, reset counter
      newState = 0b0000;
      g_belowThreshCount = 0;

    } else if (g_PF < 0.0f) {
      // Leading PF — over-corrected, release all caps immediately
      newState = 0b0000;
      g_belowThreshCount = 0;

    } else if (g_PF > PF_UPPER && g_relayState != 0b0000) {
      // Above upper threshold — over-corrected, release all caps
      newState = 0b0000;
      g_belowThreshCount = 0;

    } else if (abs(g_PF) >= PF_LOWER) {
      // Inside dead zone (PF_LOWER to PF_UPPER) — hold current state
      // This is the key anti-hunting fix:
      // once corrected, system stays put until PF drifts outside this band
      newState = g_relayState;
      g_belowThreshCount = 0;

    } else {
      // PF below lower threshold — needs correction
      // Require CONFIRM_COUNT consecutive bad readings before acting
      // Prevents single noisy readings from triggering relay chatter
      g_belowThreshCount++;

      if (g_belowThreshCount >= CONFIRM_COUNT) {
        newState = findBestRelayState(C_required);
        g_belowThreshCount = 0;  // Reset after acting
      } else {
        newState = g_relayState; // Hold — still confirming
      }
    }

    applyRelayState(newState);

    // ── Serial debug output ────────────────────────────────────────────────
    Serial.print(F("Voff="));    Serial.print(V_OFFSET);
    Serial.print(F(" Ioff="));   Serial.print(I_OFFSET);
    Serial.print(F("  V="));     Serial.print(g_Vrms,  1);
    Serial.print(F("V  I="));    Serial.print(g_Irms,  3);
    Serial.print(F("A  PF="));   Serial.print(g_PF,    4);
    Serial.print(F("  P="));     Serial.print(g_Pact,  1);
    Serial.print(F("W  Q="));    Serial.print(g_Qreac, 1);
    Serial.print(F("VAR  Cr="));  Serial.print(C_required, 2);
    Serial.print(F("uF  Ca="));   Serial.print(activeCap_uF(), 1);
    Serial.print(F("uF  Cnt="));  Serial.print(g_belowThreshCount);
    Serial.print(F("  Rel=0b"));  Serial.print(g_relayState, BIN);
    Serial.print(F("  RAM="));    Serial.println(freeRam());
  }

  // ── Display refresh cycle ──────────────────────────────────────────────────
  if (now - g_lastDisplayMs >= DISPLAY_INTERVAL_MS) {
    g_lastDisplayMs = now;
    updateDisplay();
  }
}
