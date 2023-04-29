
// Include the libraries for the OLED display and the Geiger counter
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>

// Define the pins for the OLED display
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

// Define the pins for the Geiger counter
#define GEIGER_INT 2 // Interrupt pin
#define GEIGER_VCC 3 // Power pin

// Define some constants for the Geiger counter
#define MAX_PERIOD 60000 // Maximum period for counting (ms)
#define CONVERSION_FACTOR 151.0 // Conversion factor from CPM to uSv/h for M4011 tube

// Define some variables for the Geiger counter
volatile unsigned long counts = 0; // Number of counts
unsigned long previousMillis = 0; // Previous time stamp
unsigned long period = 0; // Current period
float cpm = 0; // Counts per minute
float dose = 0; // Dose rate in uSv/h

// Define some constants for the OLED display
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define GRAPH_HEIGHT 24 // Graph height, in pixels
#define GRAPH_WIDTH 120 // Graph width, in pixels
#define GRAPH_X 4 // Graph x position, in pixels
#define GRAPH_Y 8 // Graph y position, in pixels

// Define some variables for the OLED display
int knobValue = 0; // Value of the knob connected to A1 pin
int timeBase = 0; // Time base of the graph, in seconds
int pixelIndex = 0; // Current pixel index of the graph
int pixelValue[GRAPH_WIDTH]; // Array of pixel values for the graph

// Interrupt service routine for counting pulses from the Geiger tube
void countPulse() {
  counts++; // Increment the count by one
}

// Setup function runs once when the Arduino starts
void setup() {
  Serial.begin(9600); // Start serial communication at 9600 baud rate
  
  pinMode(GEIGER_VCC, OUTPUT); // Set the Geiger power pin as output
  digitalWrite(GEIGER_VCC, HIGH); // Turn on the Geiger power
  
  pinMode(GEIGER_INT, INPUT); // Set the Geiger interrupt pin as input
  attachInterrupt(digitalPinToInterrupt(GEIGER_INT), countPulse, FALLING); // Attach an interrupt to count pulses
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Initialize the OLED display with I2C address 0x3C
  
  display.clearDisplay(); // Clear the display buffer
  display.setTextSize(1); // Set text size to 1
  display.setTextColor(WHITE); // Set text color to white
  
}

// Loop function runs repeatedly after setup is done
void loop() {
  
  unsigned long currentMillis = millis(); // Get the current time in milliseconds
  
  if (currentMillis - previousMillis >= MAX_PERIOD) { // If the maximum period is reached
    
    period = currentMillis - previousMillis; // Calculate the actual period
    
    cpm = counts / (period / 60000.0); // Calculate the counts per minute
    
    dose = cpm / CONVERSION_FACTOR; // Calculate the dose rate in uSv/h
    
    Serial.print("CPM: "); // Print CPM to serial monitor
    Serial.println(cpm);
    
    Serial.print("Dose: "); // Print dose to serial monitor
    Serial.print(dose);
    Serial.println(" uSv/h");
    
    previousMillis = currentMillis; // Update the previous time stamp
    
    counts = 0; // Reset the counts
    
    updateGraph(); // Update the graph on the OLED display
    
    display.setCursor(0,0); // Set cursor to top left corner
    display.print(cpm, 1); // Print CPM value with one decimal place
    
    display.display(); // Display the buffer on screen
    
    display.fillRect(0,8,GRAPH_X-1,GRAPH_HEIGHT+1,BLACK); // Clear the area below CPM value
    
    
    
  }
  
}

// Function to update the graph on the OLED display
void updateGraph() {
  
  knobValue = analogRead(A1); // Read the value of the knob connected to A1 pin
  
  timeBase = map(knobValue, 0, 1023, 1, 60); // Map the knob value to a time base between 1 and 60 seconds
  
  pixelIndex++; // Increment the pixel index by one
  
  if (pixelIndex >= GRAPH_WIDTH) { // If the pixel index reaches the graph width
    pixelIndex = 0; // Reset the pixel index to zero
  }
  
  pixelValue[pixelIndex] = map(cpm, 0, 1000, GRAPH_HEIGHT, 0); // Map the CPM value to a pixel value between graph height and zero
  
  display.drawLine(GRAPH_X + pixelIndex, GRAPH_Y, GRAPH_X + pixelIndex, GRAPH_Y + GRAPH_HEIGHT, BLACK); // Clear the current column of the graph
  
  display.drawPixel(GRAPH_X + pixelIndex, GRAPH_Y + pixelValue[pixelIndex], WHITE); // Draw a pixel at the current CPM value
  
  display.drawRect(GRAPH_X - 1, GRAPH_Y - 1, GRAPH_WIDTH + 2, GRAPH_HEIGHT + 2, WHITE); // Draw a rectangle around the graph
  
  display.setCursor(GRAPH_X + GRAPH_WIDTH - 16,0); // Set cursor to top right corner
  display.print(timeBase); // Print the time base value
  display.print("s"); // Print the unit of time base
  
}


//Source: Conversation with Bing, 4/29/2023
//(1) Arduino DIY Geiger Counter : 12 Steps (with Pictures) - Instructables. https://www.instructables.com/Arduino-DIY-Geiger-Counter/.
//(2) Geiger Counter With Arduino Uno - Instructables. https://www.instructables.com/Geiger-Counter/.
//(3) Arduino Geiger Counter : 6 Steps - Instructables. https://www.instructables.com/Arduino-Geiger-Counter/.
//(4) Arduino Library & Examples | Monochrome OLED Breakouts | Adafruit .... https://learn.adafruit.com/monochrome-oled-breakouts/arduino-library-and-examples.
//(5) Monochrome 128x32 I2C OLED graphic display : ID 931 - Adafruit Industries. https://www.adafruit.com/product/931.
//(6) Wiring 128x32 I2C Display | Monochrome OLED Breakouts | Adafruit .... https://learn.adafruit.com/monochrome-oled-breakouts/wiring-128x32-i2c-display.
