//Using the HiveMQ public Broker, with a random client Id
var client = new Messaging.Client(
  'vemcompai.zapto.org',
  8080,
  'myclientid_' + parseInt(Math.random() * 100, 10)
);

//Gets  called if the websocket/mqtt connection gets disconnected for any reason
client.onConnectionLost = function (responseObject) {
  //Depending on your scenario you could implement a reconnect logic here
  alert('connection lost: ' + responseObject.errorMessage);
};

//Gets called whenever you receive a message for your subscriptions
client.onMessageArrived = function (message) {
  //Do something with the push message you received
  $('#messages').html(`<h1>${message.payloadString} Kg</h1>`);
  console.log(message.payloadString)


  var myChart = echarts.init(document.getElementById('main'));
  option = {
    title: {
      text: "Kg",

      left: "center",

      bottom: "100px"
    },
    series: [{
      type: 'gauge',
      max: 10,
      progress: {
        show: true,
        width: 18
      },
      axisLine: {
        lineStyle: {
          width: 18
        }
      },
      axisTick: {
        show: true,

      },
      splitLine: {
        length: 15,
        lineStyle: {
          width: 2,
          color: '#999'
        }
      },
      axisLabel: {
        distance: 30,
        color: '#999',
        fontSize: 15,

      },
      anchor: {
        show: true,
        showAbove: true,
        size: 25,
        itemStyle: {
          borderWidth: 10
        }
      },
      title: {
        show: false
      },
      detail: {
        valueAnimation: true,
        fontSize: 50,
        offsetCenter: [0, '70%']
      },
      data: [{
        value: message.payloadString
      }]
    }]
  };
  myChart.setOption(option);



};

//Connect Options
var options = {
  timeout: 3,
  //Gets Called if the connection has sucessfully been established
  onSuccess: function () {
    alert('Connected');
  },
  //Gets Called if the connection could not be established
  onFailure: function (message) {
    alert('Connection failed: ' + message.errorMessage);
  },
};

//Creates a new Messaging.Message Object and sends it to the HiveMQ MQTT Broker
var publish = function (payload, topic, qos) {
  //Send your message (also possible to serialize it as JSON or protobuf or just use a string, no limitations)
  var message = new Messaging.Message(payload);
  message.destinationName = topic;
  message.qos = qos;
  client.send(message);
};

