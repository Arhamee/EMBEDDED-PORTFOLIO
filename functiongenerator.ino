#include <Wire.h>
#include <Adafruit_MCP4725.h>
#include <arduinoFFT.h>

Adafruit_MCP4725 dac;

int signals = 0;
unsigned int delayMicros = 1000; // waveform step delay (microseconds)

// --- FFT Setup ---
const int FFT_SIZE = 128;
double vReal[FFT_SIZE];
double vImag[FFT_SIZE];
ArduinoFFT<double> FFT(vReal, vImag, FFT_SIZE, 10000.0); // Approx. 10 kHz sampling rate

// --- Square Wave Table (100 points) ---
const PROGMEM uint16_t DACLookup_Square_100[100] = {
4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,
4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,
4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,
4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,
4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// --- Sine Wave Table (100 points) ---
const PROGMEM uint16_t DACLookup_Sine_100[100] = {
2048,2176,2304,2431,2557,2680,2801,2919,3034,3145,
3251,3353,3449,3540,3625,3704,3776,3842,3900,3951,
3995,4031,4059,4079,4091,4095,4091,4079,4059,4031,
3995,3951,3900,3842,3776,3704,3625,3540,3449,3353,
3251,3145,3034,2919,2801,2680,2557,2431,2304,2176,
2048,1919,1791,1664,1538,1415,1294,1176,1061,950,
844,742,646,555,470,391,319,253,195,144,
100,64,36,16,4,0,4,16,36,64,
100,144,195,253,319,391,470,555,646,742,
844,950,1061,1176,1294,1415,1538,1664,1791,1919
};

// --- Triangle Wave Table (100 points) ---
const PROGMEM uint16_t DACLookup_Triangle_100[100] = {
0,82,164,246,328,410,491,573,655,737,
819,901,983,1065,1147,1229,1310,1392,1474,1556,
1638,1720,1802,1884,1966,2048,2129,2211,2293,2375,
2457,2539,2621,2703,2785,2867,2948,3030,3112,3194,
3276,3358,3440,3522,3604,3686,3767,3849,3931,4013,
4095,4013,3931,3849,3767,3686,3604,3522,3440,3358,
3276,3194,3112,3030,2948,2867,2785,2703,2621,2539,
2457,2375,2293,2211,2129,2048,1966,1884,1802,1720,
1638,1556,1474,1392,1310,1229,1147,1065,983,901,
819,737,655,573,491,410,328,246,164,82
};

// --- Sawtooth Wave Table (100 points) ---
const PROGMEM uint16_t DACLookup_Saw_100[100] = {
0,41,82,123,164,205,246,287,328,369,
410,451,491,533,573,615,655,697,737,779,
819,861,901,943,983,1025,1065,1107,1147,1189,
1229,1271,1310,1353,1392,1433,1474,1515,1556,1597,
1638,1679,1720,1761,1802,1843,1884,1925,1966,2007,
2048,2089,2129,2171,2211,2253,2293,2334,2375,2416,
2457,2498,2539,2580,2621,2662,2703,2744,2785,2826,
2867,2908,2948,2989,3030,3071,3112,3153,3194,3235,
3276,3317,3358,3399,3440,3481,3522,3563,3604,3645,
3686,3727,3767,3809,3849,3891,3931,3973,4013,4095
};

void setup(void) {
Serial.begin(115200);
dac.begin(0x60);

pinMode(2, INPUT_PULLUP);
attachInterrupt(digitalPinToInterrupt(2), change_type, FALLING); // safer than LOW

Serial.println("Waveform Generator + FFT Analyzer Ready");
}

void change_type(void) {
static unsigned long lastInterrupt = 0;
if (millis() - lastInterrupt < 300) return; // debounce
lastInterrupt = millis();

signals++;
if (signals > 3) signals = 0;
}

// --- Generate & Capture ---
void loop(void) {
delayMicros = map(analogRead(A0), 0, 1023, 100, 2000); // smoother frequency range

const int samplesNeeded = FFT_SIZE;
int captured = 0;
static uint16_t tempBuffer[FFT_SIZE];

while (captured < samplesNeeded) {
for (int i = 0; i < 100 && captured < samplesNeeded; i++) {
uint16_t val;
switch (signals) {
case 0: val = pgm_read_word(&(DACLookup_Sine_100[i])); break;
case 1: val = pgm_read_word(&(DACLookup_Triangle_100[i])); break;
case 2: val = pgm_read_word(&(DACLookup_Saw_100[i])); break;
case 3: val = pgm_read_word(&(DACLookup_Square_100[i])); break;
default: val = 2048;
}
dac.setVoltage(val, false);
tempBuffer[captured++] = val;
delayMicroseconds(delayMicros);
}
}

// Prepare FFT input
for (int i = 0; i < FFT_SIZE; i++) {
vReal[i] = (double)(tempBuffer[i] - 2048); // DC offset removal
vImag[i] = 0.0;
}

FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
FFT.compute(FFT_FORWARD);
FFT.complexToMagnitude();

// Print FFT magnitude for Serial Plotter
for (int i = 1; i < FFT_SIZE / 2; i++) {
Serial.println(vReal[i]);
}

delay(50);
}