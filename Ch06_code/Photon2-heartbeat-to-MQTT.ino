
// Photon2-heartbeat-to-MQTT
// Firmware to run on an Photon 2 microcontroller
// This program runs on a microcontroller and
// sends a heartbeat message to an MQTT broker
// every 10 seconds.

// The local broker is run with the following command:
// mosquitto -c /usr/local/etc/mosquitto/mosquitto.conf -v
// The broker uses a modified configuration file.

// The MQTT library must be included in the project.
// Here, we only include the header file.
#include "MQTT.h"

// This is the function prototype for a callback function.
// A callback function will be used when we subscribe
// to a topic and expect to receive calls from the broker.
// In this program, we are only publishing to the broker
// and we do not expect to receive calls from the broker.
// Still, we provide the protoctype and the function body.
void callback(char* topic, byte* payload, unsigned int length);

// Here, we include the IP address of a running MQTT broker.
// We do not use localhost because localhost would refer to
// the microcontroller itself. Be sure to change this to
// your own IP address.
byte IPAddressOfBroker[] = { 192,168,86,29};

// Create an MQTT object by calling its contructor.
// MQTT brokers have an IP address.
// MQTT brokers listen on port 1883.
// And MQTT brokers may need to call back if the microcontroller
// is a subscriber.
MQTT broker(IPAddressOfBroker, 1883, callback);

// The callback is called by the broker if the microcontroller
// has subscribed to a topic and there are publications to that
// topic. In this publishing program, we do not expect this callback
// to be used.
void callback(char* topic, byte* payload, unsigned int length) {
   Serial.println("An unexpeted message received from broker");
}

// This is the MQTT topic name to publish under.
String topic = "heartbeat";

// This is the name of this Photon 2 client.
// The broker will require unique client names.
String clientName = "Photon2Client1";

// When DEBUG is true we will write output to the serial console.
boolean DEBUG = true;

// Establish the number of seconds to wait until we publish a message.
int NUMSECONDS = 10;

// The variable timeCtr will be used to hold the current time in milliseconds.
int timeCtr = 0;

// Device ID will be stored here after being retrieved from the device.
// This is a unique, 96 bit identifier.
// This looks like the following hex string: 0x3d002d000cf7353536383631.

String deviceID = "";

// The message array will hold a JSON string.
// This is the message we are sending to the topic.
char message[80];

// This class makes handling JSON data easy.
// Associate it with the message array of char.
JSONBufferWriter writer(message, sizeof(message));

// This method is run once on startup.
void setup() {
    // We plan on writing debug messages to the command line interface (CLI).
    // To see these debug messages, eneter particle serial monitor on the command line.
    Serial.begin(9600);

    // Here we get the unique id of this device as 24 hex characters.
    deviceID = System.deviceID().c_str();

    // Display the id to the command line interface.
    if (DEBUG) Serial.println(deviceID);

    // Establish the JSON string that we will send.

    writer.beginObject();
    writer.name("deviceID").value(deviceID);
    writer.endObject();

    // The JSON is now available through the message array.

    // Connect to the broker with a unique device name.
    // Start the broker before running this client.
    broker.connect(clientName);

}

// Start the broker before starting the Photon 2.
// Subscribe from the command line with:
// mosquitto_sub -t localhost -t heartbeat

void loop() {
     // If timeCtr is above the current time wait until the current time catches up.
     // Initially, timeCtr is 0. It is then set to the current time plus 10 seconds.
     // millis() returns an updated time.
     if (timeCtr <= millis()) {

             // If we are connected publish the message to the topic
             if (broker.isConnected()) {
                 if (DEBUG) Serial.println("Connection to MQTT established");

                 broker.publish(topic,message);


                 if(DEBUG) Serial.println("Message sent");
             }


            // set timeCtr to current time plus NUMSECONDS seconds
            timeCtr = millis() + (NUMSECONDS * 1000);
     }
}
