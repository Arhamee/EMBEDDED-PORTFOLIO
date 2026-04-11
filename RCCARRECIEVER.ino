#include "Arduino.h"
#include <SPI.h>
#include <RF24.h>

// Motor connections 
const int enA = 5;
const int in1 = 3;
const int in2 = 4;
const int enB = 6;
const int in3 = 8;
const int in4 = 7;

// RF24 configuration
const uint64_t pipeIn = 0xE8E8F0F0E1LL;
RF24 radio(9, 10);

// Variables
int analogX = 0;
int analogY = 0;
int joyKeyState = 0;
int inMessage[3];
unsigned long lastReceiveTime = 0;

const int THRESHOLD = 15; 

void setup() {
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  moveStop();

  Serial.begin(9600);
  Serial.println("Receiver started.");

  radio.begin();
  radio.setAutoAck(false);
  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(110);
  radio.openReadingPipe(1, pipeIn);
  radio.startListening();
}

void loop() {
  if (radio.available()) {
    radio.read(&inMessage, sizeof(inMessage));
    analogY = inMessage[0]; 
    analogX = inMessage[1]; 
    joyKeyState = inMessage[2];
    lastReceiveTime = millis();

    // Print raw values for debugging
    Serial.print("RX: X=");
    Serial.print(analogX);
    Serial.print(" Y=");
    Serial.print(analogY);
    Serial.print(" Key=");
    Serial.println(joyKeyState);

  
    if (abs(analogY) > THRESHOLD) {
      if (analogY > THRESHOLD) {
        Serial.println("FORWARD");
        moveForward(analogY);
      } else {
        Serial.println("BACKWARD");
        moveBackward(abs(analogY));
      }
    } 
    else if (abs(analogX) > THRESHOLD) {
      if (analogX > THRESHOLD) {
        Serial.println("RIGHT");
        turnRight();
      } else {
        Serial.println("LEFT");
        turnLeft();
      }
    } 
    else {
      Serial.println("STOP");
      moveStop();
    }
  }

  // Safety stop after 200ms of no signal
  if (millis() - lastReceiveTime > 200) {
    moveStop();
  }
}


void moveStop() {
  analogWrite(enA, 0);
  analogWrite(enB, 0);
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
}

void moveForward(int speed) {
  analogWrite(enA, speed);
  analogWrite(enB, speed);
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
}

void moveBackward(int speed) {
  analogWrite(enA, speed);
  analogWrite(enB, speed);
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
}

void turnRight() {
  analogWrite(enA, 200); 
  analogWrite(enB, 200); 
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, HIGH); 
  digitalWrite(in4, LOW);
}

void turnLeft() {
  analogWrite(enA, 200);
  analogWrite(enB, 200);
  digitalWrite(in1, HIGH); 
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
}