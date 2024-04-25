#include "hcsr04.h"
#include <Servo.h>
#include <LiquidCrystal.h>

//HCSR04 IN sensor
const int trigPin1 = 9;
const int echoPin1 = 10;
const int switchPin1 = 11;

//HCSR04 OUT sensor
const int trigPin2 = 8;
const int echoPin2 = 6;
const int switchPin2 = 5;

//SERVO MOTOR
const int servoPin = 7;

//LCD
LiquidCrystal lcd(14,15,16,17,18,19);

//STATES & FLAGS
int switchState1 = 0;
int switchState2 = 0;
int shouldMeasure = 0;
int shouldOpenGate = 0;
unsigned long startTime = 0;
unsigned long stopTime = 0;
int carIn = 0;
int carOut = 0;
int carSpotsLeft = 5;

//SENSOR CLASSES
HCSR04* hcsr041;
HCSR04* hcsr042;
Servo myServo;

void openGate() {
  myServo.write(0);
}

void closeGate() {
  myServo.write(45);
}

int objectInProximity(HCSR04* sensor) {
  float distance = sensor->getDistanceInCentimeters();
  if (distance >= 3 && distance <= 20) {
    return 1;
  }
  return 0;
}

void setup() {
  // put your setup code here, to run once:
  pinMode(trigPin1, OUTPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(switchPin1, INPUT);
  pinMode(switchPin2, INPUT);

  myServo.attach(servoPin);
  closeGate();
  Serial.begin(9600);
  hcsr041 = new HCSR04(trigPin1, echoPin1);
  hcsr042 = new HCSR04(trigPin2, echoPin2);
}

void loop() {
  switchState1 = digitalRead(switchPin1);
  switchState2 = digitalRead(switchPin2);

  if (switchState1 == HIGH && !shouldMeasure) {
    // Serial.println("Car tryin to come in");
    shouldMeasure = 1;
    shouldOpenGate = 0;  // Reset gate flag whenever a new measurement session starts
    carIn = 1;
    carOut = 0;
    startTime = millis();
    stopTime = startTime + 2000;
  }

  if (switchState2 == HIGH && !shouldMeasure) {
    // Serial.println("Car trying to come out");
    shouldMeasure = 1;
    shouldOpenGate = 0;
    carIn = 0;
    carOut = 1;
    startTime = millis();
    stopTime = startTime + 2000;
  }

  if (millis() <= stopTime) {
    if (shouldMeasure) {
      if (carIn && carSpotsLeft) {
        float distanceToObject1 = hcsr041->getDistanceInCentimeters();
        float distanceToObject2 = hcsr042->getDistanceInCentimeters();
        if (distanceToObject1 >= 3 && distanceToObject1 <= 20 && distanceToObject2 > 20) {
          shouldOpenGate = 1;  // Set flag to open gate if conditions are met during measurement
        }
      }
      if (carOut) {
        float distanceToObject1 = hcsr041->getDistanceInCentimeters();
        float distanceToObject2 = hcsr042->getDistanceInCentimeters();
        if (distanceToObject2 >= 3 && distanceToObject2 <= 20 && distanceToObject1 > 20) {
          shouldOpenGate = 1;  // Set flag to open gate if conditions are met during measurement
        }
      }
    }
  } else {
    shouldMeasure = 0;
  }

  if (shouldOpenGate) {
    // Serial.println("Should open gate");
    openGate();
  
    unsigned long endTime = millis() + 3000;
    int goodSensorReads = 0;

    if (carIn) {
      while(millis() < endTime){
        float confirmDistance = hcsr042->getDistanceInCentimeters();
        Serial.print("Distance:");
        Serial.print(confirmDistance);
        Serial.print(",");
        if(confirmDistance >=3 && confirmDistance <= 20){
          goodSensorReads++;
        }
      }

      if (goodSensorReads >=1) {  // Check if car is really inside
        carSpotsLeft--;
        // Serial.println("Car confirmed inside.");
        
      } else {
        // Serial.println("No car detected inside after gate closed");
      }
    }
    if (carOut) {
      while(millis() < endTime){
        float confirmDistance = hcsr041->getDistanceInCentimeters();
        Serial.print("Distance:");
        Serial.print(confirmDistance);
        Serial.print(",");
        if(confirmDistance >=3 && confirmDistance <= 20){
          goodSensorReads++;
        }
      }

      if (goodSensorReads >=1) {  // Check if car is really inside
        if (carSpotsLeft < 5) {
          carSpotsLeft++;
        }
        // Serial.println("Car confirmed inside.");
      } else {
        // Serial.println("No car detected inside after gate closed");
      }
    }
    carIn = 0;
    carOut = 0;
    Serial.print("Spots:");
    Serial.print(carSpotsLeft);
    Serial.print(",");




    closeGate();
    shouldOpenGate = 0;
    Serial.print("Hits:");
    Serial.println(goodSensorReads);
  }
}