// BARTLightAlert monitors data from the Bay Area Rapid Transit.

// In this code, we have also hardcoded the destination "Antioch".
// So, we are interested in trains arriving at SFIA and headed to
// Antioch.

// The response is in a standard JSON format but, here, no
// JSON parser is used. Instead, a simple string search is performed.

// The Argon blinks the light at D7 at different speeds - depending upon how close the
// nearest train is to the station. For example, blinking slow
// signifies the train is far away. Blinking fast signifies it is near the station.
// You might imagine the microcontroller being embedded in a suitcase. When you fly
// into San Francisco International, your suitcase can provide information on how
// soon your train will arrive.

// For web view visit https://www.bart.gov/schedules/eta?stn=SFIA
// The source is set at the San Francisco International Airport

// For a full view of BART, visit
// https://www.bart.gov/sites/default/files/docs/system-map-detailed-version.pdf


boolean DEBUG = true;

// Use the TCPClient to write HTTP request to BART
TCPClient client;

// a char array to hold reseponse data from an HTTP request
char reply[2048];


// Visit the BART API and, depending upon how close the train is, blink the LED on D7.
const pin_t MY_LED = D7;

// Allow code to run before being connected to Particle.
// The light will blink before breathing cyan.
// This line is optional.
SYSTEM_THREAD(ENABLED);


// enumerate different blinking states
enum State {verySlow,slow,medium,fast};

// starting state is verySlow blinking
State state = verySlow;

// Function prototype declaration for the compiler.
// This is not a call on this function.
State getStateUpdate(void);

// The setup() method is called once when the device boots.
void setup() {

    // We will write to the console for debugging.
    // We want to communicate over the USB connection to the computer.
    Serial.begin();
    // We will be using the D7 pin for output
	pinMode(MY_LED, OUTPUT);
}
// A light timer is needed to control blinking speeds
unsigned long blinking_light_timer = 0;

// A state timer is needed to control calls to BART to change state
unsigned long state_timer = 0;

// blink the light
void blink(long milli) {

        // Turn on the LED
     	digitalWrite(MY_LED, HIGH);

	    // Leave it on for milli seconds
	    delay(milli);

	    // Turn it off
	    digitalWrite(MY_LED, LOW);

}

// initialize blink delay below even verySlow
long lightDelayAmt = 5000UL;

// initialize time between calls to BART
// we will only visit the web every 15 seconds
long stateDelayAmt = 15000;

