// Guide to Internet of Things
// Chapter 10
// Part 1 WaterSensor.ino

// WaterSensor firmware
// A0 is connected to the signal pin on the DGZZI Water Level Sensor
const int waterLevelPin = A0;
// We want a report on the level every 5 seconds
const int NUMSECONDS = 5;
int timeCtr = 0;
// Use an int to hold the water level
int waterValue = 0;
// Use the command line and the command:
// "particle serial monitor" to view these values.

// Set up serial speed.
void setup() {
    Serial.begin(9600);
}

void loop() {

     //Make a report every 5 seconds
     if (timeCtr <= millis()) {
         // get the value from A0
          waterValue = analogRead(waterLevelPin);
          // display on the command line
          Serial.println(waterValue);
          // update timeCtr to be 5 seconds greater than current time
          timeCtr = millis() + (NUMSECONDS * 1000);
     }
}
