var mqtt = require('mqtt')
var fs = require('fs');
var fileName = './static/stations.json';
var file = require(fileName);

const accountSid = 'ACdcbcf7a28edb77afa1f374d1baf5a2d6';
const authToken = '9c887f15a2f4ecc090e662071ab7caee';
const twilio = require('twilio')(accountSid, authToken);
var smsTest = false;

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

  socket.on('locked', function (msg) {
    client.publish(msg, "loaded");
    if (smsTest) {
      twilio.messages
        .create({
          body: 'Deine Bestellung wurde gerade abgegeben. Hole sie mit deiner Mitgliedskarte ab. Viel Spaß beim auspacken.',
          from: '+4915735993212',
          to: '+4915788718202'
        })
        .then(message => console.log(message.sid))
        .done();
    }
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
  })
});


http.listen(3001, function () {
  console.log('listening on http://localhost:3000');
});