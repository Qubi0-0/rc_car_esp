#include <Arduino.h>
#include <ESP32Servo.h>
#include <analogWrite.h>
#include "BluetoothSerial.h"

#define pwm 13
#define in_1 12
#define in_2 14
#define servo_in 27
#define echo 25
#define trig 26
#define DISTANCE_TRESHOLD 0.5

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif


BluetoothSerial SerialBT;
Servo myservo;
String voice_command; 


/*setting pwm of the motor to 255
we can change the speed of rotaion
by chaning pwm input but we are only
using arduino so we are using higest
value to driver the motor  */ 
const int motor_speed = 255/2;
const float distance_const = 0.034/2;

int servo_position, duration, distance;


void setup() {
  pinMode(pwm,OUTPUT) ;  	//we have to set PWM pin as output
  pinMode(in_1,OUTPUT) ; 	//Logic pins are also set as output
  pinMode(in_2,OUTPUT) ;  // Sets 
  pinMode(trig, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(echo, INPUT); // Sets the echoPin as an INPUT
  myservo.attach(servo_in);
  Serial.begin(115200);
  SerialBT.begin("RC_Car"); //Bluetooth device name
  Serial.println("The device started, now pair it with bluetooth!");


}

// the loop function runs over and over again forever
void loop() {
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

  if (Serial.available()) {
    SerialBT.write(Serial.read());
  }
  if (SerialBT.available()) {

    if (distance <= DISTANCE_TRESHOLD) { // Stop the RC car
      digitalWrite(in_1,HIGH) ;
      digitalWrite(in_2,HIGH) ;
      }


    voice_command = SerialBT.read();
      if (voice_command == "forward") {
        digitalWrite(in_1,HIGH) ;
        digitalWrite(in_2,LOW) ;
        analogWrite(pwm,motor_speed) ; }
      else if (voice_command == "left") {
        myservo.write(servo_position = 30);
        delay(20);
        digitalWrite(in_1,HIGH) ;
        digitalWrite(in_2,HIGH) ; }
      else if (voice_command == "right") {
        myservo.write(servo_position = 90);
        delay(20);
        digitalWrite(in_1,HIGH) ;
        digitalWrite(in_2,HIGH) ; }
      else if (voice_command == "backward") {
        digitalWrite(in_1,LOW) ;
        digitalWrite(in_2,HIGH) ;
      }
      else {
        Serial.println("Unrecognized Command");
      }

  }



}
