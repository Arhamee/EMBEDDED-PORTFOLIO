#include <Wire.h>
#include <MPU6050_light.h>
#include <PID_v1.h>

MPU6050 mpu(Wire);

// PID variables
double Setpoint, Input, Output;
// Tune these values according to your robot
double Kp = 8, Ki = 1 , Kd = 2;

// Create the PID object
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

// Motor pins (example: L298N)
const int enA = 9;  // PWM for left motor
const int in1 = 2;
const int in2 = 3;

const int enB = 10; // PWM for right motor
const int in3 = 4;
const int in4 = 5;

unsigned long timer = 0;

void setup() {
  Serial.begin(9600);
  Wire.begin();

  byte status = mpu.begin();
  Serial.print(F("MPU6050 status: "));
  Serial.println(status);
  while (status != 0) {} // Stop if MPU6050 not found

  Serial.println(F("Calculating offsets, do not move MPU6050"));
  delay(1000);
  mpu.calcOffsets(); // Gyro and accelerometer calibration
  Serial.println("Done!\n");

  // Set initial setpoint to balance upright
  Setpoint = 0; // Robot should stay at 0 degrees (upright)

  // Initialize PID
  myPID.SetMode(AUTOMATIC);
  myPID.SetOutputLimits(-255, 255); // Limit output to motor PWM range

  // Setup motor pins
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
}

void loop() {
  mpu.update();

  // Get angle on Y-axis (tilt forward/backward)
  Input = mpu.getAngleY(); // Negative when leaning backward, positive when forward

  // Compute PID output
  myPID.Compute();

  // Control motors based on PID output
  driveMotors(Output);

  // Optional: Print debug info every 10ms
  if ((millis() - timer) > 10) {
    Serial.print("Angle Y: ");
    Serial.print(Input);
    Serial.print("\tPID Output: ");
    Serial.println(Output);
    timer = millis();
  }
}

// Function to drive motors based on PID output
void driveMotors(double pidOut) {
  // Clamp output between -255 and 255
  pidOut = constrain(pidOut, -255, 255);

  // Positive output: move forward
  // Negative output: move backward

  if (pidOut > 0) {
    // Forward
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    digitalWrite(in3, LOW);
    digitalWrite(in4, HIGH);
  } else {
    // Reverse
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
    pidOut = -pidOut; // Make positive for PWM
  }

  analogWrite(enA, pidOut); // Left motor
  analogWrite(enB, pidOut); // Right motor
}