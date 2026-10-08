/*
 * Autonomous Multi-Point Water Quality Survey Robot
 *
 * Reconstructed reference implementation
 *
 * This code was recreated based on the project's documented
 * hardware and operating procedure. It is not the original
 * source code used during the project demonstration.
 *
 * Main operation:
 * 1. Robot moves toward the next survey station.
 * 2. TTP223 touch sensor detects the station.
 * 3. Motors stop.
 * 4. Servo lowers the DS18B20 temperature sensor.
 * 5. pH, temperature, and turbidity readings are collected.
 * 6. Measurements are stored on the microSD card.
 * 7. Servo raises the temperature sensor.
 * 8. Robot resumes movement.
 *
 * NOTE:
 * Pin assignments and sensor calibration values should be
 * verified against the original circuit before actual deployment.
 */

#include <SPI.h>
#include <SD.h>
#include <Servo.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// PIN DEFINITIONS

// Water temperature sensor (DS18B20)
#define ONE_WIRE_BUS 22

// pH sensor
#define PH_PIN A0

// Turbidity sensor
#define TURBIDITY_PIN A1

// TTP223 touch sensor
#define TOUCH_PIN 23

// Servo
#define SERVO_PIN 24

// SD card module
#define SD_CS_PIN 53

// BTS7960 Motor Driver 1
#define RPWM1 5
#define LPWM1 6

// BTS7960 Motor Driver 2
#define RPWM2 7
#define LPWM2 8

// OBJECTS

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature temperatureSensor(&oneWire);

Servo sensorServo;

// SERVO POSITIONS


// Adjust these angles according to the actual robot
int SERVO_UP = 20;
int SERVO_DOWN = 90;

// SETUP

void setup() {

  Serial.begin(9600);

  // Motor pins
  pinMode(RPWM1, OUTPUT);
  pinMode(LPWM1, OUTPUT);

  pinMode(RPWM2, OUTPUT);
  pinMode(LPWM2, OUTPUT);

  // Touch sensor
  pinMode(TOUCH_PIN, INPUT);

  // Servo
  sensorServo.attach(SERVO_PIN);
  sensorServo.write(SERVO_UP);

  // Temperature sensor
  temperatureSensor.begin();

  // SD card
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("SD card initialization failed!");
  } 
  else {
    Serial.println("SD card initialized.");

    // Create file if it doesn't exist
    if (!SD.exists("data.csv")) {
      File dataFile = SD.open("data.csv", FILE_WRITE);

      if (dataFile) {
        dataFile.println("Station,pH,Temperature_C,Turbidity");
        dataFile.close();
      }
    }
  }

  Serial.println("Robot ready.");

  // Start moving
  moveForward();
}

// MAIN LOOP

void loop() {

  // Check whether the touch sensor detected a station
  if (digitalRead(TOUCH_PIN) == HIGH) {

    Serial.println("Station detected!");

    // Stop the robot
    stopMotors();

    delay(500);

    // Perform water-quality measurement
    takeMeasurement();

    // Resume movement
    moveForward();

    // Prevent multiple detections from the same station
    delay(1000);
  }
}

// WATER QUALITY MEASUREMENT

void takeMeasurement() {

  Serial.println("Starting measurement...");

  // Lower temperature sensor into the water
  
  Serial.println("Lowering temperature sensor...");

  sensorServo.write(SERVO_DOWN);

  // Give the servo time to move
  delay(1500);

  // Give the sensors time to stabilize
  delay(2000);

  // Read temperature

  temperatureSensor.requestTemperatures();

  float temperature =
    temperatureSensor.getTempCByIndex(0);

  // Read pH sensor
  
  int phRaw = analogRead(PH_PIN);

  // NOTE:
  // This is only a placeholder conversion.
  // Actual pH conversion requires calibration using
  // known pH buffer solutions.

  float voltage = phRaw * (5.0 / 1023.0);

  float pH = 7.0 + ((2.5 - voltage) / 0.18);

  // Read turbidity sensor

  int turbidityRaw = analogRead(TURBIDITY_PIN);

  // Store the raw sensor value.
  // Actual NTU conversion depends on calibration.

  float turbidity = turbidityRaw;

  // Display readings
  
  Serial.println("-------------------------");

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" C");

  Serial.print("pH: ");
  Serial.println(pH);

  Serial.print("Turbidity: ");
  Serial.println(turbidity);

  Serial.println("-------------------------");

  // Save data to SD card

  saveData(pH, temperature, turbidity);

  // Raise temperature sensor

  Serial.println("Raising temperature sensor...");

  sensorServo.write(SERVO_UP);

  delay(1500);

  Serial.println("Measurement complete.");
}
// SAVE DATA TO SD CARD

void saveData(float pH, float temperature, float turbidity) {

  File dataFile = SD.open("data.csv", FILE_WRITE);

  if (dataFile) {

    // Use millis() as a simple measurement identifier
    dataFile.print(millis());
    dataFile.print(",");

    dataFile.print(pH);
    dataFile.print(",");

    dataFile.print(temperature);
    dataFile.print(",");

    dataFile.println(turbidity);

    dataFile.close();

    Serial.println("Data saved to SD card.");

  } 
  else {

    Serial.println("Error opening data.csv");
  }
}

// MOTOR CONTROL

void moveForward() {

  // Motor 1
  analogWrite(RPWM1, 180);
  analogWrite(LPWM1, 0);

  // Motor 2
  analogWrite(RPWM2, 180);
  analogWrite(LPWM2, 0);

  Serial.println("Robot moving forward.");
}


void stopMotors() {

  analogWrite(RPWM1, 0);
  analogWrite(LPWM1, 0);

  analogWrite(RPWM2, 0);
  analogWrite(LPWM2, 0);

  Serial.println("Motors stopped.");
}