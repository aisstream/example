const WebSocket = require('ws');

const socket = new WebSocket("wss://stream.aisstream.io/v0/stream")

socket.onopen = function (_) {
    let subscriptionMessage = {
        Apikey: "<YOUR API KEY>",
        BoundingBoxes: [[[-180, -90], [180, 90]]]
    }
    socket.send(JSON.stringify(subscriptionMessage));
};

socket.onmessage = function (event) {
    let aisMessage = JSON.parse(event.data)
    if (aisMessage["MessageType"] === "PositionReport") {
        let positionReport = JSON.parse(aisMessage["Message"])
        console.log(`ShipId: ${positionReport["UserID"]} Latitude: ${positionReport['Latitude']} Latitude: ${positionReport['Longitude']}`)
    }
};

