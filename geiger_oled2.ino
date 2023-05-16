
// Include the libraries for the OLED display and the Geiger counter
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <Wire.h>

// Define interval for serial out statistics
#define SERIAL_OUT_INTERVAL 60000 // 60 so each minute

// Define the pins for the OLED display
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Define text parameters
#define DRAW_TEXT // if draw text at all
#define DRAW_TEXT_TIMEBASE // if draw timebase
//#define DRAW_TEXT_SHADOW // if cast shadow 
#define DRAW_TEXT_RECTANGLE // if clear space for graph by drawing rectangle

#define TEXT_CPM_X 0 // x position of CPM text
#define TEXT_CPM_Y SCREEN_HEIGHT-8 // y position of CPM text

#define TEXT_TIMEBASE_X 60 // x position of timebase text
#define TEXT_TIMEBASE_Y SCREEN_HEIGHT-8 // y position of timebase text

// Define the pin for the Geiger counter
#define GEIGER_PIN 2 // Interrupt pin for the Geiger counter
#define BUZZER_PIN 3 // Buzzer pin

// Define the pin for the knob
#define KNOB_PIN A0 // Analog pin for the knob

// Create an object for the OLED display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Create some variables for the Geiger counter
volatile uint32_t counts = 0; // Counts per second
volatile float cpm    = 0; // Counts per minute
volatile float cpm_avg1; // cpm short term average 
volatile float cpm_avg2; // cpm long term average 

uint32_t previousMillis = 0; // Previous time in milliseconds
uint32_t previousMillis_serial = 0; // Previous time in milliseconds for serial out routine
uint32_t previousMillis_graph = 0; // Previous time in milliseconds for graph update

uint32_t interval = 1000;     // Interval to update counts per second
uint32_t  graph_interval = 1 ; // Interval to update graph , adjustable by a knob 
bool pulse_beep = false ; // pulse detected

// Create some variables for the knob
int knobValue = 0; // Value read from the knob
uint8_t timeBase = 1; // Time base for the rolling graph in seconds

// Create some variables for the rolling graph
uint8_t graphX = 0;             // X position of the graph
uint8_t graphY = 0;             // Y position of the graph
uint8_t graphW = SCREEN_WIDTH;  // Width of the graph
uint8_t graphH = SCREEN_HEIGHT-1; // Height of the graph
uint16_t graphMin = 0 ;          // minimum value of the graph
uint16_t graphMax = 100;         // Maximum value of the graph
#define OPTIMIZED_MAX_SEARCH    // faster for bigger displays
 //but does not include most recent value in the search 

// Create an array to store the graph data
uint16_t graphData[SCREEN_WIDTH];
//int graphData[graphW]; // todo : there are things hardcoded below, beware

// Interrupt service routine for the Geiger counter
void geigerISR() {
  counts++; // Increment counts by one
  pulse_beep = true; 
}

void setup() {
  Serial.begin(115200); // Start serial communication

  pinMode(BUZZER_PIN, OUTPUT); // Set buzzer pin as output
  
  // Initialize the OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
//    while (true); // Don't proceed, loop forever
 // if display not found, proceed anyway
  }
  
  // Clear the display buffer and set text size and color
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Attach an interrupt to the Geiger counter pin
  pinMode(GEIGER_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(GEIGER_PIN), geigerISR, RISING);
}

