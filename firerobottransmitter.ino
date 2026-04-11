#include "Arduino.h"
#include <SPI.h>
#include <RF24.h>


// Joystick connections
const int joyX1 = A1; 
const int joyY1 = A0;  
const int joyX2 = A3;
const int joyY2 = A2 ;
const int Pushbutton = 3 ;
const int toggleswitch = 2 ;


int analogX1 = 0;  
int analogY1 = 0;  
int analogX2 = 0;  
int analogY2 = 0;
int Pushbuttonstate = 0 ;
int toggleswitchstate = 0 ;

int outMessage[5];

// NRF24L01 configuration (matches receiver)
RF24 radio(9, 10);
const uint64_t pipeOut = 0xE8E8F0F0E1LL;

void setup() {
  Serial.begin(9600);
  Serial.println("TRANSMITTER STARTED");

  // Initialize inputs 
  pinMode(joyX1, INPUT);
  pinMode(joyY1, INPUT);
  pinMode(joyX2, INPUT);
  pinMode(joyY2, INPUT); 
  pinMode(Pushbutton, INPUT); 
  pinMode(toggleswitch, INPUT); 



  // Initialize radio
  radio.begin();
  radio.setAutoAck(false);
  radio.setPALevel(RF24_PA_LOW);  // Must match receiver
  radio.setDataRate(RF24_250KBPS); // Must match receiver
  radio.setChannel(110);           // Must match receiver
  radio.openWritingPipe(pipeOut);
  radio.stopListening();
}

void loop() {
  // Read and map joystick values
  analogX1 = map(analogRead(joyX1), 0, 1023, -255, 255); 
  analogY1 = map(analogRead(joyY1), 0, 1023, 255, -255); 
  analogX2 = map(analogRead(joyX2), 0, 1023, -225, 255);   
  analogY2 = map(analogRead(joyY2), 0, 1023, 255, -255);   
  Pushbuttonstate = digitalRead(Pushbutton); 
  toggleswitchstate = digitalRead(toggleswitch); 


  // Store values in message array
  outMessage[0] = analogY1;   
  outMessage[1] = analogX1;   
  outMessage[2] = analogX2;   
  outMessage[3] = analogY2;   
  outMessage[4] = Pushbuttonstate ; 
  outMessage[5] = toggleswitchstate ; 

  // Transmit data
  if (!radio.write(outMessage, sizeof(outMessage))) {
    Serial.println("Transmission failed");
  } else {
    // Debug output
    Serial.print("Y1:");
    Serial.print(analogY1);
    Serial.print(" X1:");
    Serial.print(analogX1);
    Serial.print(" X2:");
    Serial.print(analogX2);
    Serial.print(" Y2:");
    Serial.print(analogY2);
    Serial.print(" PBTN:");
    Serial.println(Pushbuttonstate);
    Serial.print(" TBTN:");
    Serial.println(toggleswitchstate);

  }

  delay(20); // Prevent flooding
}