var wmessage = "Message from broker";
// Create a client instance
client = new Paho.MQTT.Client("m16.cloudmqtt.com", 35432, "WebFarmClient");

// set callback handlers
client.onConnectionLost = onConnectionLost;
client.onMessageArrived = onMessageArrived;

var options = {
    useSSL: true,
    userName: "idimoiey",
    password: "DtiBxZxDcQsI",
    onSuccess:onConnect,
    onFailure:doFail
}

// connect the client
client.connect(options);

// called when the client connects
function onConnect() {
    // Once a connection has been made, make a subscription and send a message.
    console.log("onConnect");
    client.subscribe("/cloudmqtt");
    message = new Paho.MQTT.Message("Hello CloudMQTT");
    message.destinationName = "/cloudmqtt";
    client.send(message);
}

function doFail(e){
    console.log(e);
}

// called when the client loses its connection
function onConnectionLost(responseObject) {
    if (responseObject.errorCode !== 0) {
    console.log("onConnectionLost:"+responseObject.errorMessage);
    }
}

// called when a message arrives
function onMessageArrived(message) {
    console.log("onMessageArrived:"+message.payloadString);
    wmessage = message.payloadString
}