#include <Servo.h>
#include <NewPing.h>

#define TRIGGER_PIN 12  // Arduino pin connected to trigger pin of the sensor
#define ECHO_PIN   11  // Arduino pin connected to echo pin of the sensor
#define MAX_DISTANCE 200  // Maximum distance to measure in centimeters
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

 int distance ;

const int Motor1 = 3 ;
const int Motor2 = 4 ;
const int Motor3 = 5 ;
const int Motor4 = 6 ;

 Servo myServo ;

void setup() {

  pinMode(Motor1,OUTPUT);
  pinMode(Motor2,OUTPUT);
  pinMode(Motor3,OUTPUT);
  pinMode(Motor4,OUTPUT);
  myServo.attach(9); // attached to pwm pin
  myServo.write(90);  
}
void loop() {
   distance = sonar.ping_cm();
     delay(100);
      int Rdistance = 0 ;
      int Ldistance = 0 ;
      delay(50);
  if(distance > 31){
    MoveForward() ;
  }
  else if(distance <= 30){
     moveStop() ;
     delay(300) ;
     MoveBackward() ;
     delay(300) ;
     moveStop() ;
     delay(300) ;
     Ldistance = lookLeft();
     delay(100);
     Rdistance = lookRight();
    delay(100);
  if(Ldistance > Rdistance){
    MoveLeft() ;
    delay(300);
    moveStop();
  }
  else if (Ldistance < Rdistance){
    MoveRight() ;
    delay(300);
    moveStop();
  }
  }
  delay(100);
}
void  MoveBackward(){
   digitalWrite(Motor1, HIGH);
   digitalWrite(Motor2, LOW);
   digitalWrite(Motor3, HIGH);
   digitalWrite(Motor4, LOW);
}
void MoveForward(){
   digitalWrite(Motor1, LOW);
   digitalWrite(Motor2, HIGH);
   digitalWrite(Motor3, LOW);
   digitalWrite(Motor4, HIGH);
}
void MoveRight(){
   digitalWrite(Motor1, LOW);
   digitalWrite(Motor2, HIGH);
   digitalWrite(Motor3, HIGH);
   digitalWrite(Motor4, LOW);
}
void MoveLeft(){
   digitalWrite(Motor1, HIGH);
   digitalWrite(Motor2, LOW);
   digitalWrite(Motor3, LOW);
   digitalWrite(Motor4, HIGH);
}
void moveStop(){      
  digitalWrite(Motor1, LOW);
  digitalWrite(Motor2, LOW);
  digitalWrite(Motor3, LOW);
  digitalWrite(Motor4, LOW);
}
int lookRight(){
  myServo.write(0);   
  delay(300) ;
  int dist = sonar.ping_cm();
    delay(100) ;
        myServo.write(90);  
      delay(300) ;
     return dist ;
}
int lookLeft(){
    myServo.write(180);   
      delay(300) ;
    int dist = sonar.ping_cm();
      delay(100) ;
      myServo.write(90);  
      delay(300) ;
 return dist ;
}