// Guide to Internet of Things Chapter 4
// File name: VisitBARTWebService.js
// This web service visits the BART API to collect the time
// of arrival of the next train that departs the source
// and leaves for the detination. This program is a client
// of a web service (BART) and also acts as a server - returning
// the number of minutes until the next train arrives.
// The source and destination are provided from the comand line
// at startup.
// If there is a train departing in 5 minutes from source to destination then the
// JSON string {"Arrival" : "5"} is returned.
// If there is a train departing very soon from source to destination then the
// JSON string {"Arrival" : "Leaving"} is returned.
// If there is no train going to the destination then the JSON string
// {"Arrival" : "-1"} is returned.
// If the source station abbreviation is incorrect the program issues
// an correct usage message and halts.
// Before running this program. Be sure to have node installed and
// run 'npm init' and 'npm install express' in the same directory as this code.
// You will also need a connection to the internet.

// In order to execute this web service code, enter the CLI command
// node VisitBARTWebService.js SourceAbbreviation DestinationAbbreviation

// Create an array of strings from the command line.
// Start from index 2 (this is the third string on the command line).
// We do not want to include "node VisitBARTWebService" - strings 0 and 1.
var stringsFromCommandLine = process.argv.slice(2);

// In order to continue, we need to have both the source and destination abbreviations.
if(stringsFromCommandLine[0] === undefined || stringsFromCommandLine[1] === undefined) {
  console.log("Usage error: node VisitBart.js SourceAbbreviation DestinationAbbreviation");
  console.log("For example, node VisitBART.js sfia antc");
  process.exit(0);
}
// Set up a string to hold the source abbreviation.
var mySource = stringsFromCommandLine[0];

// Set up a string to hold the destination abbreviation.
var myDestination = stringsFromCommandLine[1];

// Tell the user the source and destination
console.log("On each http visit, we will fetch a schedule from " + mySource + " to " + myDestination);

// Setup the express web server to handle the http requests.
const express = require('express')
const app = express()

// This code is a server but also a client of the BART API.
// Here we get access to an https client so that we can make
// calls to the BARTAPI.
const https = require('https');

// This service will listen on TCP port 3000.
const port = 3000


// Set up a listener and display a note to the console.
app.listen(port, () => {
  console.log('To cause a visit to the Bart API, use an HTTP get request and visit http://localhost:' + port +'/minutesUntilArrival');
})

// This variable will hold the http resonse that we will send back to the visitor.
var httpResponse;

// Handle an HTTP GET request from a microcontroller or browser.
app.get('/minutesUntilArrival', (req, res) => {

  // announce a new visitor
  console.log("\nNew Get request");
  // We will send data back with the httpResponse variable.
  httpResponse = res;
  // Call the visit BART API. When the response from BART arrives, write the response data
  // to the httpResponse variable. The response will go back to the client.
  visitBARTAPI();

})

// This function makes a call to BART and esatblishes callback handlers.
function visitBARTAPI(res) {
  // Make the call to BART and provide a callback handler
  const myHttpRequest = https.get('https://api.bart.gov/api/etd.aspx?cmd=etd&orig='+mySource+'&key=MW9S-E7SL-26DU-VV8V&json=y', handleBARTResponse);

  // Handle error in request. For example, this method runs if the '.gov' were removed
  // from the URL.
  myHttpRequest.on('error', (err) => {
     console.log("Error: " + err.message);
     process.exit(0);
  }); // end of myHttpRequest.on
} //  end of VisitBARTAPI and handlers established.

// Callback handler for handling the response.
// Called by node.js when a response arrives.
function handleBARTResponse(response) {
  // This function establishes handlers for the http response object.
  // When a response arrive, we will collect its data in the variable named data.
  let data = '';

  // This anonymous function will be called when data is arriving.
  // Here we simply attach the anonymous function to the response object.
  response.on('data', (chunk) => {
    data += chunk;
  });

  // This anonymous function will be called after the response has arrived.
  // Here we simply attach the anonymous function to the response object.
  response.on('end', () => {
    // We are at the end of the response.
    // Build a Javascript object from the JSON data that has arrived.
    var theResult = JSON.parse(data);

    // Check if there is an error message in the response
    // This will occur if the source is not a correct abbreviation.
    // We halt the program if the source is wrong.

     if(theResult.root.message.hasOwnProperty("error")) {
       console.log("Probable Error in source abbreviation");
       console.log(theResult.root.message.error);
       process.exit(0);
     }

    // Examine each departure (etd) and find the particular destination
    // that we expressed interest in (on the command line).
    // Respond with when the train is expected to leave.
    // This final value is placed in the httpResponse variable.

    // found is set to false because we have not yet found the destination
    // in the response.
    var found = false;
    var min = 0;
    // before seaching for destination, make sure there is an etd for some train
    if(theResult.root.station[0].hasOwnProperty('etd')) {
       // Loop through each etd
       for(j = 0; j < theResult.root.station[0].etd.length; j++) {
         // Search for the first matching destination.
         if(theResult.root.station[0].etd[j].abbreviation.toUpperCase() === myDestination.toUpperCase()) {
           found = true;  // found a match
           console.log(theResult.root.station[0].etd[j]);
           // take the number of minutes until arrival or 'Leaving'
           min = theResult.root.station[0].etd[j].estimate[0].minutes;
           // Before returing to the client, echo status information to the console
           if(min === 'Leaving') {
               console.log("Train is leaving the station");
            }
            else {
              // Describe when the train is expected to arrive
              console.log("Train leaves in " + min + " minutes.");
              if(min >= 15) console.log("You have at least 15 minutes.");
              if(min >= 10  && min < 15)console.log("You have at least ten minutes.");
              if(min >= 5  && min < 10)console.log("You have at least five minutes.");
              if(min < 5)console.log("You have less than five minutes. Better hurry!");
            }
         }
     } // end of for loop
   } // end of if
  // Now, send the data back to the calling client in JSON format.
  if (found) httpResponse.send('{"Arrival":'+'"'+min+'"}');
  // Check if there is no train leaving for this destination.
  if(!found) {
    httpResponse.send('{"Arrival":'+'"'+"-1"+'"}');
    console.log("No destination train to " + myDestination + " found.");
  }
  });
  // Handle response error
  response.on('error', (err) => {
    console.log('Error in response' + err.msg);
  });
} // end of handleBARTResponse
