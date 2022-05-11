#include <Arduino.h>
#include <ESP32Servo.h>
#include <analogWrite.h>

#define pwm 13
#define in_1 12
#define in_2 14
#define servo_in 27
#define echo 25
#define trig 26
Servo myservo;
const int motor_speed = 255/2;
int val, duration, distance;
const float distance_const = 0.034/2;
// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  // pinMode(LED_BUILTIN, OUTPUT);
  pinMode(pwm,OUTPUT) ;  	//we have to set PWM pin as output
  pinMode(in_1,OUTPUT) ; 	//Logic pins are also set as output
  pinMode(in_2,OUTPUT) ;  // Sets 
  pinMode(trig, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(echo, INPUT); // Sets the echoPin as an INPUT
  myservo.attach(servo_in);
  Serial.begin(9600); // // Serial Communication is starting with 9600 of baudrate speed
}

// the loop function runs over and over again forever
void loop() {
//For Clock wise motion , in_1 = High , in_2 = Low
digitalWrite(in_1,HIGH) ;
digitalWrite(in_2,LOW) ;
analogWrite(pwm,motor_speed) ;
val = 30;     // scale it to use it with the servo (value between 0 and 180)
// myservo.write(val);   
/*setting pwm of the motor to 255
we can change the speed of rotaion
by chaning pwm input but we are only
using arduino so we are using higest
value to driver the motor  */ 

//Clockwise for 3 secs
delay(3000) ; 		

//For brake
digitalWrite(in_1,HIGH) ;
digitalWrite(in_2,HIGH) ;
delay(1000) ;
val = 90;     // scale it to use it with the servo (value between 0 and 180)
// myservo.write(val); 
//For Anti Clock-wise motion - IN_1 = LOW , IN_2 = HIGH
digitalWrite(in_1,LOW) ;
digitalWrite(in_2,HIGH) ;
delay(3000) ;
val = 120;     // scale it to use it with the servo (value between 0 and 180)
// myservo.write(val); 
//For brake
digitalWrite(in_1,HIGH) ;
digitalWrite(in_2,HIGH) ;
delay(1000) ;                  // wait for a second

  // Clears the trigPin
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echo, HIGH);
  // Calculating the distance
  distance = duration * distance_const;
  // Prints the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.println(distance);



}
