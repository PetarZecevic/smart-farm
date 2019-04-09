var parameters = {
    "temperature" : 25
}

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
    console.log("onConnect");
    client.subscribe("farm/temperature");
}

function doFail(e){
    console.log(e);
}

// called when the client loses its connection
function onConnectionLost(responseObject) {
    if (responseObject.errorCode !== 0) {
    console.log("Connection lost: " + responseObject.errorMessage);
    }
}

// called when a message arrives
function onMessageArrived(message) {
    console.log("Message arrived: " + message.payloadString);
    console.log("Message topic: " + message.destinationName);
    if(message.destinationName == "farm/temperature")
    {
        parameters["temperature"] = message.payloadString;
        console.log("Changed temp to: " + parameters["temperature"]);
    }
}

function getTemperature()
{
    document.clear();
    document.write(parameters["temperature"]);
}