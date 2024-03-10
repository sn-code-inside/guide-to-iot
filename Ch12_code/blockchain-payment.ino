// Guide to Internet of Things
// Chapter 12
// Part 2 blockchain-payment.ino

/* This program is designed to demonstrate a simple, unattended digital payment service.
It periodically checks a web service to see if money has been paid to its Ethereum account.
If the program detects an increase in its Ethereum balance, it turns on its D7 LED for a
certain number of seconds. The number of seconds the LED is on corresponds to the amount
of increase that was paid to its account. For each ETH, we get 1 second of light.

Before flashing the program to a Particle Photon 2 mocrocontroller, make two changes:

(1) Change the IP address to the correct address of the machine where the web service is running.
    Currently, it is set with char myHost[] = "192.168.86.250";
(2) Change the Ethereum account address to the address of the microcontroller's
Ethereum account. Currently, it is set with
char pathWithEthereumAddress[] = "/getBalance/0xf39Fd6e51aad88F6F4ce6aB8827279cffFb92266";

*/


// This program will make HTTP requests to a web service.
#include <HttpClient.h>
// Standard Particle header.
#include "Particle.h"

// This is the light this program will control.
const pin_t MY_LED = D7;


// http will be of class HttpClient.
HttpClient http;

// An array of structs is used to represent the http header message sent to the server.
// We expect to receive a JSON response.
http_header_t headers[] = {

    { "Content-Type", "application/json" },
    { "Accept" , "application/json" },
    { "Accept" , "*/*"},
    // null terminate the array
    { NULL, NULL }
};

// A request will include the headers
http_request_t request;
// Get an HTTP response with JSON
http_response_t response;

// Service  IP location. This will normally need to change.
char myHost[] = "192.168.86.250";
// Service port
int port = 3000;

// This is the path to Node.js service.
// Specify the microcontroller's Ethereum account address on the path.
// This will normally need to change.

char pathWithEthereumAddress[] = "/getBalance/0xf39Fd6e51aad88F6F4ce6aB8827279cffFb92266";

// Handle time values.
// oldCallTime is used for timing the calls to the servie.
unsigned int oldCallTime = 0;
// oldSecondsTime is used check the LED every second.
unsigned int oldSecondsTime = 0;

// We will make a remote call every 5 seconds
unsigned int callInterval = 5000;
// And check the light every one second
unsigned int secondInterval = 1000;


// Hold Ethereum balance
int prevBalance = 0;
// Add to secondsBucket if we see an increase in the controller's account.
int secondsBucket = 0;

// We will fill up these arrays from the response.
// We want the address and the balance on the account.
char addr[1000];
char balance[1000];

// Do this on start up.
// Note that we make a call for the initial balance on startup.
// The light will come on when there is an increase in the balance.
void setup() {

    Serial.begin(9600);
    request.hostname = myHost;
    request.port = port;
    request.path = pathWithEthereumAddress;
    pinMode(MY_LED, OUTPUT);
    // Turn light off
    digitalWrite(MY_LED, LOW);
    // get initial balance fro the blockchain
    prevBalance = callAndGetTruncatedEthBalance();
}


void loop() {


     // Every second take 1 away from the seconds bucket or set it to 0.
     // If there are seconds in the bucket, turn the light on.
     // If there are no seconds left in the bucket, turn the light off.
     if (millis() - oldSecondsTime >= secondInterval) {

        if (secondsBucket > 0) {
              digitalWrite(MY_LED, HIGH);
              secondsBucket = secondsBucket - 1;
        }
        else {
            digitalWrite(MY_LED, LOW);
            secondsBucket = 0;
        }

        oldSecondsTime = millis();
     }


     // Every 5 seconds (callInterval) make a call to the service
     // and if we have an increase, place seconds in the seconds bucket.
     if (millis() - oldCallTime >= callInterval) {

        int bal = callAndGetTruncatedEthBalance();
        // If there is an increase, add seconds to the bucket.
        if(bal > prevBalance) {
            secondsBucket = secondsBucket + (bal - prevBalance);
        }
        // The previous balance becomes the new balance.
        prevBalance = bal;
        // Update oldCallTime with new time
        oldCallTime = millis();
     } // end if

}
// Make a call to a web application to
// learn the balance in Eth of this account.

int callAndGetTruncatedEthBalance() {

        // result will hold the new balance
        int result = 0;

        // Make the HTTP Get request and include the Ethereum address.
        http.get(request, response, headers);

        // If all went well...
        if(response.status == 200) {

             JSONValue outerObj = JSONValue::parseCopy(response.body);
             JSONObjectIterator iter(outerObj);

             // More than one name-value pair is returned so use
             // while rather than if. 
             while (iter.next())
             {

                if(iter.name() == "Account")
                {
                   strcpy(addr,iter.value().toString().data());
                   Serial.printf("Account address: %s", addr);
                }
                if (iter.name() == "balance") {
                   strcpy(balance,iter.value().toString().data());
                   result = balanceTruncatedToEth(balance);
                   // Serial.printf("Balance truncated to Eth %d", result );
                }
             } // end while
        } // end if
        else {
            Serial.println("Response not 200");
            result = -1;
        }

        return result;
}


// Balances are returned with positions for full eth followed by 18 positions
// for fractions of an eth. Here, we remove the 18 positions from consideration.
int balanceTruncatedToEth(char *balanceAsString) {

    int len = strlen(balanceAsString);     // len of entire string
    int ethCharactersLen = len - 18;       // len of charaters representing Eth

    if(ethCharactersLen <= 0) return 0;    // handle case where we have no full eth

    char* substr = (char *) malloc(ethCharactersLen + 1);   // make room plus one for null
    strncpy(substr, balanceAsString, ethCharactersLen);   // copy Eth characters
    substr[ethCharactersLen] = '\0';   // null terminate

    int result = atoi(substr); // convert ascii to int

    free(substr); // free temporary memory created with malloc

    return result;  // return the number of Eth
}
