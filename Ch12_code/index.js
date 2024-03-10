// Guide to Internet of Things
// Chapter 12 Unattended digital payments
// Part 1
// index.js

// This JavaScript program runs in Node.js. It communicates with a
// local instance of the Ethereum blockchain simulated by Hardhat.
// It provides a web service that takes, as input, on the URL, the
// public address of an Ethereum account (from Hardhat). It returns
// a JSON string holding the account address and the current balance
// in ETH.

// Before running, be sure to install version 5.7.2 of ethers
// by running:
// npm install ethers@5.7.2

// After starting this program, use a browser to visit with a URL like
// this:
//
// http://localhost:3000/getBalance/0x64f63F50074A514E8A6De7d073C5932997C6951c
// The hexadecimal value is an Ethereum address from Hardhat.
// Be sure to use your own address from a running instance of Hardhat.

// This code simply reads the account balance from the blockchain.
// It maintains no private key and, therefore, cannot transfeer funds.
// Use a wallet, such as MetaMask, to transfer funds into an account
// read by this program.
// The program uses Node.js, Express and ethers to run.
// Run at the command prompt with:
// node index.js
// Visit at http://localhost:3000/getBalance/AccountID
// Note: the AccountID is copied and pasted from a running Hardhat nstance.
// From a browser, use the address of an account in the URL to see the account's
// balance.

// Use Express
const express = require('express');
app = express();
// Listen on port 3000
const port = 3000;
// Access needed middleware libraries
const { ethers, JsonRpcProvider } = require("ethers");

// Get the balance for an account from Hardhat using ethers.
async function retrieveBalance(account,myCallBack) {
  const provider = new ethers.providers.JsonRpcProvider("http://localhost:8545");
  // Replace with the address that you want to check the balance of.
  const address = "0xf39Fd6e51aad88F6F4ce6aB8827279cffFb92266";
  const balance = await provider.getBalance(address);
  const etherString = ethers.utils.formatEther(balance);
  console.log(`The balance of ${address} is ${etherString} ETH`);
  myCallBack(balance);
}

// Initial values
var balance = 0;
var account;
var response;

// This function runs after a response from the Ethereum blockchain.
function handleResponse(balance) {
  // We want a JSON response. Build an object and the stringify to json.
  let returnObject = new Object();
  returnObject.account = account;
  returnObject.balance = balance.toString();
  response.send(JSON.stringify(returnObject));
}

// Respond to an HTTP get request from a browser or a microcontroller.
app.get('/getBalance/:account', (req, res) => {
  // The callback will send the response when its ready.
  // Here, we make the response object global.
  response = res;
  // Place the account ID in the global variable account.
  account = req.params.account;
  // Call to make the query.
  retrieveBalance(req.params.account,handleResponse);

})

// When we run this code, we see this message on how to use the program.
app.listen(port, () => {
  console.log(`Blochain query listening for GET at
http://localhost:${port}/getBalance/accountID`);
})
