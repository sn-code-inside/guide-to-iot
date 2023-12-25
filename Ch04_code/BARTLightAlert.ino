// Guide to Internet of Things Chapter 4 
// BARTLightAlert monitors data from the Bay Area Rapid Transit.
// The Photon 2 blinks the light at D7 at different speeds - depending upon how close the
// nearest train is to the station. For example, blinking slow
// signifies the train is far away. Blinking fast signifies it is near the station.
// If there is no train scheduled to arrive or if the web service is down, the light turns
// on but does not blink.
// You might imagine the microcontroller being embedded in a suitcase. When you fly
// into San Francisco International, your suitcase can provide information on how
// soon your train will arrive. Or, the microcontroller might be atached to your bike
// and have celluar access to the web service.
// For a full view of BART, visit
// https://www.bart.gov/sites/default/files/docs/system-map-detailed-version.pdf
// Prerequisite: This firmware makes HTTP calls to a web service named
// VisitBARTWebService.js. The IP address of the web service must be hard coded into
// this firmware. It is VisitBARTWebService.js that actually makes https calls to
// BART. This firmware is using http to call the web service.
// The HTTPClient library must be included in this project.
#include <HttpClient.h>

boolean DEBUG = true;

// Define the http variable to be of type HttpClient.
HttpClient http;

// We always pass Http headers on each request to the Http server
// Here, we only define a single header. The NULL, NULL pair is used
// to terminate the list of headers.

// The Accept header is used to inform the server
// of the type of message that the client understands. Here,
// we tell the server that we expect to receive data marked up in
// JSON.

http_header_t headers[] = {
   { "Accept", "application/json" },
   { NULL, NULL }
};

// Here we define structures to hold the request and the response data.
// These are declared with types defined in the header file included above.

http_request_t request;
http_response_t response;


// Visit the BART API and, depending upon how close the train is, blink the LED on D7.
const pin_t MY_LED = D7;

// Allow code to run before being connected to Particle.
// The light will blink before breathing cyan.
// This line is optional.
SYSTEM_THREAD(ENABLED);

// Enumerate different blinking states.
// These five identifiers are simply mapped to constants.
enum State {noBlink,verySlow,slow,medium,fast};

// The starting state is no blinking.
State state = noBlink;

// Function prototypes are declarations for the compiler.
// This is not a call to these functions.
State getStateUpdate(void);
String getMinutes(void);
String extractResponseFromJSON(String body);

// The setup() method is called once when the device boots.
void setup() {

    // We will write to the console for debugging.
    // We want to communicate over the USB connection to the computer.
    Serial.begin();

    // In the request object, set the IP address and port of the server that
   // we will visit. This could be any IP address and port.
   request.ip = IPAddress(192,168,86,216);
   request.port = 3000;

    // We will be using the D7 pin for output
	pinMode(MY_LED, OUTPUT);
}
// A light timer is needed to control blinking speeds
unsigned long blinking_light_timer = 0;

// A state timer is needed to control calls to BART to change state
unsigned long state_timer = 0;

// blink the light
void blink(long milli) {

        if(milli == 0) {
              // no blnk condition
              // Turn on the LED
     	      digitalWrite(MY_LED, HIGH);
        }
     	 else {
             // Perform a blink
             // Turn on the LED
     	     digitalWrite(MY_LED, HIGH);

	         // Leave it on for milli seconds
	         delay(milli);

	         // Turn it off
	         digitalWrite(MY_LED, LOW);
     	 }

}

// initialize blink delay below even verySlow
long lightDelayAmt = 15000UL;

// initialize time between calls to BART
// we will only visit the web every 15 seconds
long stateDelayAmt = 15000;

