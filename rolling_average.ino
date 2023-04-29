
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

// Define the pins for the Geiger counter
#define GEIGER_PIN 2 // Connect the Pulse from the Geiger counter to this pin

// Define the pin for the knob selecting the graph time base
#define KNOB_PIN A1 // Connect the knob to this analog pin

// Define some constants for the OLED display
#define TEXT_SIZE 1 // Text size for normal text
#define TEXT_COLOR SSD1306_WHITE // Text color for normal text
#define LINE_COLOR SSD1306_WHITE // Line color for graph and separator
#define LINE_YES 1 // wheter to draw line
#define LINE_Y 9 // Y coordinate of horizontal line
#define CPM_YES 1 // wheter to print CPM label
#define CPM_X 0 // X coordinate of CPM label
#define CPM_Y 0 // Y coordinate of CPM label
#define USVH_YES 1 // wheter to print uSv/h label
#define USVH_X 48 // X coordinate of uSv/h label
#define USVH_Y 0 // Y coordinate of uSv/h label
#define TIMEBASE_YES 1 // wheter to print timebase label
#define TIMEBASE_X 96 // X coordinate of time base label
#define TIMEBASE_Y 0 // Y coordinate of time base label
#define GRAPH_X_MIN 0 // X coordinate of the graph minimum
#define GRAPH_X_MAX 127 // X coordinate of the graph maximum
#define GRAPH_Y_MIN 0 // Y coordinate of the graph minimum
#define GRAPH_Y_MAX 31 // Y coordinate of the graph maximum

// Create an object for the OLED display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Create some variables for the Geiger counter
volatile unsigned long counts = 0; // Variable for GM Tube events
unsigned long previousMillis = 0; // Variable for time measurement
unsigned long interval = 1000; // Interval to calculate CPM (milliseconds)
unsigned int cpm = 0; // Variable for counts per minute, 65535 max. 
                      // 100 is normal 
                      // 300 is max background radiation 
                      // 800 is max natural radiation (f.e. granite) and gives cancer in about 7 years
                      // anything above is exceeding typical body renewal rate of cells (7 years)

// Create some variables for the rolling graph
const int numReadings = GRAPH_X_MAX-GRAPH_X_MIN ; // Number of readings to store in the buffer
            // set to size of the graph 
unsigned int readings[numReadings]; // The buffer to store the readings
int readIndex = 0; // The index of the current reading
long total = 0; // The running total of the readings (changed to long to avoid overflow)
int average = 0; // The average of the readings
int longTermAverage = 0; // The long term average of the readings

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
    //while (true); // Don't proceed, loop forever
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
    
    longTermAverage = (longTermAverage + average) / 2; // Calculate long term average
    
    knobValue = analogRead(KNOB_PIN); // Read knob value
    
    timeBase = map(knobValue, knobMin, knobMax, 1, 10); // Map knob value to time base
    
    interval = timeBase * 1000; // Adjust interval according to time base
    
    display.clearDisplay(); // Clear display buffer
    
    display.setTextSize(TEXT_SIZE); // Set text size to normal
    
    display.setTextColor(TEXT_COLOR); // Set text color to white
    
    display.setCursor(CPM_X, CPM_Y); // Set cursor position for CPM label
    
    display.print("CPM: "); // Print CPM label
    
    display.print(cpm);
    
    display.setCursor(USVH_X, USVH_Y); // Set cursor position for uSv/h label
    
    display.print(" uSv/h: "); // Print uSv/h label
    
    display.print(cpm / 151.0, 2); // Print uSv/h value (assuming M4011 tube)
    
    display.setCursor(TIME_X, TIME_Y); // Set cursor position for time base label
    
    display.print(" Time: "); // Print time base label
    
    display.print(timeBase); // Print time base value
    
    display.print("s"); // Print time unit
    
    display.drawLine(0, LINE_Y, 127, LINE_Y, LINE_COLOR); // Draw a horizontal line
    
    for (int i = 0; i < numReadings; i++) { // For each reading in the buffer
      
      int x = map(i, 0, numReadings - 1, 0, 127); // Map index to x coordinate
      
      int y = map(readings[i], 0, average * 2, 31, 17); // Map reading to y coordinate
      
      display.drawPixel(x, y, LINE_COLOR); // Draw a pixel at (x,y)
      
      int y2 = map(longTermAverage, 0, average * 2, 31, 17); // Map long term average to y coordinate
      
      if (i % 4 == 0) { // Draw a sparse dotted line for long term average
        
        display.drawPixel(x, y2, LINE_COLOR); // Draw a pixel at (x,y2)
        
      }
      
    }
    
    display.display(); // Show display buffer on the screen
    
  }
  
}

// Interrupt function for Geiger counter
void tube_impulse() {
  
  counts++; // Increment counts
  
}


//Source: Conversation with Bing, 4/29/2023
//(1) Program for average of an array without running into overflow. https://www.geeksforgeeks.org/program-for-average-of-an-array-without-running-into-overflow/.
//(2) Help with getting Sensor Average using Arrays - arduino uno. https://arduino.stackexchange.com/questions/25010/help-with-getting-sensor-average-using-arrays.
//(3) Array overflow when storing more than 32 values - Arduino Forum. https://forum.arduino.cc/t/array-overflow-when-storing-more-than-32-values/978773.
