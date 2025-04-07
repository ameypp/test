// #include <Wire.h>
// #include <LiquidCrystal_I2C.h>

// // LCD Configuration
// LiquidCrystal_I2C lcd(0x27, 16, 2);  // Set the LCD address to 0x27 for a 16x2 display

// // Rotary Encoder Pins
// #define ENCODER_CLK 32
// #define ENCODER_DT  33

// // Joystick Pins
// #define VRX_PIN 34  // X-axis analog pin
// #define VRY_PIN 35  // Y-axis analog pin

// // Thresholds for X-axis (left-right)
// #define LEFT_THRESHOLD 1000
// #define RIGHT_THRESHOLD 4000

// // Thresholds for Y-axis (up-down)
// #define UP_THRESHOLD 1000
// #define DOWN_THRESHOLD 4000

// // Command definitions
// #define COMMAND_NO 0
// #define COMMAND_LEFT 1
// #define COMMAND_RIGHT 2
// #define COMMAND_UP 3
// #define COMMAND_DOWN 4

// // Menu Configuration
// #define MENU_SIZE 5
// const char* menuItems[MENU_SIZE] = {
//   "Temperature",
//   "GPS Coordinates",
//   "Humidity",
//   "Pressure",
//   "Altitude"
// };

// // Variables
// int menuIndex = 0;
// int lastCLK = LOW;
// int valueX = 0;
// int valueY = 0;
// int command = COMMAND_NO;

// void setup() {
//   // Initialize serial
//   Serial.begin(9600);

//   // Initialize LCD
//   lcd.begin();
//   lcd.backlight();

//   // Show startup messages
//   lcd.clear();
//   lcd.print("Initializing...");
//   delay(1000);

//   lcd.clear();
//   lcd.print("Establishing");
//   lcd.setCursor(0, 1);
//   lcd.print("Connection...");
//   delay(1000);

//   // Configure encoder pins
//   pinMode(ENCODER_CLK, INPUT);
//   pinMode(ENCODER_DT, INPUT);

//   // Show initial menu
//   displayMenu();
// }

// void loop() {
//   // Read encoder for menu navigation
//   int currentCLK = digitalRead(ENCODER_CLK);

//   // Detect rotation
//   if (currentCLK != lastCLK && currentCLK == HIGH) {
//     if (digitalRead(ENCODER_DT) != currentCLK) {
//       // Turned counterclockwise
//       menuIndex = (menuIndex + 1) % MENU_SIZE;
//     } else {
//       // Turned clockwise
//       menuIndex = (menuIndex - 1 + MENU_SIZE) % MENU_SIZE;
//     }
//     displayMenu();
//   }
//   lastCLK = currentCLK;

//   // Read joystick values
//   valueX = analogRead(VRX_PIN);
//   valueY = analogRead(VRY_PIN);

//   // First check for X-axis movement (prioritizing left-right)
//   if (valueX < LEFT_THRESHOLD) {
//     command = COMMAND_LEFT;
//   } else if (valueX > RIGHT_THRESHOLD) {
//     command = COMMAND_RIGHT;
//   }
//   // If no X movement, check for Y-axis movement
//   else if (valueY < UP_THRESHOLD) {
//     command = COMMAND_UP;
//   } else if (valueY > DOWN_THRESHOLD) {
//     command = COMMAND_DOWN;
//   } else {
//     command = COMMAND_NO;
//   }

//   // Handle the command
//   switch (command) {
//     case COMMAND_LEFT:
//       Serial.println("COMMAND LEFT");
//       break;

//     case COMMAND_RIGHT:
//       Serial.println("COMMAND RIGHT");
//       break;

//     case COMMAND_UP:
//       Serial.println("COMMAND UP");
//       break;

//     case COMMAND_DOWN:
//       Serial.println("COMMAND DOWN");
//       break;
//   }

//   delay(100);  // Small delay to avoid flooding
// }

// void displayMenu() {
//   lcd.clear();

//   // Display parameter name on top line
//   lcd.setCursor(0, 0);
//   lcd.print("> ");
//   lcd.print(menuItems[menuIndex]);

//   // Display reading on bottom line
//   lcd.setCursor(2, 1);
//   lcd.print("Value: NaN");
// }

// ESP-NOW Transmitter Code for ESP32 (Board Version 2.0.14)
// Sends commands to receiver at MAC: F8:B3:B7:26:5D:28

#include <esp_now.h>
#include <WiFi.h>

// Receiver MAC Address
uint8_t receiverMacAddress[] = {0xF8, 0xB3, 0xB7, 0x26, 0x5D, 0x28};

// Callback function when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\n----- ESP-NOW Transmitter Starting -----");

  // Initialize WiFi in Station Mode
  WiFi.mode(WIFI_STA);

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  } else {
    Serial.println("ESP-NOW initialized successfully");
  }

  // Register send callback
  esp_now_register_send_cb(OnDataSent);

  // Add receiver as peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverMacAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  } else {
    Serial.println("Peer added successfully");
  }
}

void loop() {
  static int counter = 0;
  String message = "Command " + String(counter);

  // Convert message to char array
  char msg[32];
  message.toCharArray(msg, sizeof(msg));

  Serial.print("Sending: ");
  Serial.println(msg);

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(receiverMacAddress, (uint8_t *) &msg, sizeof(msg));
  if (result != ESP_OK) {
    Serial.println("Error sending the data");
  }

  counter++;
  delay(2000);  // Send a new message every 2 seconds
}

