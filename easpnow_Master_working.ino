// #include <Wire.h>
// #include <LiquidCrystal_I2C.h>
// #include <esp_now.h>
// #include <WiFi.h>

// // Constants
// #define TRANSMISSION_DELAY_MS 200
// #define LCD_ADDRESS 0x27
// #define LCD_COLUMNS 16
// #define LCD_ROWS 2

// // Pin Definitions
// #define VRX_PIN 34
// #define VRY_PIN 35

// // Receiver MAC Address
// uint8_t receiverMacAddress[] = {0xF8, 0xB3, 0xB7, 0x26, 0x5D, 0x28};

// // Data structure (must be declared globally)
// typedef struct {
//   int valueX;
//   int valueY;
// } struct_message;

// LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);

// void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
//   Serial.print(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
// }

// void setup() {
//   Serial.begin(115200);
//   lcd.begin();
//   lcd.backlight();
//   lcd.print("Initializing...");

//   WiFi.mode(WIFI_STA);
//   WiFi.disconnect();
  
//   if (esp_now_init() != ESP_OK) {
//     lcd.clear();
//     lcd.print("ESP-NOW Init Fail");
//     while(1) delay(1000);
//   }

//   esp_now_register_send_cb(OnDataSent);

//   esp_now_peer_info_t peerInfo;
//   memset(&peerInfo, 0, sizeof(peerInfo));
//   memcpy(peerInfo.peer_addr, receiverMacAddress, 6);
//   peerInfo.channel = 0;
//   peerInfo.encrypt = false;
  
//   if (esp_now_is_peer_exist(receiverMacAddress)) {
//     esp_now_del_peer(receiverMacAddress);
//   }

//   if (esp_now_add_peer(&peerInfo) != ESP_OK) {
//     lcd.clear();
//     lcd.print("Peer Add Failed");
//     while(1) delay(1000);
//   }

//   lcd.clear();
//   lcd.print("Ready to send");
// }

// void loop() {
//   struct_message joystickData;  // Now using the globally defined struct
  
//   joystickData.valueX = analogRead(VRX_PIN);
//   joystickData.valueY = analogRead(VRY_PIN);

//   esp_err_t result = esp_now_send(receiverMacAddress, (uint8_t *)&joystickData, sizeof(joystickData));
  
//   if (result == ESP_OK) {
//     Serial.printf("Sent X:%d Y:%d\n", joystickData.valueX, joystickData.valueY);
//   } else {
//     Serial.println("Send Error");
//   }
  
//   delay(200);
// }

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <esp_now.h>
#include <WiFi.h>

// Constants
#define TRANSMISSION_DELAY_MS 200
#define LCD_ADDRESS 0x27
#define LCD_COLUMNS 16
#define LCD_ROWS 2
#define MENU_SIZE 5
#define ENCODER_CLK 32
#define ENCODER_DT 33
#define ENCODER_BTN 25
#define VRX_PIN 34
#define VRY_PIN 35

// Receiver MAC Address
uint8_t receiverMacAddress[] = {0x3C, 0x8A, 0x1F, 0x5D, 0x13, 0x8C};

// Menu System
const char* menuItems[MENU_SIZE] = {
  "Temperature",
  "GPS Coordinates",
  "Humidity",
  "Pressure",
  "Altitude"
};

// Data structure
typedef struct {
  int valueX;
  int valueY;
  float parameters[MENU_SIZE]; // Array to hold all parameters
} struct_message;

LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);
struct_message joystickData;
int menuIndex = 0;
int lastEncoderState = LOW;
bool menuActive = true;
unsigned long lastDisplayUpdate = 0;
bool displayNeedsUpdate = true;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void initializeESPNOW() {
  WiFi.mode(WIFI_STA);
  
  if (esp_now_init() != ESP_OK) {
    lcd.clear();
    lcd.print("ESP-NOW Init Fail");
    while(1);
  }

  esp_now_register_send_cb(OnDataSent);

  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, receiverMacAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    lcd.clear();
    lcd.print("Peer Add Failed");
    while(1);
  }
}

void updateEncoder() {
  int currentStateCLK = digitalRead(ENCODER_CLK);
  if (currentStateCLK != lastEncoderState && currentStateCLK == HIGH) {
    if (digitalRead(ENCODER_DT) == currentStateCLK) {
      menuIndex = (menuIndex + 1) % MENU_SIZE;
    } else {
      menuIndex = (menuIndex - 1 + MENU_SIZE) % MENU_SIZE;
    }
    displayNeedsUpdate = true;
  }
  lastEncoderState = currentStateCLK;
}

void displayMenu() {
  if (!displayNeedsUpdate) return;
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(">");
  lcd.print(menuItems[menuIndex]);
  
  lcd.setCursor(0, 1);
  lcd.print("Value: ");
  
  if (isnan(joystickData.parameters[menuIndex])) {
    lcd.print("NaN");
  } else {
    lcd.print(joystickData.parameters[menuIndex]);
  }
  
  displayNeedsUpdate = false;
  lastDisplayUpdate = millis();
}

void setup() {
  Serial.begin(115200);
  
  // Initialize LCD
  lcd.begin();
  lcd.backlight();
  lcd.print("Initializing...");
  
  // Initialize Encoder
  pinMode(ENCODER_CLK, INPUT);
  pinMode(ENCODER_DT, INPUT);
  pinMode(ENCODER_BTN, INPUT_PULLUP);
  
  // Initialize ESP-NOW
  initializeESPNOW();
  
  // Initialize parameters with NaN
  for (int i = 0; i < MENU_SIZE; i++) {
    joystickData.parameters[i] = NAN;
  }
  
  displayNeedsUpdate = true;
}

void loop() {
  // Read encoder
  updateEncoder();
  
  // Read joystick
  joystickData.valueX = analogRead(VRX_PIN);
  joystickData.valueY = analogRead(VRY_PIN);
  
  // Send data
  esp_err_t result = esp_now_send(receiverMacAddress, (uint8_t *)&joystickData, sizeof(joystickData));
  
  if (result == ESP_OK) {
    Serial.printf("Sent X:%d Y:%d\n", joystickData.valueX, joystickData.valueY);
  } else {
    Serial.println("Send Error");
  }
  
  // Button press to toggle menu/send mode
  if (digitalRead(ENCODER_BTN) == LOW) {
    delay(50); // Debounce
    menuActive = !menuActive;
    displayNeedsUpdate = true;
    while(digitalRead(ENCODER_BTN) == LOW); // Wait for release
  }
  
  // Update display only when needed
  if (menuActive && (displayNeedsUpdate || (millis() - lastDisplayUpdate > 1000))) {
    displayMenu();
  } else if (!menuActive) {
    lcd.clear();
    //lcd.noBacklight(); // Optional: turn off backlight in send mode
  }
  
  delay(TRANSMISSION_DELAY_MS);
}