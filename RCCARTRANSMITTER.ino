#include "Arduino.h"
#include <SPI.h>
#include <RF24.h>

// Joystick connections
const int joyX = A0;
const int joyY = A3;
const int joyKey = 4;

// Variables to store joystick state
int analogX = 0;
int analogY = 0;
int joyKeyState = 0;

// Array to store transmitted message
int outMessage[3];

// NRF24L01 configuration
RF24 radio(9, 10);
const uint64_t my_radio_pipe = 0xE8E8F0F0E1LL;

void setup() {
  Serial.begin(9600);
  Serial.println("TRANSMITTER");

  // Initialize joystick pins
  pinMode(joyX, INPUT);
  pinMode(joyY, INPUT);
  pinMode(joyKey, INPUT);

  // Initialize NRF24L01
  radio.begin();
  radio.setAutoAck(false);
  radio.setPALevel(RF24_PA_MIN);
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(110);
  radio.openWritingPipe(my_radio_pipe);
}

void loop() {
  // Read and map joystick values
  analogX = map(analogRead(joyX), 512, 1023, 0, 255);
  analogY = map(analogRead(joyY), 512, 1023, 0, 255);
  joyKeyState = digitalRead(joyKey);

  // Store values in the message array
  outMessage[0] = analogX;
  outMessage[1] = analogY;
  outMessage[2] = joyKeyState;

  // Transmit data
  radio.stopListening();
  if (!radio.write(outMessage, sizeof(outMessage))) {
    Serial.println("Transmission failed - is the receiver connected?");
  }
}