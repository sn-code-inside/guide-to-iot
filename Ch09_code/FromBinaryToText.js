
// Guide to Internet of Things Chapter 9

// This is Javascript logic that is placed inside
// a Node-RED function node.

// It takes a message object as input, modifies its
// payload and returns the same message object.

// When working with a Node-RED function node, we need to
// return an object - usually a message object - never
// a number or a string.

// Note: when debugging use node.warn("x == " + x);

// This function converts a binary payload to text.
// We have a msg object as input.
// The incoming msg.payload is a JSON string.
// We want to convert the string in msg.payload
// into a Javascript object.
var msgInObject = JSON.parse(msg.payload);

// We need the data from the array.
// The array is within a JSON string.
// Drill down in the object to the array.
// Note that "2a1c" is a BLE standard number for the
// temperature characteristic. It is a key in the JSON string
// that we interrogate here.
var arrObj = msgInObject["characteristics"]["2a1c"]["data"];

// Create an object to collect the new output.
var msgOutObject = new Object();

// Place the time in the object
msgOutObject.temperatureTime = new Date();

// Let's examine the least significant bit in the first byte.
// We will perform an AND operation with 1.
// If the result is 1 then the temperature is in Fahrenheit.
// If the result is 0 then the temperature is in Celsius.
if ((arrObj[0] & 1) === 1) {
    msgOutObject.temperatureUnits = 'Fahrenheit';
}
else {
     msgOutObject.temperatureUnits = 'Celsius';
}
// Consult the BLE documentation for exact encodings
// for each body location. There are several not covered here.
// Here, we test only for mouth or finger.
if (arrObj[5] === 6) {
    msgOutObject.temperatureLocation = 'mouth';
}
else if (arrObj[5] === 4) {
    msgOutObject.temperatureLocation = 'finger';
}

// Create a number from bytes[1], [2], and [3].
// arrObj[3] is most significant and arrObj[1] is least significant.
var numericValue = arrObj[1];   // 1's place
numericValue = arrObj[2] * 256 + numericValue;
numericValue = arrObj[3] * 256 * 256 + numericValue;

// The encoding includes an exponent.
// The exponent is in 2's complement notation.
// If the leftmost bit is 1, the number is negative.
// If the number is negative, we flip the bits and add 1.
var exponent = 0;

// Find the value of the exponent in byte [4].
// Is the leftmost bit on ?
if((arrObj[4] & 0x80) !== 0) {
    // the number is negative. Flip the bits with XOR 1.
    exponent = arrObj[4] ^ 0xFF;
    // add 1
    exponent = exponent + 1;
    // make it negative
    exponent = (exponent * (-1));

}
// raise the numericValue to the exponent

msgOutObject.temperatureValue = numericValue * Math.pow(10.0,exponent);

// Change the message payload to include a new JSON string.
// We do this by calling stringify on the output object.
msg.payload = JSON.stringify(msgOutObject);

// Pass on to the next node in Node-RED
return msg;
