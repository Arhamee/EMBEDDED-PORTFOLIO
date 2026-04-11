// Pin Definitions
int ENA = 9;
int ENB = 3;   
int MOTOR_A1 = 7; 
int MOTOR_A2 = 6; 
int MOTOR_B1 = 5; 
int MOTOR_B2 = 4; 

int RIGHT = A0; // Right IR sensor
int LEFT = A1;  // Left IR sensor

// Variables
int S = 100;    //  motor speed (PWM value)
int R = 0, L = 0; // Sensor readings
bool isTurning = false; // Turn lock flag

void setup() {
  
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(MOTOR_A1, OUTPUT);
  pinMode(MOTOR_A2, OUTPUT);
  pinMode(MOTOR_B1, OUTPUT);
  pinMode(MOTOR_B2, OUTPUT);

  pinMode(RIGHT, INPUT);
  pinMode(LEFT, INPUT);

  Serial.begin(9600);
}

void loop() {
  // Read sensor values
  R = analogRead(RIGHT);
  L = analogRead(LEFT);

  // Print sensor values for debugging
  Serial.print("RIGHT: ");
  Serial.print(R);
  Serial.print(" | LEFT: ");
  Serial.println(L);


  if (!isTurning) { 
    if (R < 200 && L < 200) {
      
      moveForward();
    } else if (L >= 850) {
      
      turnRight();
    } else if (R >= 850) {
    
      turnLeft();
    } else {
  
      moveStop();
    }
  }

  delay(100);
}

// Function to stop the robot
void moveStop() {
  Serial.println("Stopping Motors");
  analogWrite(ENA, 0); 
  analogWrite(ENB, 0); 
  digitalWrite(MOTOR_A1, LOW);
  digitalWrite(MOTOR_A2, LOW);
  digitalWrite(MOTOR_B1, LOW);
  digitalWrite(MOTOR_B2, LOW);
  isTurning = false; // Reset turn lock
}

// Function to move the robot forward
void moveForward() {
  Serial.println("Moving Forward");
  analogWrite(ENA, 70);
  analogWrite(ENB, 70); 
  digitalWrite(MOTOR_A1, HIGH); 
  digitalWrite(MOTOR_A2, LOW);
  digitalWrite(MOTOR_B1, HIGH); 
  digitalWrite(MOTOR_B2, LOW);
  isTurning = false; // Reset turn lock
}

// Function to turn the robot right
void turnRight() {
  isTurning = true; 
  Serial.println("Turning Right");
  analogWrite(ENA, S); 
  analogWrite(ENB, 0); 
  digitalWrite(MOTOR_A1, HIGH); 
  digitalWrite(MOTOR_A2, LOW);
  digitalWrite(MOTOR_B1, LOW); 
  digitalWrite(MOTOR_B2, HIGH);
  delay(300); 
  isTurning = false; // Unlock after the turn
}

// Function to turn the robot left
void turnLeft() {
  isTurning = true; 
  Serial.println("Turning Left");
  analogWrite(ENA, 0); 
  analogWrite(ENB, S); 
  digitalWrite(MOTOR_A1, LOW); 
  digitalWrite(MOTOR_A2, HIGH);
  digitalWrite(MOTOR_B1, HIGH); 
  digitalWrite(MOTOR_B2, LOW);
  delay(300); 
  isTurning = false; // Unlock after the turn
}