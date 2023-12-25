// Guide to Internet of Things
// Chapter 9 Node.js server code

// This file is named index.js.
// This code is used to serve up the single file in the public directory.
// The file being served is named index.html.

// A directory named "public" is available in the current directory.
// The public directory holds index.html containing HTML and Javascript.
// A browser will visit with http://localhost:3000/index.html.
// The file index.html will be fetched by HTTP and loaded into the browser.
// The browser is able to render the HTML and execute the Javascript.

const express = require('express')
const port = 3000
app = express();
// allow access to the files in public
app.use(express.static('public'));

app.listen(port, () => {
  console.log(`Patient temperature data available at  http://localhost:${port}/index.html`)
})
