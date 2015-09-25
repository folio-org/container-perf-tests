// Load the http module to create an http server.
var http = require('http');

// Configure our HTTP server to respond with Hello World to all requests.
var server = http.createServer(function (req, res) {
  var count = 0;
  req.on('data', function (data) {
    count += data.length;
  });
  req.on('end', function () {
      res.writeHead(200, {"Content-Type": "text/plain"});
      res.end("Skipped over "+count+"\n");
  });
});

// Listen on port 8000, IP defaults to 127.0.0.1
server.listen(8080);
console.log("Server running at http://127.0.0.1:8080/");
