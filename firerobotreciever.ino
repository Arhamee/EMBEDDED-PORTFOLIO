#include "Arduino.h"
#include <SPI.h>
#include <RF24.h>
#include <Servo.h>

// Motor connections 
const int IN1 = 2;
const int IN2 = 3;
const int IN3 = 4;
const int IN4 = 7;

int lastToggleState = LOW;
int Lockangle1 = 90;
int Lockangle2 = 90;

// Servo 
Servo servo1;
const int SERVO1_PIN = 5;
int servo1Control = 0;

// Servo 
Servo servo2 ;
const int SERVO2_PIN = 6;
int servo2Control = 0;
// Water pump
const int WATER = 8;

// RF24 configuration
const uint64_t PIPE_IN = 0xE8E8F0F0E1LL;
RF24 radio(9, 10);

// Joystick values
int analogX1 = 0;  
int analogY1 = 0;  
int analogX2 = 0;  
int analogY2 = 0;  
int Pushbuttonstate = 0 ;
int toggleswitchstate = 0 ;

int inMessage[5];
unsigned long lastReceiveTime = 0;

int THRESHOLD = 15;
const unsigned long SAFETY_TIMEOUT = 200; // ms

void setup() {
  // Servo setup
  servo1.attach(SERVO1_PIN); 
  servo2.attach(SERVO2_PIN); 

  // Pin modes
  pinMode(WATER, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  
  // Initialize outputs
  moveStop();
  digitalWrite(WATER, LOW);

  // Radio setup
  Serial.begin(9600);
  Serial.println("Receiver started.");

  radio.begin();
  radio.setAutoAck(false);
  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(110);
  radio.openReadingPipe(1, PIPE_IN);
  radio.startListening();
}

void loop() {
  if (radio.available()) {
    radio.read(&inMessage, sizeof(inMessage));
    
    analogY1 = inMessage[0];  
    analogX1 = inMessage[1]; 
    analogX2 = inMessage[2];  
    analogY2 = inMessage[3];  
    Pushbuttonstate = inMessage[4];
    toggleswitchstate = inMessage[5];
    
    lastReceiveTime = millis();

  servo1Control = map(analogX2, -255, 255, 0, 180);
  servo2Control = map(analogY2, 255, -255, 0, 180);
  servo1.write(servo1Control);
  servo2.write(servo2Control);
    // Control water pump
    waterSpray();

    // Process movement (strict priority: forward/backward > left/right)
    if (abs(analogY1) > THRESHOLD) {
      if (analogY1 > THRESHOLD) {
        Serial.println("FORWARD");
        moveForward();
      } else if (analogY1 < -THRESHOLD) {
        Serial.println("BACKWARD");
        moveBackward();
      }
    } 
    else if (abs(analogX1) > THRESHOLD) {
      if (analogX1 > THRESHOLD) {
        Serial.println("RIGHT");
        turnRight();
      } else if (analogX1 < -THRESHOLD) {
        Serial.println("LEFT");
        turnLeft();
      }
    } 
    else {
      Serial.println("STOP");
      moveStop();
    }
  }

  // Safety stop after timeout of no signal
  if (millis() - lastReceiveTime > SAFETY_TIMEOUT) {
    moveStop();
    digitalWrite(WATER, LOW);
  }
}

// Motor control functions
void moveStop() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void moveForward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void moveBackward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void turnRight() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); 
  digitalWrite(IN4, LOW);
}

void turnLeft() {
  digitalWrite(IN1, HIGH); 
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void waterSpray() {
  if (Pushbuttonstate == 1) {
    digitalWrite(WATER, HIGH);
    Serial.println("WATER PUMP ON");
  } else {
    digitalWrite(WATER, LOW);
  }
}

