// Guide to Internet of Things
// Chapter 5
// ReadTemperature_SHT30-D.ino monitors temperature data from a SHT30-D temperature sensor.
// Be sure to include the adafruit-sht31 library in this project.
// Include the header file for this library.

#include <adafruit-sht31.h>

// Get access to an Adafruit_SHT31 object.
Adafruit_SHT31 sht31 = Adafruit_SHT31();

// Initialize timer
unsigned long timer = 0;
// Delay amount is set to six seconds
unsigned long delayAmount = 6000;

void setup() {
  // The variable connected will be true or false depending on the connection to SHT31-D.
  bool connected;
  // We may use the USB connection as well.
  Serial.begin();

  // Establish a connection to temperature sensor.
  // The SHT30-D's address is 0x44. Other I2C sensors will
  // have other addresses. These addreeses are specified in the datasheet
  // for the device.
  connected = sht31.begin(0x44);
  // Inform the user if the connection was established.
  if (connected) {
    Serial.println("Connected to SHT31");
  }
  else {
    Serial.println("Error: Not connected to SHT31");
  }
  // initialize timer
  timer = millis();
} // end of setup

void loop() {

  // The variable goodTemp will be true when the temperature is a number.
  bool goodTemp;
  // Get the temperature every 6 seconds or so.
  if(millis() - timer >= delayAmount) {

      // The device reports temperatures in celsius.
      float celsius = sht31.readTemperature();

      // Is the value a number? If so then not a number (isnan) is false and !isnan is true.
      goodTemp = !isnan(celsius);

      if (goodTemp) {

         Serial.print("Temperature in celsius = ");
         Serial.println(celsius);

         // Convert to fahrenheit.
         float fahrenheit = (celsius* 9.0) /5.0 + 32.0;

         Serial.print("Temperature in fahrenheit = ");
         Serial.println(fahrenheit);
       } else { // not a good temp
         Serial.println("The temperature is not a number.");
       }
       // reset the timer
       timer = millis();
   }
}
