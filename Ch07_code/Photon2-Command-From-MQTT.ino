// Guide to Internet of Things
// Chapter 7
// Photon2-Command-From-MQTT
// This firmware runs on a Photon 2 microcontroller.
// It receives commands from MQTT.

// The local broker is run with the following command:
// mosquitto -c /usr/local/etc/mosquitto/mosquitto.conf -v
// The broker uses a modified configuration file.

// The MQTT library must be included in the project.
// Here, we only include the header file.
#include "MQTT.h"

// When DEBUG is true we will write output to the serial console.
boolean DEBUG = true;

// This is the function prototype for a callback function.
// We do not need a prototype unless we reference the function
// by name before actually defining the function.
// This code refe=rences the function when the MQTT broker object
// is constructed. So, we include this prototype.

void receiveCommand(char* topic, byte* payload, unsigned int length);

// Here, we include the IP address of a running MQTT broker.
// We do not use localhost because localhost would refer to
// the microcontroller itself. Be sure to change this to
// your own IP address - the address where the broker is running.
// The broker might be running anywhere on the internet. But
// we need to have its IP address.
byte IPAddressOfBroker[] = { 192,168,86,29};

// Here, we create an MQTT object by calling its contructor.
// MQTT brokers have an IP address.
// MQTT brokers listen on port 1883.
// The middleware need to know the name of the callback
// handler. In this case, the name of the callback function
// is "receiveCommand".
// The compiler is able to check that the function
// has the appropriate signature. The prototype has already
// been encountered.
MQTT broker(IPAddressOfBroker, 1883, receiveCommand);

// For our light, we will use the LED on D7.
const pin_t MY_LED = D7;


// A callback function will be used when we subscribe
// to a topic and expect to receive calls from the broker.
// In this program, we are receiving command messages
// from MQTT. This code will be called from within the MQTT library
// code. The MQTT library is acting as a middleware layer and
// is handling the communication details.


void receiveCommand(char* topic, byte* payload, unsigned int length) {

   if(DEBUG) Serial.println("A message was received from the broker. The message is from a publication to ");
   if(DEBUG) Serial.println(topic);

   // Copy the message into an array of char.
   char message[length + 1];
   memcpy(message, payload, length);
   message[length] = NULL;

   // Parse the JSON message
   JSONValue commandJSONValue = JSONValue::parseCopy(message);

   // Wrap the object with an iterator.
   JSONObjectIterator iter(commandJSONValue);

   // If there is a value of "on" or "off" turn the light "on" or "off"
   if(iter.next()) {

       if(DEBUG) Serial.print("Found the name: ");
       if(DEBUG) Serial.println(iter.name().data());
       if(DEBUG) Serial.print("Found the value: ");
       if(DEBUG) Serial.println(iter.value().toString().data());

       // If the command is "ON" set the light on.

       if(strcmp(iter.value().toString().data(),"on") == 0) {

	       digitalWrite(MY_LED, HIGH);

       }
       // if the command is "off" then turn the light off
       else if (strcmp(iter.value().toString().data(),"off") == 0) {

	       digitalWrite(MY_LED, LOW);
       }

   }
   else if(DEBUG) {
       Serial.println("iter.next() returned 0");
   }

}

// This is the MQTT topic name that we are subscribing to.
String topic = "microcontrollerCommand";

// This is the name of this client.
// The broker will require unique client names.
String clientName = "CommandReceiver1";


// This method is run once on startup.
void setup() {
    // We plan on writing debug messages to the command line interface (CLI).
    // To see these debug messages, eneter particle serial monitor on the command line.
    Serial.begin(9600);
    // Connect to the broker with a unique device name.
    // Start the broker before running this client.
    broker.connect(clientName);
    if (broker.isConnected()) {
        if (DEBUG) Serial.println("Connection to MQTT established");
        // Subscribe to a topic. Messages published to that topic
        // will result in the callback receiveCommand being called.
        broker.subscribe(topic,MQTT::QOS0);
    }
    else {
         if (DEBUG) Serial.println("Connection to MQTT not established in setup().");
    }
     // We will be using the D7 pin for output
	pinMode(MY_LED, OUTPUT);
	digitalWrite(MY_LED, LOW);

}

// To publish a command from the command line, we can use:
//  mosquitto_pub -h localhost -t microcontrollerCommand -i commander -m "{\"light\":\"on\"}"
// The broker is on localhost (-h).
// The topic is microcontrollerCommand (-t).
// The name of the client is commander (-i).
// We are sending a JSON message {"light" : "on" }
// Start mosquitto:
// mosquitto -c /usr/local/etc/mosquitto/mosquitto.conf -v
// Flash this firmware.


void loop() {
     // Call loop on the broker so that it can process network data.
     // If there is data to process, it will make a call on the callback
     // function.
     // If we do not call loop(), there is no chance that the callback will get called.

     if (broker.isConnected())  broker.loop();
}
