// Guide to Internet of Things
// Chapter 10
// Part 2 WaterSensorWithHTTP.ino

// WaterSensorWithHTTP firmware
// Library: HttpClient included in the Web IDE

// This #include statement was automatically added by the Particle IDE.
#include <HttpClient.h>

// Define the http variable to be of type HttpClient.
 HttpClient http;

 // We always pass Http headers on each request to the Http Server.
 // Here, we only define a single header. The NULL, NULL pair is used
 // to terminate the list of headers.

 // The Content-Type header is used to inform the server
 // of the type of message that it will be receiving. Here,
 // we tell the server to expect to receive data marked up in
 // JSON.

 http_header_t headers[] = {
    { "Content-Type", "application/json" },
    { NULL, NULL }
 };

 // Here we define structures to hold the request and the response data.
 // These are declared with types defined in the header file included above.

 http_request_t request;
 http_response_t response;

// The setup() function runs once when the microcontroller boots up.
 void setup() {

   // The IP address of the server running on our machine.
   // Do not use localhost. The microcontroller would attempt
   // to visit itself with localhost.
   request.ip = IPAddress(192,168,86,25);

   // Specify the port that our server is listening on.
   // This Node-RED instance is listening on port 1880.
   request.port = 1880;

   Serial.begin(9600);

 }

 // Provided with a response, display it to the command line interface.
 void printResponse(http_response_t &response) {
   Serial.println("HTTP Response: ");
   Serial.println(response.status);
   Serial.println(response.body);
 }

 // Post a water value message to the server
 void doPostRequest(int waterValue) {

   // Define a character array large enought to hold the integer
   // represented as a string.
   char waterValueStr[10];
   // Use sprintf to print a formatted string into the char array.
   // The sprintf function is being used to create a string from the integer.
   sprintf(waterValueStr, "%d", waterValue);

   // Specify the URL in the HTTP request.
   request.path = "/stormWaterReading";

   // Build the JSON request
   char json[1000] = "{\"stormWaterValue\":\"";
   // Add the water measurement
   strcat(json,waterValueStr);
   // add the JSON ending
   strcat(json,"\"}");
   // assign the JSON string to the HTTP request body
   request.body = json;
   // post the request
   http.post(request, response, headers);
   // show response
   printResponse(response);

 }

// A0 is conected to the signal pin on the DGZZI Water Level Sensor
const int waterLevelPin = A0;
// We want a report on the level every 5 seconds
const int NUMSECONDS = 5;
int timeCtr = 0;
// Use an int to hold the water level
int waterValue = 0;

// Use the command line and the command:
// "particle serial monitor"
// to view the Serial.println values.
// The loop() function is called repeatedly by the operating system.
void loop() {

     //Make a report every 5 seconds
     if (timeCtr <= millis()) {
         // get the value from A0
          waterValue = analogRead(waterLevelPin);
          // display on the command line
          Serial.println(waterValue);
          // Make an HTTP call
          doPostRequest(waterValue);
          // update timeCtr to be 5 seconds greater than current time
          timeCtr = millis() + (NUMSECONDS * 1000);
     }
}
