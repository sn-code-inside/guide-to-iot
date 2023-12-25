// Guide to Internet of Things Chapter 9 Constrained Networking
// This firmware was adapted from the BLE tutorial
// found here: https://docs.particle.io/tutorials/device-os/bluetooth-le/

// The BLE class documentation is here:
// https://docs.particle.io/cards/firmware/bluetooth-le-ble/ble-class/

// There is a license here:
// https://github.com/particle-iot/docs/blob/master/LICENSE.txt
// Set debugging on
boolean DEBUG = true;

// After a connection is established, publish the encoded temperature every 5 seconds.
unsigned long  interval = 5000;

// Track milliseconds since last update.
unsigned long lastUpdate = 0;

// In C++, the function declaration (prototype) must be seen before a call.
float getTempC();

// We need to encode a float temperature with a standard encoding.
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

// We don't actually have a thermometer here, we just randomly adjust this value.
float lastValue = 37.0; // 98.6 deg F;

// A byte array to hold 6 bytes for the WiFi mac address.
byte mac[6];

void setup() {

     // If we want to print to the serial port we call 'begin' with a baud rate.
     Serial.begin(9600);

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
}

void loop() {


    // Only on occasion, let the if be true.
	  if (millis() - lastUpdate >= interval) {
		    lastUpdate = millis();


        // If a central has connected to this device then send an array of 6 bytes.
		    if (BLE.connected()) {

		       // Prepare room for standard response data.
		       // The Temperature Measurement characteristic data formatis defined in detail in this XML document.
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
			  uint32_t value = encoding_from_float_using_ieee11073(getTempC());
			  // copy 4 bytes into buf[1], buf[2], buf[3], and buf[4]
			  memcpy(&buf[1], &value, 4);
        // Include this data in the characteristic.
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
}

float getTempC() {
	// This function returns a random temperature.
	// Half the time, increase the value by 0.1.
	if (rand() > (RAND_MAX / 2)) {
		lastValue += 0.1;
	}
	else {
	// Half the time, decrease the value by 0.1.
		lastValue -= 0.1;
	}

    if(DEBUG) Serial.println(lastValue);

	return lastValue;
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
