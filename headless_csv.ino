I'll try to create that. Here is a possible code for Arduino that implements a Geiger counter, outputting average each minute to serial port, in CSV format with first field being timestamp in millis and second field CPM value. I used some information from the web search results¹²³ to write this code.

```c
// Define pins
#define GEIGER_PIN 2 // Geiger counter output pin
#define LED_PIN 13 // LED pin
#define BUZZER_PIN 3 // Buzzer pin

// Define constants
#define INTERVAL 60000 // Interval for calculating average CPM (in milliseconds)
#define CONVERSION 151 // Conversion factor from CPM to uSv/h for M4011 GM tube

// Define variables
volatile unsigned long count = 0; // Counter for Geiger pulses
unsigned long previousMillis = 0; // Previous time for interval calculation
unsigned long currentMillis; // Current time for interval calculation
float cpm; // Counts per minute
float dose; // Radiation dose in uSv/h

// Interrupt service routine for Geiger pulses
void pulse() {
  count++; // Increment counter
  digitalWrite(LED_PIN, HIGH); // Turn on LED
  tone(BUZZER_PIN, 1000, 100); // Play a tone on buzzer
}

void setup() {
  Serial.begin(9600); // Start serial communication
  pinMode(GEIGER_PIN, INPUT); // Set Geiger pin as input
  pinMode(LED_PIN, OUTPUT); // Set LED pin as output
  pinMode(BUZZER_PIN, OUTPUT); // Set buzzer pin as output
  attachInterrupt(digitalPinToInterrupt(GEIGER_PIN), pulse, RISING); // Attach interrupt to Geiger pin
}

void loop() {
  currentMillis = millis(); // Get current time
  if (currentMillis - previousMillis >= INTERVAL) { // Check if interval has passed
    cpm = count / (INTERVAL / 60000.0); // Calculate counts per minute
    dose = cpm / CONVERSION; // Calculate radiation dose in uSv/h
    Serial.print(currentMillis); // Print timestamp in millis
    Serial.print(","); // Print comma separator
    Serial.println(cpm); // Print counts per minute
    count = 0; // Reset counter
    previousMillis = currentMillis; // Update previous time
  }
  digitalWrite(LED_PIN, LOW); // Turn off LED
}
```

Source: Conversation with Bing, 5/10/2023
(1) Arduino DIY Geiger Counter : 12 Steps (with Pictures) - Instructables. https://www.instructables.com/Arduino-DIY-Geiger-Counter/.
(2) Geiger Counter With Arduino Uno - Instructables. https://www.instructables.com/Geiger-Counter/.
(3) Arduino Geiger Counter : 6 Steps - Instructables. https://www.instructables.com/Arduino-Geiger-Counter/.
