
// Guide to Internet of Things Chapter 3
// File name: index.js
// In order to execute this code, enter the CLI command
// node index.js
// Get requests from browsers will be acknowledged as a courtesy.
// A browser may visit with http://localhost:3000/microcontrollerEvent
// The main purpose is to handle HTTP POST requests to the following URL:
// http://localhost:3000/microcontrollerEvent

// Each visit logs data to the file MicrocontrollerLogger.txt.

// Setup the express web server.
const express = require('express')
const app = express()

// The HTTP request will include JSON data.
// We need to parse JSON data.
app.use(express.json());

// Listen on TCP port 3000.
const port = 3000

// We will write data about each visit to a log file.
// So, we need access to the file system of this server.
// We will use {flags:'a'} because we want to append to the log file.
// If the file does not exist, it will be created.
// The file name is MicrocontrollerLogger.txt.
var fs = require('fs');
var stream = fs.createWriteStream("MicrocontrollerLogger.txt", {flags:'a'});

// Set up a listener and display a note to the console.
app.listen(port, () => {
  console.log(`Logging encrypted data from POST visits to http://localhost:${port}/microcontrollerEvent`)
})

// Used this variable to count visitors
var visitor = 0;

// Handle an HTTP POST request from a microcontroller.
app.post('/microcontrollerEvent', (req, res) => {

  // announce a new visitor
  console.log("\nNew Post request");

  // Increment the visitor count
  visitor++;
  console.log("Visitor Number" + visitor);

  // Get the time stamp of this visit.
  var visitTimeStamp = Date.now();

  // Access the encrypted data in JSON format from the request body
  var encryptedDataInJSON = req.body;

  // Show the time stamp and encrypted data on the console.
  console.log("Time:" + visitTimeStamp + " JSON Message: " + JSON.stringify(encryptedDataInJSON));
  console.log("Encrypted value = " + encryptedDataInJSON.encryptedData);

  // Record this visit on the log file
  stream.write("Visit: "+ visitor+ " Timestamp " + visitTimeStamp + "," + "Encrypted data: " + JSON.stringify(encryptedDataInJSON) + "\n");

  // Send an HTTP response back to the caller.
  // The encrypted message was received and stored.
  // Send HTTP 200 OK.

  res.status(200).send('OK');
})

app.get('/microcontrollerEvent', (req, res) => {
  // Our microntroller must use POST and not GET.
  // Here, we will serve up a simple OK to a browser.
  res.status(200).send('OK');
  console.log("Browser visit");
})
