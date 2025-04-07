#define VRX_PIN       34    // X-axis analog pin
#define VRY_PIN       35    // Y-axis analog pin

// Thresholds for X-axis (left-right)
#define LEFT_THRESHOLD    1000
#define RIGHT_THRESHOLD   4000

// Thresholds for Y-axis (up-down)
#define UP_THRESHOLD      1000
#define DOWN_THRESHOLD    4000

// Command definitions
#define COMMAND_NO        0
#define COMMAND_LEFT      1
#define COMMAND_RIGHT     2
#define COMMAND_UP        3
#define COMMAND_DOWN      4

int valueX = 0;
int valueY = 0;
int command = COMMAND_NO;

void setup() {
  Serial.begin(9600);
}

void loop() {
  // Read X and Y analog values
  valueX = analogRead(VRX_PIN);
  valueY = analogRead(VRY_PIN);
  
  // Print raw values
  Serial.print("X: ");
  Serial.print(valueX);
  Serial.print(" Y: ");
  Serial.print(valueY);
  Serial.print(" - ");
  
  // First check for X-axis movement (prioritizing left-right)
  if (valueX < LEFT_THRESHOLD) {
    command = COMMAND_RIGHT;
  } 
  else if (valueX > RIGHT_THRESHOLD) {
    command = COMMAND_LEFT;
  }
  // If no X movement, check for Y-axis movement
  else if (valueY < UP_THRESHOLD) {
    command = COMMAND_DOWN;
  }
  else if (valueY > DOWN_THRESHOLD) {
    command = COMMAND_UP;
  }
  else {
    command = COMMAND_NO;
  }
  
  // Handle the command
  switch (command) {
    case COMMAND_LEFT:
      Serial.println("COMMAND LEFT");
      // TODO: Add your task here for left command
      break;
      
    case COMMAND_RIGHT:
      Serial.println("COMMAND RIGHT");
      // TODO: Add your task here for right command
      break;
      
    case COMMAND_UP:
      Serial.println("COMMAND UP");
      // TODO: Add your task here for up command
      break;
      
    case COMMAND_DOWN:
      Serial.println("COMMAND DOWN");
      // TODO: Add your task here for down command
      break;
      
    case COMMAND_NO:
      Serial.println("NO COMMAND");
      break;
  }
  
  delay(500);
}