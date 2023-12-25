// Guide to Internet of Things
// Chapter 12 Unattended digital payments
// Part 1
// getBalance.js

// This JavaScript program runs in Node.js. It uses a Web3 object
// to communicate with a local instance of the Ethereum blockchain.
// It provides a web service that takes, as input, on the URL, the
// public address of an Ethereum account (on Ganache). It returns
// a JSON string holding the account address and the current balance
// in ETH.

// After starting this program, use a browser to visit with a URL like this:
// http://localhost:3000/getBalance/0x64f63F50074A514E8A6De7d073C5932997C6951c
// Be sure to use a real address from a running instance of Ganache.

// This code simply reads the account balance from the blockchain.
// It maintains no private key and, therefore, cannot transfeer funds.
// Use a wallet, such as MetaMask, to transfer funds into an account
// read by this program.
// The program uses Node.js, Express and Web3 to run.
// Run at the command prompt with:
// node getBalance.js
// Visit at http://localhost:3000/getBalance/AccountID
// Note: the AccountID is copied and pasted from Ganache. Use the
// address of an account in the URL to see the account's balance.

// Use Express
const express = require('express');
app = express();
// Listen on port 3000
const port = 3000;

// Use Web3
var Web3 = require('web3');

// Check if web3 is available (in the node_modules directory).
if (typeof web3 !== 'undefined')
{
  web3 = new Web3(web3.currentProvider);
}  else
      {
      // Using ganache at port 7545
      // set the provider you want from Web3.providers
      web3 = new Web3(new Web3.providers.HttpProvider('http://localhost:7545'));

   }

 // The account (ID on Ganache) is found from the client's request.
 // the client might be a browser or microcontroller.
 // The balance is found by making a query on Ganache for the account balance.
 var balance = 0;
 var account;

 // Make a call to Ganache and provide a handler for the response
 // from Ganache.
 function retrieveBalance(account,myCallBack) {
   // Use web3 middileware to make the call.
    web3.eth.getBalance(account).then(bal => {
      // Now, we have the balance from the response.
      balance = bal;
      // Pass it to the handler (handleResponse).
      myCallBack(balance);
    })
}
// Global response object
var response;
// This function runs after a response from the Ganache blockchain.
function handleResponse(balance) {
  // We want a JSON response. Build an object and the stringify to json.
  let returnObject = new Object();
  returnObject.account = account;
  returnObject.balance = balance;
  response.send(JSON.stringify(returnObject));
}

// Respond to an HTTP get request
app.get('/getBalance/:account', (req, res) => {
  console.log("We have a visitor");
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
  console.log(`Blochain query listening for GET at http://localhost:${port}/getBalance/accountID`);
})
