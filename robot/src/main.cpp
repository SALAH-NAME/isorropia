#include <Arduino.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <MPU9250_asukiaaa.h>
#include <math.h>

// HC-06 Bluetooth connection
SoftwareSerial btSerial(10, 9); // RX (connect to HC-06 TX), TX (connect to HC-06 RX)

// MPU6500 IMU Initialization
MPU9250_asukiaaa mySensor(0x68);

// Motor Driver Pins (based on README.md)
// Motor A (Left Motor)
const int ENA = 3; // PWM Speed Control
const int IN1 = 4; // Direction 1
const int IN2 = 5; // Direction 2

// Motor B (Right Motor)
const int ENB = 6; // PWM Speed Control
const int IN3 = 8; // Direction 1
const int IN4 = 7; // Direction 2

// Variables tracking current operation
int currentLeftSpeed = 0;
int currentRightSpeed = 0;
unsigned long lastTelemetryTime = 0;

// Function to control one motor
// speed from -100 (full reverse) to 100 (full forward), 0 is stop
void setMotor(int speed, int enPin, int in1Pin, int in2Pin) {
  // Constrain speed to -100 ... 100 to prevent overflow logic errors
  speed = constrain(speed, -100, 100);

  if (speed == 0) {
    // Stop Motor
    digitalWrite(in1Pin, LOW);
    digitalWrite(in2Pin, LOW);
    analogWrite(enPin, 0);
  } else if (speed > 0) {
    // Forward
    digitalWrite(in1Pin, HIGH);
    digitalWrite(in2Pin, LOW);
    // map 0-100% to 0-255 PWM signal
    int pwmValue = map(speed, 0, 100, 0, 255);
    analogWrite(enPin, pwmValue);
  } else {
    // Reverse
    digitalWrite(in1Pin, LOW);
    digitalWrite(in2Pin, HIGH);
    // speed is negative, invert it to positive and map to PWM
    int pwmValue = map(-speed, 0, 100, 0, 255);
    analogWrite(enPin, pwmValue);
  }
}

void setup() {
  Serial.begin(115200);
  btSerial.begin(9600);
  
  // Initialize I2C and IMU
  Wire.begin();
  mySensor.beginAccel();
  mySensor.beginGyro();

  // Set motor pins as outputs
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Default to motors stopped
  setMotor(0, ENA, IN1, IN2);
  setMotor(0, ENB, IN3, IN4);

  Serial.println("Robot Ready. Motor and IMU initialized.");
  btSerial.println("Robot Ready.");
}

void loop() {
  // Expecting a command like: "<left_speed> <right_speed>\n"
  // For example: "50 50" (move forward half max speed)
  // Or: "-100 -100" (move backward at max speed)
  
  if (btSerial.available()) {
    // readStringUntil reads the incoming characters up to the newline '\n'
    String data = btSerial.readStringUntil('\n');
    data.trim(); // remove extra spaces/newlines

    // Parse the space-separated integer payload
    int spaceIndex = data.indexOf(' ');
    if (spaceIndex > 0) {
      String leftStr = data.substring(0, spaceIndex);
      String rightStr = data.substring(spaceIndex + 1);
      
      int leftSpeed = leftStr.toInt();
      int rightSpeed = rightStr.toInt();

      currentLeftSpeed = leftSpeed;
      currentRightSpeed = rightSpeed;

      Serial.print("Received Speed -> Left: ");
      Serial.print(currentLeftSpeed);
      Serial.print(", Right: ");
      Serial.println(currentRightSpeed);

      // Apply the speeds to the motors
      setMotor(currentLeftSpeed, ENA, IN1, IN2);
      setMotor(currentRightSpeed, ENB, IN3, IN4);
      
    } else if (data.length() > 0) {
      // Unrecognized format or single number... just stop.
      currentLeftSpeed = 0;
      currentRightSpeed = 0;
      setMotor(0, ENA, IN1, IN2);
      setMotor(0, ENB, IN3, IN4);
      Serial.print("Invalid command received, stopping motors: ");
      Serial.println(data);
    }
  }

  // Send telemetry data back to App every 1000ms
  if (millis() - lastTelemetryTime >= 1000) {
    lastTelemetryTime = millis();
    
    // Update Sensor
    mySensor.accelUpdate();
    // (Optional) if needed, update gyro: mySensor.gyroUpdate();

    float ax = mySensor.accelX();
    float ay = mySensor.accelY();
    float az = mySensor.accelZ();

    // Compute tilt angles
    float pitch = atan2(-ax, sqrt(ay * ay + az * az)) * 180.0 / PI;
    float roll  = atan2(ay, az) * 180.0 / PI;

    // Send formatted telemetry to Bluetooth "L:x R:x Pitch:x Roll:x"
    btSerial.print("TELEMETRY L:");
    btSerial.print(currentLeftSpeed);
    btSerial.print(" R:");
    btSerial.print(currentRightSpeed);
    btSerial.print(" Pitch:");
    btSerial.print(pitch);
    btSerial.print(" Roll:");
    btSerial.println(roll);
  }
}