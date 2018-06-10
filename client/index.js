var mqtt = require('mqtt')
var fs = require('fs');
var fileName = './static/stations.json';
var file = require(fileName);



var host = "mqtt://m21.cloudmqtt.com:18990"
var options = {
  username: "ixysflsb",
  password: "yqVLDBTa3h1c"
}

var express = require('express');
var app = express();
var http = require('http').Server(app);
var io = require('socket.io')(http);

app.use(express.static('static'));

app.get('/', function (req, res) {
  res.sendFile(__dirname + '/index.html');
});


io.on('connection', function (socket) {
  var client = mqtt.connect(host, options)
  client.on('connect', function () {
    client.subscribe('#')
  })
  // socket.on('send message', function (data) {

  //   client.publish(data.topic, data.message)

  // })
  socket.on('disconnect', function () {
    console.log('user disconnected');
  });

  client.on('message', function (topic, message) {
    // message is Buffer
    // console.log(message.toString())
    // console.log(topic.toString())
    var slashReg = new RegExp("([^\/]+$)")
    var num = slashReg.exec(topic);
    num = num[0]

    file.features[num].properties.status = message.toString();

    fs.writeFile(fileName, JSON.stringify(file), function (err) {
      if (err) return console.log(err);
      console.log(JSON.stringify(file));
      console.log('writing to ' + fileName);
    });
    socket.emit('message', {
      topic: topic.toString(),
      message: message.toString()
    })
  })
});


http.listen(3000, function () {
  console.log('listening on http://localhost:3000');
});