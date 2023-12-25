// Guide to Internet of Things
// Chapter 10
// Part 3 PrepareForFileWrite.js

// Chapter 10, Part 3
// for testing, use node.warn(msg.payload);

// take water value from http message
var value = msg.payload.stormWaterValue;
// take the current time
var time = new Date();
// create a new message object, including topic
msg.payload = {topic:'Photon 2 Water Level', payload:value, timeStamp:time}
// forward to the next node
return msg;
