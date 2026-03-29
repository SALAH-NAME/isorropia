# Isorropia
auto-balancing robot system.

## HARDWARE CONFIGURATION

### Components:

- Arduino Uno
- MPU6500 (6-axis IMU sensor)
- HC-06 (Bluetooth module)
- L298N (Motor driver)
- Two DC motors with wheels

### Pin Connections (MUST USE EXACTLY):

L298N Motor Driver:
- ENA → Pin 3 (PWM - Motor A Enable)
- IN1 → Pin 4 (Motor A Direction 1)
- IN2 → Pin 5 (PWM - Motor A Direction 2)
- IN3 → Pin 8 (PWM - Motor B Direction 1)
- IN4 → Pin 7 (Motor B Direction 2)
- ENB → Pin 6 (PWM - Motor B Enable)

MPU6500 IMU: (Use "MPU9250_asukiaaa" library)
- SCL → A5 (I2C Clock)
- SDA → A4 (I2C Data)
- INT → Pin 2 (Data Ready Interrupt - optional but recommended) (Not used currently)

HC-06 Bluetooth:
- TXD → Pin 10 (Arduino RX via SoftwareSerial)
- RXD → Pin 9 (Arduino TX via SoftwareSerial)

