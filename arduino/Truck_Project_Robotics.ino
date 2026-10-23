#include <AFMotor.h>
#include <SoftwareSerial.h>

// =====================================================
// BLUETOOTH
// HC-05 TXD -> Arduino A5
// HC-05 RXD -> Arduino A4
// =====================================================
SoftwareSerial Bluetooth(A5, A4);

// =====================================================
// ULTRASONIC SENSOR
// =====================================================
const byte ECHO_PIN = A2;
const byte TRIG_PIN = A1;

// =====================================================
// BUZZER
// =====================================================
const byte BUZZER_PIN = A3;

// =====================================================
// MOTORS
// =====================================================
AF_DCMotor motor1(1);
AF_DCMotor motor2(2);

// =====================================================
// SETTINGS
// =====================================================
const byte MOTOR_SPEED = 180;
const int WARNING_DISTANCE = 30;
const int STOP_DISTANCE = 10;

// =====================================================
// BUZZER
// =====================================================
unsigned long lastBeepTime = 0;
bool buzzerState = false;

// =====================================================
// SETUP
// =====================================================
void setup() {
  Serial.begin(9600);
  Bluetooth.begin(9600);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);

  pinMode(BUZZER_PIN, OUTPUT);
  noTone(BUZZER_PIN);

  motor1.setSpeed(MOTOR_SPEED);
  motor2.setSpeed(MOTOR_SPEED);

  // Start stopped
  stopTruck();

  Serial.println("==============================");
  Serial.println("TRUCK READY");
  Serial.println("==============================");
}

// =====================================================
// MAIN LOOP
// =====================================================
void loop() {
  // Bluetooth controller commands:
  // UP = F, DOWN = B, LEFT = L, RIGHT = R, STOP = S
  if (Bluetooth.available()) {
    char command = Bluetooth.read();

    if (command != '\n' && command != '\r') {
      Serial.print("Received: ");
      Serial.println(command);

      if (command == 'F' || command == 'f') {
        moveForward();
      } else if (command == 'B' || command == 'b') {
        moveBackward();
      } else if (command == 'L' || command == 'l') {
        turnLeft();
      } else if (command == 'R' || command == 'r') {
        turnRight();
      } else if (command == 'S' || command == 's') {
        stopTruck();
      }
    }
  }

  int distance = getDistance();

  // Very close: stop the truck and sound a fast alert.
  if (distance > 0 && distance <= STOP_DISTANCE) {
    stopTruck();
    tone(BUZZER_PIN, 2000);
    delay(80);
    noTone(BUZZER_PIN);
  }
  // Warning zone: beep faster as an obstacle gets closer.
  else if (distance > STOP_DISTANCE && distance <= WARNING_DISTANCE) {
    warningBeep(distance);
  }
  // Safe zone or no echo.
  else {
    noTone(BUZZER_PIN);
    buzzerState = false;
  }

  delay(20);
}

// =====================================================
// ULTRASONIC DISTANCE
// =====================================================
int getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 25000);

  if (duration == 0) {
    return -1; // No echo detected
  }

  int distance = duration * 0.0343 / 2;
  return distance;
}

// =====================================================
// WARNING BEEP
// =====================================================
void warningBeep(int distance) {
  int interval;

  if (distance <= 15) {
    interval = 120;
  } else if (distance <= 20) {
    interval = 250;
  } else {
    interval = 500;
  }

  if (millis() - lastBeepTime >= (unsigned long)interval) {
    lastBeepTime = millis();
    buzzerState = !buzzerState;

    if (buzzerState) {
      tone(BUZZER_PIN, 2000);
    } else {
      noTone(BUZZER_PIN);
    }
  }
}

// =====================================================
// MOVEMENT
// =====================================================
void moveForward() {
  motor1.setSpeed(MOTOR_SPEED);
  motor2.setSpeed(MOTOR_SPEED);
  motor1.run(FORWARD);
  motor2.run(FORWARD);
}

void moveBackward() {
  motor1.setSpeed(MOTOR_SPEED);
  motor2.setSpeed(MOTOR_SPEED);
  motor1.run(BACKWARD);
  motor2.run(BACKWARD);
}

void turnLeft() {
  motor1.setSpeed(MOTOR_SPEED);
  motor2.setSpeed(MOTOR_SPEED);
  motor1.run(BACKWARD);
  motor2.run(FORWARD);
}

void turnRight() {
  motor1.setSpeed(MOTOR_SPEED);
  motor2.setSpeed(MOTOR_SPEED);
  motor1.run(FORWARD);
  motor2.run(BACKWARD);
}

void stopTruck() {
  motor1.run(RELEASE);
  motor2.run(RELEASE);
  noTone(BUZZER_PIN);
  buzzerState = false;
}
