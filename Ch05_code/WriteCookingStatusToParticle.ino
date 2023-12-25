// Guide to Internet of Things
// Chapter 5
// WriteCookingStatusToParticle.ino reads data from a SHT30-D temperature sensor
// and uses decision logic to write cooking status information to the Particle
// Console.

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

// Cooking values
float cookingTemperature = 78.0;
int numMinutes = 1;

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
         // Write ststus evey 6 seconds or so
         writeCookingStatus(fahrenheit, cookingTemperature, numMinutes);

         Serial.print("Temperature in fahrenheit = ");
         Serial.println(fahrenheit);
       } else { // not a good temp
         Serial.println("The temperature is not a number.");
       }
       // reset the timer
       timer = millis();
   }
}
// If we rise above the cooking temperature we are in an interval.
bool inInterval = false;
// We count how many times we are in that interval.
int intervalCounter = 0;

// Send the current temperauture and the cooking status to the Particle Console.
void writeCookingStatus(float currentTemperature, float cookingTemperature, int minutes) {

  // 1 cookingInterval == 6 seconds
  // 1 cookingInterval/(6 seconds) == 1
  // For one minute of cooking, we need 10 cookingIntervals.
  // cookingIntervals == minutes * 60 / 6
  int cookingIntervals = minutes * 60 / 6;
  // store current temperature  in "temperature" string
  String temperature = String(currentTemperature);
  // Publish a name-value pair to Particle
  Particle.publish("temperature", temperature, PRIVATE);

  if(currentTemperature > cookingTemperature) {
    // We are cooking at the proper temperature
    inInterval = true;
    // Count the number of times that we are in a cooking interval.
    intervalCounter = (intervalCounter + 1);

    // In the interval for long enough?
    if(intervalCounter >= cookingIntervals) {
      // publish to cloud that the cooking is done
      Particle.publish("your_food_is_done");
    }
    else {
      // publish to cloud that cooking is in progress
      Particle.publish("cooking_in_progress");
    }
  }
  else {
    // Publish to cloud that the current temperature is not sufficient.
    // We are not in a cooking interval.
    inInterval = false;
    intervalCounter = 0;
    Particle.publish("temperature_below_threshold");
  }
}