void loop() {
  uint32_t currentMillis = millis(); // Get current time in milliseconds
  
  // Update counts per second every interval
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis; // Save current time
    
    cpm = (cpm * 59.0 + counts * 60.0) / 60.0; // Calculate counts per minute using exponential moving average
    cpm_avg1 = (cpm_avg1 *  3.0 + cpm) /  4.0;  // short term average for external graph
    cpm_avg2 = (cpm_avg2 * 11.0 + cpm_avg1) / 12.0;  // long  term average for external graph

    noInterrupts();   // Disable interrupts while updating counts
    counts = 0;       // Reset counts to zero
    interrupts();     // Enable interrupts again
    
//    Serial.print("CPM: "); // Print counts per minute to serial monitor
//    Serial.println(cpm);
    if (currentMillis - previousMillis_serial >= SERIAL_OUT_INTERVAL) { // Check if interval has passed
      previousMillis_serial = currentMillis; // Update previous time for serial out
      Serial.print(currentMillis); // Print timestamp in millis
      Serial.print(","); // Print comma separator
      Serial.print(cpm_avg1,3); // Print short term average
      Serial.print(","); // Print comma separator
      Serial.println(cpm_avg2,3); // Print long term average
      }
    
    knobValue = analogRead(KNOB_PIN); // Read value from knob
    timeBase = map(knobValue, 0, 920, 1, 120); // Map knob value to time base in seconds
    graph_interval = timeBase * 1000 ; // Calculate graph interval based on time base 
    
  if (currentMillis - previousMillis_graph >= graph_interval) {
    previousMillis_graph = currentMillis; // Save current time
                       // Clear the display buffer
    display.clearDisplay();
    updateGraph();     // Update rolling graph with new data    
    drawGraph();       // Draw rolling graph on OLED display
  }
      
#ifdef DRAW_TEXT    
    updateDisplay();   // Update OLED display with text  
#endif //DRAW_TEXT
    display.display(); // Show display buffer on screen
    
 
  } //if 1 second interval

    if (pulse_beep){
    tone(BUZZER_PIN, 4000, 10); // beep buzzer
    pulse_beep = false ; // reset pulse_beep 
    }
}

#ifdef DRAW_TEXT
// Update OLED display with new data
void updateDisplay() {


#ifdef DRAW_TEXT_SHADOW
//cast +1 -1 shadow first
  display.setTextColor(SSD1306_BLACK);
  // Set the cursor position and print CPM value
  display.setCursor(TEXT_CPM_X+1, TEXT_CPM_Y-1);
  //display.print("CPM: ");
  display.print(cpm);
  
  display.setCursor(TEXT_TIMEBASE_X+1, TEXT_TIMEBASE_Y-1);
  display.print("T: ");
  display.print(timeBase);
  display.print("s");
  
//cast +1 +1 shadow 
  display.setTextColor(SSD1306_BLACK);
  // Set the cursor position and print CPM value
  display.setCursor(TEXT_CPM_X+1, TEXT_CPM_Y+1);
  //display.print("CPM: ");
  display.print(cpm);
  
  // Set the cursor position and print time base value
  display.setCursor(TEXT_TIMEBASE_X+1, TEXT_TIMEBASE_Y+1);
  display.print("T: ");
  display.print(timeBase);
  display.print("s");

//cast -1 +1 shadow first
  display.setTextColor(SSD1306_BLACK);
  // Set the cursor position and print CPM value
  display.setCursor(TEXT_CPM_X-1, TEXT_CPM_Y+1);
  //display.print("CPM: ");
  display.print(cpm);
  
  // Set the cursor position and print time base value
  display.setCursor(TEXT_TIMEBASE_X-1, TEXT_TIMEBASE_Y+1);
  display.print("T: ");
  display.print(timeBase);
  display.print("s");
#endif //DRAW_TEXT_SHADOW

#ifdef DRAW_TEXT_RECTANGLE
int16_t  x1, y1;
uint16_t w, h;

char buffer[40];
  sprintf(buffer, "%d.%02d",(uint16_t)cpm, (uint16_t)(cpm*100)%100);
display.getTextBounds(buffer, TEXT_CPM_X, TEXT_CPM_Y, &x1, &y1, &w, &h);
display.fillRect(x1,y1-1,w,h+1,BLACK); // Clear the area below CPM value
#endif // DRAW_TEXT_RECTANGLE

//paint text
  display.setTextColor(SSD1306_WHITE);
  // Set the cursor position and print CPM value
  display.setCursor(TEXT_CPM_X, TEXT_CPM_Y);
  //display.print("CPM: ");
  display.print(buffer);

#ifdef DRAW_TEXT_TIMEBASE  
#ifdef DRAW_TEXT_RECTANGLE
display.fillRect(TEXT_TIMEBASE_X,TEXT_TIMEBASE_Y-1,40,9,BLACK); // Clear the area below timebase value
#endif // DRAW_TEXT_RECTANGLE

  // Set the cursor position and print time base value
  display.setCursor(TEXT_TIMEBASE_X, TEXT_TIMEBASE_Y);
  display.print("T: ");
  display.print(timeBase);
  display.print("s");
#endif //DRAW_TEXT_TIMEBASE
}
#endif DRAW_TEXT

