var express = require('express');
//var app = require('express')();
var app = express();
var http = require('http').Server(app);
var io = require('socket.io')(http);
var path = require('path');
var clientIp = "";

var usersArray = new Array();

var User = (function() {
    // "private" variables 
    var _bar;

    // constructor
    function User() {};

    // add the methods to the prototype so that all of the 
    // Foo instances can access the private static
    User.prototype.getBar = function() {
        return _bar;
    };
    User.prototype.setBar = function(bar) {
        _bar = bar;
    };

    	this.name = "";
            this.guid = "";
            this.message = "";
    return User;
})();


// The below works but isn't a good practice because it's not scalable
app.use('/socket.io-1.2.0.js', express.static(path.join(__dirname, '/socket.io-1.2.0.js')));
app.use('/jquery-1.11.1.js', express.static(path.join(__dirname, '/jquery-1.11.1.js')));


// You should create a public directory in your project folder and
// place all your static files there and the below app.use() will
// serve all files and sub-directories contained within it.
//app.use('public', express.static(path.join(__dirname, 'public')));


app.get('/', function(req, res){
  res.sendFile(__dirname + '/index.html');
});

	  function S4() {
		return (((1+Math.random())*0x10000)|0).toString(16).substring(1); 
		}

io.on('connection', function(socket){

 var socketId = socket.id;
  clientIp = socket.request.connection.remoteAddress;
  console.log(clientIp);

    // then to call it, plus stitch in '4' in the third group
    var guid = (S4() + S4() + "-" + S4() + "-4" + S4().substr(0,3) + "-" + S4() + "-" + S4() + S4() + S4()).toLowerCase();
    console.log(guid);

    var user = new User();
    usersArray.push(user);
	//alert(guid);
  io.emit('setguid', guid);

  socket.on('chat message', function(msg){
    console.log(msg);
      var user = msg;
      var message = user.name + ":" + user.message;
    io.emit('chat message', message);
  });

  socket.on('setname', function(msg){
      var message = clientIp + ":" + msg;
    //io.emit('chat message', message);
    console.log(message);
  });

});

io.on('disconnect', function(soccket){
     var socketId = socket.id;
  clientIp = socket.request.connection.remoteAddress;
  console.log("disconnect" + clientIp);    
});

http.listen(80, function(){
  console.log('listening on *:80');
});
