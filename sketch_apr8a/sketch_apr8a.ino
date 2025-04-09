#include <Servo.h>

const int trigPin = 7;
const int echoPin = 6;
const int soilPin = A1;
const int servoPin = 8;

Servo servo1;

long duration;
int moistureRaw = 0;
int currentServoPos = 85;  // Starting from corrected neutral

// Servo positions (adjusted neutral for physical tilt)
const int neutralPos = 85;
const int dryPos = 10;
const int wetPos = 170;

void setup() {
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  servo1.attach(servoPin);
  servo1.write(neutralPos);
  currentServoPos = neutralPos;
  Serial.println("🚀 Smart Waste Sorter: Smooth Motion + Trigger Outside 13–15 cm + Tilt Compensation");
}

void loop() {
  int distance = getStableDistance();

  Serial.print("📏 Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  if (distance < 13 || distance > 15) {
    Serial.println("✅ Object OUT of 13–15 cm range — Checking Moisture...");
    moistureRaw = readMoisture();

    Serial.print("💧 Moisture RAW: ");
    Serial.println(moistureRaw);

    if (moistureRaw > 1020) {
      Serial.println("🌬️ AIR Detected — No Rotation");
    } else if (moistureRaw >= 900 && moistureRaw <= 1000) {
      Serial.println("🟤 DRY Waste ➡ Smooth Rotate Left");
      smoothRotate(dryPos);
    } else if (moistureRaw < 900) {
      Serial.println("🟢 WET Waste ➡ Smooth Rotate Right");
      smoothRotate(wetPos);
    } else {
      Serial.println("❓ Unknown Moisture — Skipping");
    }

    delay(3000);  // Sorting pause
    smoothRotate(neutralPos);  // Return to corrected center
    delay(1000);
  } else {
    Serial.println("🟡 Stable Object in Range — Ignoring");
  }

  delay(300);
}

void smoothRotate(int targetPos) {
  int step = (targetPos > currentServoPos) ? 1 : -1;

  for (int pos = currentServoPos; pos != targetPos; pos += step) {
    servo1.write(pos);
    delay(10); // smaller delay = faster movement, larger = slower
  }

  servo1.write(targetPos);
  currentServoPos = targetPos;
}

int getStableDistance() {
  int total = 0;
  int valid = 0;

  for (int i = 0; i < 5; i++) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    duration = pulseIn(echoPin, HIGH, 25000);
    if (duration > 0) {
      int d = duration * 0.034 / 2;
      if (d < 50) {
        total += d;
        valid++;
      }
    }
    delay(20);
  }

  return valid > 0 ? (total / valid) : 999;
}

int readMoisture() {
  int total = 0;
  for (int i = 0; i < 5; i++) {
    total += analogRead(soilPin);
    delay(40);
  }
  return total / 5;
}
