#include <esp_now.h>
#include <WiFi.h>

// Struct to match transmitter's data structure
typedef struct {
  int valueX;
  int valueY;
} struct_message;

// Motor A connections
#define MOTOR_A_IN1 12
#define MOTOR_A_IN2 14
#define MOTOR_A_PWM_CHANNEL 0

// Motor B connections
#define MOTOR_B_IN3 27
#define MOTOR_B_IN4 26
#define MOTOR_B_PWM_CHANNEL 1

// PWM properties
#define PWM_FREQ 5000
#define PWM_RESOLUTION 8

// Joystick data receiver
struct_message receivedData;

// Function prototypes
void setupMotors();
void setMotorSpeed(int channel, int pin1, int pin2, int speed);
void processJoystickData(struct_message* data);

// ESP-NOW receive callback
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  memcpy(&receivedData, incomingData, sizeof(receivedData));
  
  // Debug print
  Serial.printf("Received - X: %d, Y: %d\n", receivedData.valueX, receivedData.valueY);
  
  // Process joystick data
  processJoystickData(&receivedData);
}

void setupMotors() {
  // Configure PWM for motor control
  ledcSetup(MOTOR_A_PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(MOTOR_B_PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  
  // Attach PWM channels to pins
  ledcAttachPin(MOTOR_A_IN1, MOTOR_A_PWM_CHANNEL);
  ledcAttachPin(MOTOR_A_IN2, MOTOR_A_PWM_CHANNEL + 1);
  ledcAttachPin(MOTOR_B_IN3, MOTOR_B_PWM_CHANNEL);
  ledcAttachPin(MOTOR_B_IN4, MOTOR_B_PWM_CHANNEL + 1);
}

void setMotorSpeed(int channel, int pin1, int pin2, int speed) {
  // Ensure speed is within -255 to 255 range
  speed = constrain(speed, -255, 255);
  
  if (speed > 0) {
    // Forward
    ledcWrite(channel, speed);
    ledcWrite(channel + 1, 0);
  } else if (speed < 0) {
    // Backward
    ledcWrite(channel, 0);
    ledcWrite(channel + 1, -speed);
  } else {
    // Stop
    ledcWrite(channel, 0);
    ledcWrite(channel + 1, 0);
  }
}

void processJoystickData(struct_message* data) {
  // Deadzone to prevent tiny movements
  const int DEADZONE = 10;
  
  // Extract X and Y values
  int x = data->valueX;
  int y = data->valueY;
  
  // Apply deadzone
  x = abs(x) > DEADZONE ? x : 0;
  y = abs(y) > DEADZONE ? y : 0;
  
  // Mixing algorithm for tank-style differential drive
  int leftMotorSpeed = y + x;
  int rightMotorSpeed = y - x;
  
  // Normalize speeds to prevent overflow
  int maxSpeed = max(abs(leftMotorSpeed), abs(rightMotorSpeed));
  if (maxSpeed > 255) {
    leftMotorSpeed = map(leftMotorSpeed, -maxSpeed, maxSpeed, -255, 255);
    rightMotorSpeed = map(rightMotorSpeed, -maxSpeed, maxSpeed, -255, 255);
  }
  
  // Set motor speeds
  setMotorSpeed(MOTOR_A_PWM_CHANNEL, MOTOR_A_IN1, MOTOR_A_IN2, leftMotorSpeed);
  setMotorSpeed(MOTOR_B_PWM_CHANNEL, MOTOR_B_IN3, MOTOR_B_IN4, rightMotorSpeed);
}

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  
  // Setup motor control
  setupMotors();
  
  // Set WiFi to station mode
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  
  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW initialization failed");
    while (1) {
      delay(1000);
    }
  }
  
  // Register receive callback
  esp_now_register_recv_cb(OnDataRecv);
  
  Serial.println("ESP-NOW Motor Control Receiver Ready");
}

void loop() {
  // Nothing needed here - everything is interrupt-driven
  delay(10);  // Small delay to prevent watchdog timer issues
}