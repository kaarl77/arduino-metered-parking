#include "hcsr04.h"
#include <Servo.h>
#include <LiquidCrystal.h>

//HCSR04 IN sensor
const int trigPin1 = 9;
const int echoPin1 = 15;
const int switchPin1 = 16;

//HCSR04 OUT sensor
const int trigPin2 = 8;
const int echoPin2 = 14;
const int switchPin2 = 17;

//LEDs
const int parkingFullLed = 13;
const int spotsAvailableLed = 6;


//SERVO MOTOR
const int servoPin = 7;

//LCD
LiquidCrystal lcd(12,11,5,4,3,2);

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
  myServo.write(90);
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
  pinMode(parkingFullLed, OUTPUT);
  pinMode(spotsAvailableLed, OUTPUT);
  myServo.attach(servoPin);
  closeGate();
  Serial.begin(9600);
  hcsr041 = new HCSR04(trigPin1, echoPin1);
  hcsr042 = new HCSR04(trigPin2, echoPin2);
}

void loop() {
  lcd.setCursor(0, 1);
  lcd.print("Spots: ");
  lcd.print(carSpotsLeft);


  if(carSpotsLeft==0){
    digitalWrite(parkingFullLed, HIGH);
    digitalWrite(spotsAvailableLed, LOW);
  } 
  else{
    digitalWrite(parkingFullLed, LOW);
    digitalWrite(spotsAvailableLed, HIGH);
  }


  switchState1 = digitalRead(switchPin1);
  switchState2 = digitalRead(switchPin2);

//a car is trying to enter
  if (switchState1 == HIGH && !shouldMeasure) {
    shouldMeasure = 1;
    shouldOpenGate = 0;  // Reset gate flag whenever a new measurement session starts
    carIn = 1;
    carOut = 0;
    startTime = millis();
    stopTime = startTime + 2000;
  }

  if (switchState2 == HIGH){
    Serial.println("Button pressed ");
  }


//a car is trying to exit
  if (switchState2 == HIGH && !shouldMeasure) {
    Serial.println("TRYING TO EXIT");
    shouldMeasure = 1;
    shouldOpenGate = 0;
    carIn = 0;
    carOut = 1;
    startTime = millis();
    stopTime = startTime + 2000;
  }

//timer for car going in or out
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

//conditions were met, gate should be open
  if (shouldOpenGate) {
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