// The loop() method is called frequently.
// We want to exit the loop() quickly if possible
void loop() {

    // every 15 seconds call the API and check the state of trains

    if(millis() - state_timer >= stateDelayAmt) {

        // make a call to BART
        if(DEBUG) Serial.println("Make a call to BART");
        if(DEBUG) Serial.println("Change the state");
        state = getStateUpdate();

        // set the light speed based on the state
        switch(state) {
           case verySlow : lightDelayAmt = 3000UL;
           break;
           case slow: lightDelayAmt = 1000UL;
           break;
           case medium : lightDelayAmt = 500UL;
           break;
           case fast : lightDelayAmt = 100UL;
           break;
           default : lightDelayAmt = 5000UL;
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
           // send an http get request to BART
           if(DEBUG) Serial.println("Connecting to web server: ");
           if (client.connect("api.bart.gov", 80)) {
               if(DEBUG) Serial.println("Connected...");
               out_to_server("GET /api/etd.aspx?cmd=etd&orig=sfia&key=MW9S-E7SL-26DU-VV8V&json=y\r\n");
               //out("Host: mylocalserver:8080\r\n");
               out_to_server("User-Agent: Argon/1.0\r\n");
               out_to_server("Accept: application/json\r\n");
               out_to_server("Content-Type: application/x-www-form-urlencoded\r\n");
               out_to_server("Connection: close\r\n");

               if(DEBUG) Serial.println("Request sent to server");
               if(DEBUG) Serial.println("Closing Connection...");
               // Particle.publish("ServerAvailable");
               in_from_server(reply, 3000);
            }
            else {  // server not found to be available
               if(DEBUG) Serial.println("Connection failed");
               if(DEBUG) Serial.println("Disconnecting.");
               client.stop();

            }

            // handle reply by searching string for destination

            String destination = "Antioch";
            String minutes = getMinutes(reply, destination);


            // Check the minutes field to decide which light to display.
            if(minutes.compareTo("No such train") == 0) {
                // blink slow if there is no train
                if(DEBUG)Serial.print("No current train to ");
                if(DEBUG)Serial.println(destination);
                return verySlow;
            }
            else

               if(minutes.compareTo("Leaving") == 0) {
                   // blink fast if the train is leaving
                   if(DEBUG)Serial.println("You had better be on the train. It is leaving. Blink fast");
                   return fast;
               }
               else { // blink speed based on how close the train is

                 if(DEBUG)Serial.print("The number of minutes until arrival is ");

                 int numMinutes = minutes.toInt();

                 if(DEBUG)Serial.println(numMinutes);

                 if (numMinutes != 0) {
                     if(numMinutes >= 15) {
                           if(DEBUG)Serial.println("very slow if more that 15 minutes away");
                           return verySlow;
                     }
                     else {
                           if(numMinutes >= 10) {
                               if(DEBUG)Serial.println("slow if more than 10 minutes and less than 15");
                               return slow;
                           }
                           else {
                               if(numMinutes >= 5) {
                                   if(DEBUG)Serial.println("medium speed blinking if 5 minutes or above away");
                                   return medium;
                               }
                               else {
                                   if(DEBUG)Serial.println("fast for the train is quite close");
                                   return fast;
                               }
                    }

                }

            }
            return verySlow;
        }
}

// The destination may be a name or an abbreviation.
// The data arrives from BART in order of train arrival.
// This routine searches the JSON for the value of minutes field.
// It would be nice to have a JSON parsing library avaiable, but we don't.
String getMinutes(char *reply, String destination) {

            String str = reply;
            // find the first mention of this destination
            int destLoc = str.indexOf(destination);
            if(destLoc == -1 ) return "No such train";
            // consider only the part of the string beginning with destination
            str = str.substring(destLoc);
            // find the location of the minutes
            int minLoc = str.indexOf("minutes");
            // skip over text and return string holding minutes or Leaving
            str = str.substring(minLoc + 9);
            // We are looking at something like "7",  or  "23",
            // Find the double quote and comma.
            int doubleQuoteCommaLoc = str.indexOf("\",");
            // Get the substring - skipping the first double quote
            // and stopping just befroe the double quote and comma.
            str = str.substring(1,doubleQuoteCommaLoc);
            // return the string with the number of minutes
            return str;
}



/* Write a string using tcpclient */
void out_to_server(const char *s) {

    size_t numBytesWritten = client.write( (const uint8_t*)s, strlen(s) );
    if(DEBUG) Serial.printf("Sent %d bytes to server. ", numBytesWritten);

}


// The reply data is left in a char array being pointed to by ptr.
// The timeout specifies how long we are willing to wait.
void in_from_server(char *ptr, uint8_t timeout) {
        int pos = 0;
        unsigned long lastTime = millis();

        // client.available() returns the number of bytes from the server that are available for reading
        // While there is nothing to read but we still have time, just wait.
        while( client.available()==0 && millis()-lastTime<timeout) { //timeout
            // waiting on the network to give us some bytes
        }

        // Data has arrived. This becomes the last time data has arrived.
        unsigned long last_time_data_arrived = millis();

        // Take characters from the server.
        // Loop as long as we have data available or more time is available to read.
        // The time available to read is based on the time that the last byte was read.
        // We provide a 500 millisecond waiting period.
        while ( client.available() || (millis()-last_time_data_arrived < 500)) {

            // if there is some data to read
            if (client.available()!= 0) {
                // Read a byte from the server.
                char c = client.read();
                if(DEBUG)Serial.print(c);
                // Set lastData to the time of last read
                last_time_data_arrived = millis();
                // Store the byte in array named ptr.
                ptr[pos] = c;
                // Increment the index for the next read location.
                pos++;
            }
            // If we are out of memory in our read buffer then break
            // out of this loop.
            if (pos >= 2048 - 1) break;

        }
        // Null terminate the array of char to form a c++ string.
        ptr[pos] = '\0';
        // If there is any remaining data from the server, skip it, we have not provided
        // sufficient room.
        while (client.available()) client.read(); // read and drop any extra data


        // Leve some time for things to settle with a brief pause of 400 milliseconds.
        delay(400);
        // Disconnect from the server.
        client.stop();

        // show total bytes read if debug is true
        if(DEBUG){
            Serial.println();
            Serial.print("Done, total bytes read: ");
            Serial.println(pos);
        }
}
