// Guide to Internet of Things
// Chapter 9
// This code combines SHT30 temperature sensing with BLE wireless communication.

// sht30-to-ble.ino monitors temperature data from a SHT30-D temperature sensor.
// It then sends this over wireless BLE.
// Be sure to include the adafruit-sht31 library in this project.
// Include the header file for this library.

#include <adafruit-sht31.h>

// Set debug to true for particle serial monitor
bool DEBUG = TRUE;

// Get access to an Adafruit_SHT31 object.
Adafruit_SHT31 sht31 = Adafruit_SHT31();

// Prototypes for functions
// Send to temperature to a BLE central device
void sendToBLE(float celsius);
// Get the temperature from a SHT30 sensor.
float getTempFromSHT30();

// We need to encode a float temperature with a standard encoding for BLE transmission.
uint32_t encoding_from_float_using_ieee11073(float temperature);


// The "Health Thermometer" GATT service is 0x1809.
// See https://www.bluetooth.com/specifications/assigned-numbers/
// and look in the document 16-bit-UUIDs.

// Create a UUID object for the service.
BleUuid healthThermometerService(BLE_SIG_UUID_HEALTH_THERMONETER_SVC);

// The Temperature Measurement GATT chacteristic is 0x2A1C.
// See https://www.bluetooth.com/specifications/assigned-numbers/
// and look in the document 16-bit-UUIDs.

// Create a BLE Characteristic object with a name and a reference to its service.
// By 'notify' we specify this as a value that may be subscribed to. The user can sit
// back and receive these values.
// The value 0x2A1C will be available to the central device.

BleCharacteristic temperatureMeasurementCharacteristic("temp", BleCharacteristicProperty::NOTIFY, BleUuid(0x2A1C), healthThermometerService);

// A byte array to hold 6 bytes for the WiFi mac address.
byte mac[6];

// used to avoid use of the delay()
unsigned long loop_timer = 0;

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

 // Turn the BLE radio on.
 BLE.on();

  // We want to publish this value.
  BLE.addCharacteristic(temperatureMeasurementCharacteristic);

  // We want to advertise this service.
  BleAdvertisingData advData;
  advData.appendServiceUUID(healthThermometerService);

  // Continuously advertise when not connected to a central device.
  BLE.advertise(&advData);


  // The mac address of the WiFi interface will be close to the
  // mac address of the BLE interface. We can use the mac address
  // of the WiFi interface to help locate the BLE mac address.

  WiFi.macAddress(mac);
  // initialize timer
  loop_timer = millis();
} // end of setup

void loop(void) {

  float celsius;

  if(millis() - loop_timer >= 5000UL) {
      loop_timer = millis();
      // Get temp from sensor
      celsius = (float) getTempFromSHT30();
      // and send over BLE to a central device
      sendToBLE(celsius);
  }
}

float getTempFromSHT30() {
  // The device reports temperatures in celsius.
  float celsius = sht31.readTemperature();
  if(DEBUG) Serial.println(celsius);

  return celsius;
}

void sendToBLE(float celsius) {

    // If a central has connected to this device then send an array of 6 bytes.
    if (BLE.connected()) {

		       // Prepare room for standard response data.
		       // The Temperature Measurement characteristic data formatis defined in detail in the SIG.
		       // The BLE standard defines the encoding of the 6 bytes

			   uint8_t buf[6];

			    // First byte is flags. We're using Celsius units (bit 0 == 0), no timestamp (bit 1 == 0), with temperature type (bit 2 == 1), so the flags are 0x04.
			    // Remaining flags (3,4,5,6,7) are for future use.
		   	  buf[0] = 0x04;

              // Last byte describes where in the body the temperature was taken.
              // For example, 1 == armpit, 4 == finger, and 6 == mouth.

			  buf[5] = 6; // Mouth

			  // We have 4 bytes to encode the actual temperature.
			  // buf[1],buf[2],buf[3], and buf[4]
			  // This is a 32 bit encoding.

			  // Get the temperature and encode in 32 bits.
			  // ieee 11073 defines how to do this encoding
			  uint32_t value = encoding_from_float_using_ieee11073(celsius);
			  // copy 4 bytes into buf[1], buf[2], buf[3], and buf[4]
			  memcpy(&buf[1], &value, 4);
              // Include this data in the characteristic. That will cause transmission over BLE.
              temperatureMeasurementCharacteristic.setValue(buf, sizeof(buf));
		}
		else {
		   // If no central has tried to connect, display a MAC address that is close to the BLE interface.
		   // This may be helpful for a user trying to connect.
		   if(DEBUG) {
		       Serial.println("Not connected to BLE");
		       Serial.printlnf("Scan for a mac address that is close to this WiFi mac: %02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		   }
		}

}
uint32_t encoding_from_float_using_ieee11073(float temperature) {

    // The standard way to represent a float as an integer
    // is described by ISO/IEEE 11073 Personal Health Data Standards.
    // Precondition: health temperature > 0.
     // Postcondition: Returns 32 bits of encoded data.
    // We will multiply the temperature by 100 and so we need to store an exponent of -2 in the encoding.
    // The receiver will then know to divide the temperature by 100.
    // Let's step through how to represent -2 in 8 bits.
    // First, represent it as positive.
    uint8_t posInt = 0b00000010;
    // Second, flip the bits using a bitwise not.
    uint8_t flippedBits = ~posInt;
    // Third, add one
     uint8_t exponent = flippedBits + 1;
    // Now, we have represented the -2 as an 8 bit integer in 2's compliment notation.
	// place the exponent on the left of a 32 bit unsigned integer
	  uint32_t result = ((uint32_t)exponent) << 24;
	  // multiply the temperature by 100 to shift the decimal point to the right two places.
	  float shiftedTemperature = temperature * 100;
	  // truncate the fractional portion and store the result in mantissa.
	  uint32_t mantissa = (uint32_t)(shiftedTemperature);
	  // combine the exponent with the mantissa using a bitwise or
  	  result = result | mantissa;
	  if(DEBUG) Serial.printlnf("TEMPERATURE %08X",result);
      return result;
}
