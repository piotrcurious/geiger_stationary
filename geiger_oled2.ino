
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
#define GEIGER_PIN 2 // Interrupt pin for the Geiger counter

// Define the pin for the knob
#define KNOB_PIN A1 // Analog pin for the knob

// Create an object for the OLED display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Create some variables for the Geiger counter
volatile unsigned long counts = 0; // Counts per second
volatile unsigned long cpm = 0; // Counts per minute
unsigned long previousMillis = 0; // Previous time in milliseconds
unsigned long interval = 1000; // Interval to update counts per second

// Create some variables for the knob
int knobValue = 0; // Value read from the knob
int timeBase = 10; // Time base for the rolling graph in seconds

// Create some variables for the rolling graph
int graphX = 0; // X position of the graph
int graphY = 0; // Y position of the graph
int graphW = SCREEN_WIDTH; // Width of the graph
int graphH = SCREEN_HEIGHT - 8; // Height of the graph
int graphMax = 100; // Maximum value of the graph

// Create an array to store the graph data
int graphData[SCREEN_WIDTH];

// Interrupt service routine for the Geiger counter
void geigerISR() {
  counts++; // Increment counts by one
}

void setup() {
  Serial.begin(9600); // Start serial communication
  
  // Initialize the OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true); // Don't proceed, loop forever
  }
  
  // Clear the display buffer and set text size and color
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Attach an interrupt to the Geiger counter pin
  pinMode(GEIGER_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(GEIGER_PIN), geigerISR, FALLING);
}

void loop() {
  unsigned long currentMillis = millis(); // Get current time in milliseconds
  
  // Update counts per second every interval
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis; // Save current time
    
    cpm = (cpm * 59 + counts * 60) / 60; // Calculate counts per minute using exponential moving average
    
    noInterrupts(); // Disable interrupts while updating counts
    counts = 0; // Reset counts to zero
    interrupts(); // Enable interrupts again
    
    Serial.print("CPM: "); // Print counts per minute to serial monitor
    Serial.println(cpm);
    
    updateDisplay(); // Update OLED display with new data
    
    updateGraph(); // Update rolling graph with new data
    
    drawGraph(); // Draw rolling graph on OLED display
    
    display.display(); // Show display buffer on screen
    
    knobValue = analogRead(KNOB_PIN); // Read value from knob
    
    timeBase = map(knobValue, 0, 1023, 1, 60); // Map knob value to time base in seconds
    
    Serial.print("Time base: "); // Print time base to serial monitor
    Serial.println(timeBase);
    
    interval = timeBase * 1000 / graphW; // Calculate interval based on time base and graph width
    
    Serial.print("Interval: "); // Print interval to serial monitor
    Serial.println(interval);
    
    delay(10); 
    // Small delay to avoid bouncing
  }
}

// Update OLED display with new data
void updateDisplay() {
  // Clear the display buffer
  display.clearDisplay();
  
  // Set the cursor position and print CPM value
  display.setCursor(0, 0);
  display.print("CPM: ");
  display.print(cpm);
  
  // Set the cursor position and print time base value
  display.setCursor(64, 0);
  display.print("Time base: ");
  display.print(timeBase);
  display.print("s");
}

// Update rolling graph with new data
void updateGraph() {
  // Shift the graph data to the left by one pixel
  for (int i = 0; i < graphW - 1; i++) {
    graphData[i] = graphData[i + 1];
  }
  
  // Add the new data to the rightmost pixel
  graphData[graphW - 1] = cpm;
  
  // Find the maximum value in the graph data
  graphMax = 0;
  for (int i = 0; i < graphW; i++) {
    if (graphData[i] > graphMax) {
      graphMax = graphData[i];
    }
  }
}

// Draw rolling graph on OLED display
void drawGraph() {
  // Draw a horizontal line at the bottom of the graph
  display.drawLine(graphX, graphY + graphH - 1, graphX + graphW - 1, graphY + graphH - 1, SSD1306_WHITE);
  
  // Draw a vertical line at the left of the graph
  display.drawLine(graphX, graphY, graphX, graphY + graphH - 1, SSD1306_WHITE);
  
  // Draw the graph data as vertical bars
  for (int i = 0; i < graphW; i++) {
    // Map the data value to the graph height
    int barHeight = map(graphData[i], 0, graphMax, 0, graphH - 2);
    
    // Draw a vertical bar from the bottom to the data value
    display.drawLine(graphX + i, graphY + graphH - 2, graphX + i, graphY + graphH - 2 - barHeight, SSD1306_WHITE);
  }
}



//Source: Conversation with Bing, 4/30/2023
//(1) Arduino DIY Geiger Counter : 12 Steps (with Pictures) - Instructables. https://www.instructables.com/Arduino-DIY-Geiger-Counter/.
//(2) Arduino Library & Examples | Monochrome OLED Breakouts | Adafruit .... https://learn.adafruit.com/monochrome-oled-breakouts/arduino-library-and-examples.
//(3) Geiger Counter With Arduino Uno - Instructables. https://www.instructables.com/Geiger-Counter/.
//(4) Arduino Geiger Counter : 6 Steps - Instructables. https://www.instructables.com/Arduino-Geiger-Counter/.
//(5) Wiring 128x32 I2C Display | Monochrome OLED Breakouts | Adafruit .... https://learn.adafruit.com/monochrome-oled-breakouts/wiring-128x32-i2c-display.
//(6) Wiring 128x32 SPI OLED display | Monochrome OLED Breakouts | Adafruit .... https://learn.adafruit.com/monochrome-oled-breakouts/wiring-128x32-spi-oled-display.
