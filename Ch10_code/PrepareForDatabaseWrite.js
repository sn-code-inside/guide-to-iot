// Guide to Internet of Things
// Chapter 10
// Part 4 PrepareForDatabaseWrite.js


// PrepareForDatabaseWrite
// take water value from http message
var value = msg.payload.stormWaterValue;
// Show the value in debug window
// node.warn(value);
// take the current time
var time = Date.now();
// Show the time in debug window
// node.warn(time);

// create a new object to hold a database record
msgRecord = {topic:'Photon 2 Water Level', payload:value, timeStamp:time}
// make it a JSON string
msgRecordString = JSON.stringify(msgRecord);
// add it to the msg object
// Sqlite wants the data in the params property
msg.params = { $thetime:time, $thevalue:msgRecordString}
return msg;
