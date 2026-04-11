char data = 0; // Variable to store incoming data
const int motor1 = 3;
const int motor2 = 4;
const int motor3 = 5;
const int motor4 = 6;

void setup() {
  pinMode(motor1, OUTPUT);
  pinMode(motor2, OUTPUT);
  pinMode(motor3, OUTPUT);
  pinMode(motor4, OUTPUT);
  Serial.begin(9600); 
}

void loop() {
  if (Serial.available()) {
    data = Serial.read(); // Read one byte from serial

    switch(data){
      case 'F':
        Moveforward();
        break;
      case 'B':
        Movebackward();
        break;
      case 'L':
        MoveLeft();
        break;
      case 'R':
        MoveRight();
        break;
      case 'S':
        moveStop();
        break;
    }
  }
}

// Motor control functions
void moveStop() {
  digitalWrite(motor1, LOW);
  digitalWrite(motor2, LOW);
  digitalWrite(motor3, LOW);
  digitalWrite(motor4, LOW);
}

void Movebackward() {
  digitalWrite(motor1, HIGH);
  digitalWrite(motor2, LOW);
  digitalWrite(motor3, HIGH);
  digitalWrite(motor4, LOW);
}

void Moveforward() {
  digitalWrite(motor1, LOW);
  digitalWrite(motor2, HIGH);
  digitalWrite(motor3, LOW);
  digitalWrite(motor4, HIGH);
}

void MoveLeft() {
  digitalWrite(motor1, HIGH);
  digitalWrite(motor2, LOW);
  digitalWrite(motor3, LOW);
  digitalWrite(motor4, HIGH);
}

void MoveRight() {
  digitalWrite(motor1, LOW);
  digitalWrite(motor2, HIGH);
  digitalWrite(motor3, HIGH);
  digitalWrite(motor4, LOW);
}
