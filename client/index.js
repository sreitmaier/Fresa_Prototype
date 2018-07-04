var mqtt = require('mqtt')
var fs = require('fs');
var fileName = './static/stations.json';
var file = require(fileName);

var TMClient = require('textmagic-rest-client');

var c = new TMClient('sabinereitmaier', 'KEykHoBTj0MjGbNeIAK25uwUZEaRtB');

var host = "mqtt://m21.cloudmqtt.com:18990"
var options = {
  username: "ixysflsb",
  password: "yqVLDBTa3h1c"
}

var smsTest = false;

var express = require('express');
var app = express();
var http = require('http').Server(app);
var io = require('socket.io')(http);

app.use(express.static('static'));

app.get('/', function (req, res) {
  res.sendFile(__dirname + '/index.html');
});

app.get('/qr', function (req, res) {
  res.sendFile(__dirname + '/qr/index.html');
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

  socket.on('open lock', function (msg) {
    client.publish(msg, "open lock")
  })

  socket.on('reserve', function (data) {
    client.publish(data.topic, data.message)
  })

  client.on('message', function (topic, message) {
    // message is Buffer
    // console.log(message.toString())
    // console.log(topic.toString())

    // get Number from Topic
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

    if (message.toString() == "loaded") {
      if (smsTest) {
        c.Messages.send({
          text: 'Deine Bestellung wartet nun in deiner Speisekamemr auf dich. Viel Spass beim Kochen.',
          phones: '491733030149',
          from: "Fresa"
        }, function (err, res) {
          console.log('Messages.send()', err, res);
        });
      } else if (!smsTest) {
        console.log("sms send");
      }
    }
  })
});


http.listen(3001, function () {
  console.log('listening on http://localhost:3001');
});