// Update rolling graph with new data
void updateGraph() {
#ifdef OPTIMIZED_MAX_SEARCH
// Shift the graph data to the left by one pixel and find the maximum value
  graphMin = graphMax; // set graphMin to last graphMax value
  graphMax = 0;
  for (int i = 0; i < graphW - 1; i++) {
    graphData[i] = graphData[i + 1];
    if (graphData[i] > graphMax) {
      graphMax = graphData[i];
      }
    if (graphData[i] < graphMin) {
      graphMin = graphData[i];
    }
    // graphMax = max(graphMax, graphData[i]); 
    // or use that instead 
  }
#endif OPTIMIZED_MAX_SEARCH

#ifndef OPTIMIZED_MAX_SEARCH
  // Shift the graph data to the left by one pixel
  for (int i = 0; i < graphW - 1; i++) {
    graphData[i] = graphData[i + 1];
  }
#endif OPTIMIZED_MAX_SEARCH
  
  // Add the new data to the rightmost pixel
  graphData[graphW - 1] = cpm;

#ifndef OPTIMIZED_MAX_SEARCH
  // Find the maximum value in the graph data
  graphMax = 0;
  graphMin = 0; 
  for (int i = 0; i < graphW; i++) {
    if (graphData[i] > graphMax) {
      graphMax = graphData[i];
    }
  }
#endif OPTIMIZED_MAX_SEARCH


}

// Draw rolling graph on OLED display
void drawGraph() {
  // Draw a horizontal line at the bottom of the graph
  //display.drawLine(graphX, graphY + graphH, graphX + graphW , graphY + graphH , SSD1306_WHITE);
  
  // Draw a vertical line at the left of the graph
  //display.drawLine(graphX, graphY, graphX, graphY + graphH - 1, SSD1306_WHITE);
  
  // Draw the graph data as vertical bars
  for (uint8_t i = 0; i < graphW; i++) {
    // Map the data value to the graph height
    uint8_t barHeight = map(graphData[i], graphMin, graphMax, 0, graphH );
    
    // Draw a vertical bar from the bottom to the data value
//    display.drawLine(graphX + i, graphY + graphH , graphX + i, graphY + graphH - barHeight, SSD1306_WHITE);
    //display.drawFastVLine(graphX + i, graphY + graphH , barHeight, SSD1306_WHITE);
    display.drawFastVLine(graphX + i, graphY+(graphH-barHeight) , barHeight, SSD1306_WHITE);
    
  }
}



//Source: Conversation with Bing, 4/30/2023
//(1) Arduino DIY Geiger Counter : 12 Steps (with Pictures) - Instructables. https://www.instructables.com/Arduino-DIY-Geiger-Counter/.
//(2) Arduino Library & Examples | Monochrome OLED Breakouts | Adafruit .... https://learn.adafruit.com/monochrome-oled-breakouts/arduino-library-and-examples.
//(3) Geiger Counter With Arduino Uno - Instructables. https://www.instructables.com/Geiger-Counter/.
//(4) Arduino Geiger Counter : 6 Steps - Instructables. https://www.instructables.com/Arduino-Geiger-Counter/.
//(5) Wiring 128x32 I2C Display | Monochrome OLED Breakouts | Adafruit .... https://learn.adafruit.com/monochrome-oled-breakouts/wiring-128x32-i2c-display.
//(6) Wiring 128x32 SPI OLED display | Monochrome OLED Breakouts | Adafruit .... https://learn.adafruit.com/monochrome-oled-breakouts/wiring-128x32-spi-oled-display.
