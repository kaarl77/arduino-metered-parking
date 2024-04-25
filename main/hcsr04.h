class HCSR04 {
  private:
    float duration, distance;
    int trigPin, echoPin;
  public:
    HCSR04(int trigPin, int echoPin){
      this->trigPin = trigPin;
      this->echoPin = echoPin;
      Serial.println("HCSR04 initialized");
      delay(50);
    }

    float getDistanceInCentimeters(){
      digitalWrite(trigPin, LOW);
      delayMicroseconds(2);
      digitalWrite(trigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPin, LOW);

      duration = pulseIn(echoPin, HIGH);

      distance = (duration * .0343) / 2;

      return distance;
    }
    void printDistanceToSerial(){
      Serial.print("Distance: ");
      Serial.println(distance);
      delay(100);
    }
};