#include <Arduino.h>
#include <ESP32Servo.h>
#include <analogWrite.h>

Servo myservo;
const int pwm = 13 ;	//initializing pin 2 as pwm
const int in_1 = 12 ;
const int in_2 = 14 ;
const int servo_in = 27;
const int motor_speed = 255/2;
int val;
// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  // pinMode(LED_BUILTIN, OUTPUT);
  pinMode(pwm,OUTPUT) ;  	//we have to set PWM pin as output
  pinMode(in_1,OUTPUT) ; 	//Logic pins are also set as output
  pinMode(in_2,OUTPUT) ;
  myservo.attach(servo_in);

}

// the loop function runs over and over again forever
void loop() {
//For Clock wise motion , in_1 = High , in_2 = Low
digitalWrite(in_1,HIGH) ;
digitalWrite(in_2,LOW) ;
analogWrite(pwm,motor_speed) ;
val = 30;     // scale it to use it with the servo (value between 0 and 180)
myservo.write(val);   
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
myservo.write(val); 
//For Anti Clock-wise motion - IN_1 = LOW , IN_2 = HIGH
digitalWrite(in_1,LOW) ;
digitalWrite(in_2,HIGH) ;
delay(3000) ;
val = 120;     // scale it to use it with the servo (value between 0 and 180)
myservo.write(val); 
//For brake
digitalWrite(in_1,HIGH) ;
digitalWrite(in_2,HIGH) ;
delay(1000) ;                  // wait for a second


}