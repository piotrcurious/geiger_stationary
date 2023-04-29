
// Include the libraries for the OLED display and the Geiger counter
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <Wire.h>

// Define the pins for the OLED display
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Define the pin for the Geiger counter
#define GEIGER_PIN 2 // Connect the VIN on the Geiger counter to this pin

// Define the pin for the knob
#define KNOB_PIN A1 // Connect the knob to this analog pin

// Create an object for the OLED display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Create some variables for the Geiger counter
volatile unsigned long counts = 0; // Variable for GM Tube events
unsigned long previousMillis = 0; // Variable for time measurement
unsigned long interval = 1000; // Interval to calculate CPM (milliseconds)
unsigned int cpm = 0; // Variable for counts per minute

// Create some variables for the rolling graph
const int numReadings = 64; // Number of readings to store in the buffer
int readings[numReadings]; // The buffer to store the readings
int readIndex = 0; // The index of the current reading
int total = 0; // The running total of the readings
int average = 0; // The average of the readings

// Create some variables for the knob
int knobValue = 0; // The value read from the knob
int knobMin = 0; // The minimum value of the knob
int knobMax = 1023; // The maximum value of the knob
int timeBase = 1; // The time base of the rolling graph (seconds)

void setup() {
  Serial.begin(9600); // Start serial communication
  
  // Initialize the OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true); // Don't proceed, loop forever
  }
  
  // Initialize the Geiger counter interrupt
  pinMode(GEIGER_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(GEIGER_PIN), tube_impulse, FALLING);
  
  // Initialize the rolling graph buffer
  for (int i = 0; i < numReadings; i++) {
    readings[i] = 0;
  }
}

void loop() {
  
  unsigned long currentMillis = millis(); // Get current time
  
  if (currentMillis - previousMillis >= interval) { // If interval has passed
    
    previousMillis = currentMillis; // Update previous time
    
    cpm = counts * (60000 / interval); // Calculate CPM
    
    counts = 0; // Reset counts
    
    Serial.print("CPM: "); // Print CPM to serial monitor
    Serial.println(cpm);
    
    total = total - readings[readIndex]; // Subtract old reading from total
    
    readings[readIndex] = cpm; // Store new reading in buffer
    
    total = total + readings[readIndex]; // Add new reading to total
    
    readIndex++; // Increment index
    
    if (readIndex >= numReadings) { // If index reaches end of buffer
      
      readIndex = 0; // Reset index
      
    }
    
    average = total / numReadings; // Calculate average
    
    knobValue = analogRead(KNOB_PIN); // Read knob value
    
    timeBase = map(knobValue, knobMin, knobMax, 1, 10); // Map knob value to time base
    
    display.clearDisplay(); // Clear display buffer
    
    display.setTextSize(1); // Set text size to normal
    
    display.setTextColor(SSD1306_WHITE); // Set text color to white
    
    display.setCursor(0,0); // Set cursor position to top left corner
    
    display.print("CPM: "); // Print CPM label
    
    display.print(cpm);

    display.print(" uSv/h: "); // Print uSv/h label
    
    display.print(cpm / 151.0, 2); // Print uSv/h value (assuming M4011 tube)
    
    display.print(" Time: "); // Print time base label
    
    display.print(timeBase); // Print time base value
    
    display.print("s"); // Print time unit
    
    display.drawLine(0, 16, 127, 16, SSD1306_WHITE); // Draw a horizontal line
    
    for (int i = 0; i < numReadings; i++) { // For each reading in the buffer
      
      int x = map(i, 0, numReadings - 1, 0, 127); // Map index to x coordinate
      
      int y = map(readings[i], 0, average * 2, 31, 17); // Map reading to y coordinate
      
      display.drawPixel(x, y, SSD1306_WHITE); // Draw a pixel at (x,y)
      
    }
    
    display.display(); // Show display buffer on the screen
    
  }
  
}

// Interrupt function for Geiger counter
void tube_impulse() {
  
  counts++; // Increment counts
  
}



//Source: Conversation with Bing, 4/29/2023
//(1) Arduino DIY Geiger Counter : 12 Steps (with Pictures) - Instructables. https://www.instructables.com/Arduino-DIY-Geiger-Counter/.
//(2) Geiger Counter With Arduino Uno - Instructables. https://www.instructables.com/Geiger-Counter/.
//(3) Coding Geiger Counter SEN-11345 ROHS - Arduino Forum. https://forum.arduino.cc/t/coding-geiger-counter-sen-11345-rohs/373463.
//(4) Arduino Library & Examples | Monochrome OLED Breakouts | Adafruit .... https://learn.adafruit.com/monochrome-oled-breakouts/arduino-library-and-examples.
//(5) Wiring 128x32 I2C Display | Monochrome OLED Breakouts | Adafruit .... https://learn.adafruit.com/monochrome-oled-breakouts/wiring-128x32-i2c-display.
//(6) Monochrome 128x32 I2C OLDER graphic display : ID 931 - Adafruit Industries. https://www.adafruit.com/product/931.
