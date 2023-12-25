// Guide to Internet of Things Chapter 4
// VisitBART.js
// Visit the API of the Bay Area Rapid Transit system in San Francisco.
// To use a browser for trains leaving San Francisco International Airport,
// visit https://www.bart.gov/schedules/eta?stn=SFIA.
// This program uses station codes found here:
// https://api.bart.gov/docs/overview/abbrev.aspx
// A few station codes:
//    rich Richmond
//    sbrn San Bruno
//    sfia San Francisco International Airport
//    pitt Pittsburg/Bay Point
// Run with the following command:
// node VisitBART.js SourceStationCode DestinationStationCode
const https = require('https');

// Create an array of strings from the command line.
// Start from index 2 (this is the third string on the command line).
// We do not want to include "node VisitBART" - strings 0 and 1.
var stringsFromCommandLine = process.argv.slice(2);

// We need to have the source and destination abbreviations.
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
console.log("Searching for schedule from " + mySource + " to " + myDestination);

// This function makes a call to BART and esatblishes callback handlers.
function visitBARTAPI() {
  // Make the call to BART and provide a callback handler
  const myHttpRequest = https.get('https://api.bart.gov/api/etd.aspx?cmd=etd&orig='+mySource+'&key=MW9S-E7SL-26DU-VV8V&json=y', handleBARTResponse);

  // Handle error in request. For example, this method runs if the '.gov' were removed
  // from the URL.
  myHttpRequest.on('error', (err) => {
     console.log("Error: " + err.message);
     process.exit(0);
  }); // end of myHttpRequest.on
} // end of VisitBARTAPI

// Callback handler for handling the response.
// Called by node.js when a response arrives.
function handleBARTResponse(response) {
  // This function establishes handlers ro the http response object
  // When a response arrive, we will collect its data in the variable named data.
  let data = '';

  // This anonymous function will be called when data is arriving.
  // Here we simply attach the anonymous function to the response object.
  response.on('data', (chunk) => {
    data += chunk;
  }); // end of on 'data'

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
    // State when the train is expected to leave.

    // found is set to false because we have not yet found the destination
    // in the response.[0]
    var found = false;
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
  // Check if there is a train leaving for this destination.
  if(!found) console.log("No destination train to " + myDestination + " found.");
  // Tell the user we will continue to check.
  console.log("We are checking again in 15 seconds.");
  });
  // Handle response error
  response.on('error', (err) => {
    console.log('Error in response' + err.msg);
  });
} // end of handleBARTResponse

// This code runs on startup.
console.log("Checking for train leaving from "+ mySource + " to " + myDestination);

// Call visitBARTAPI at start and then every 15 seconds.
visitBARTAPI();
// Visit the routine visitBARTAPI every 15 seconds
setInterval(visitBARTAPI, 15*1000);
// We now have completed the program but the handlers are still running.