// The loop() method is called frequently.
// We want to exit the loop() quickly if possible
void loop() {

    // every 15 seconds call the API and check the state of trains

    if(millis() - state_timer >= stateDelayAmt) {

        // make a call to BART
        if(DEBUG) Serial.println("Making a call to BART proxy web service");
        if(DEBUG) Serial.println("to change the state.");
        state = getStateUpdate();

        // set the light speed based on the state
        switch(state) {
           case noBlink : lightDelayAmt = 0UL;
           break;
           case verySlow : lightDelayAmt = 3000UL;
           break;
           case slow: lightDelayAmt = 1000UL;
           break;
           case medium : lightDelayAmt = 500UL;
           break;
           case fast : lightDelayAmt = 100UL;
           break;
           default : lightDelayAmt = 15000UL;
        }

        // don't call again for 15 seconds
        state_timer = millis();
    }

    // after delayAmt, handle blinking
    if(millis() - blinking_light_timer >= lightDelayAmt) {

        blink(lightDelayAmt);

        blinking_light_timer = millis();
    }
}

State getStateUpdate(void) {
            // get the number of minutes remaining
            // String minutes = getMinutes();
            String minutes = getMinutes();

            // Check the minutes field to decide which light to display.
            // -1 means there is no train coming.
            // This may be due to a report from BART or from a failure to contact the web
            // service. We simply know of no train arriving for this destination.
            if(minutes.compareTo("-1") == 0) {
                // set state to no blink
                if(DEBUG)Serial.print("No current train to destination. Halt the blinking and stay bright.");
                return noBlink;
            }
            else
               if(minutes.compareTo("Leaving") == 0) {
                   // blink fast if the train is leaving
                   if(DEBUG)Serial.println("You had better be on the train. It is leaving. Blink fast");
                   return fast;
               }
               else { // blink speed based on how close the train is

                 if(DEBUG)Serial.printlnf("The number of minutes until arrival is %s", minutes.c_str());

                 int numMinutes = minutes.toInt();

                 if(numMinutes >= 15) {
                           if(DEBUG)Serial.println("very slow if the train is at least 15 minutes away");
                           return verySlow;
                 }
                 else {
                           if(numMinutes >= 10) {
                               if(DEBUG)Serial.println("slow if the train is at least 10 minutes but less than 15 minutes away");
                               return slow;
                 }
                 else {
                           if(numMinutes >= 5) {
                                   if(DEBUG)Serial.println("medium speed blinking if the train is at least 5 minutes but less than 10 minutes away");
                                   return medium;
                 }
                else {
                            if(DEBUG)Serial.println("fast for the train is quite close");
                            return fast;
                }
            }

        }
        return noBlink; // The logic should never arrive here. This is for the compiler.
    }
}
// Make a call on the web service tha, in turn, makes a call on BART.
String getMinutes() {
    String str;
    // Make call to BART proxy to get the number of minutes
    // as a string
    str = doGetRequest();
    // return the string with the number of minutes or -1 or Leaving
    return str;
}

// Provided with a response, display it to the command line interface.
 void printResponse(http_response_t &response) {
   Serial.println("HTTP Response: ");
   Serial.println(response.status);
   Serial.print("Response body:");
   Serial.println(response.body);
 }
// Get minutes from the server
String doGetRequest() {
  // set the path and clear any old response data
  request.path = "/minutesUntilArrival";
  response.status = 0;
  response.body = "";
  // perform an HTTP Get
  http.get(request, response, headers);
  // display if DEBUG is true
  if(DEBUG) printResponse(response);
  // Return the value from the Arrival key
  return (extractResponseFromJSON(response.body));
}

String extractResponseFromJSON(String body) {
     // The default is -1. This means no trains arriving.
     String message = "-1";
     // Parse the JSON
     JSONValue outerObj = JSONValue::parseCopy(body);
     // Drill in and pick up the value
    JSONObjectIterator iter(outerObj);
    if(iter.next()) {
        String name = (String)iter.name();
        if(DEBUG) Serial.println(name);
        if(strcmp(name,"Arrival") == 0) {
            // message might become -1, number of minutes, or Leaving
            message = (String)iter.value().toString();
        }
    }
    // Return -1 or Leaving or number of minutes
    return message;
}
