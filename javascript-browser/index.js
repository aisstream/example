const container = document.getElementById("position-data");

const socket = new WebSocket("wss://stream.aisstream.io/v0/stream");

socket.onopen = function (event) {
  console.log("socket is open", event);
  const subscriptionMessage = {
    Apikey: "<YOUR API KEY>",
    BoundingBoxes: [
      [
        [-180, -90],
        [180, 90],
      ],
    ],
  };
  socket.send(JSON.stringify(subscriptionMessage));
};

socket.onmessage = function (event) {
  const aisMessage = JSON.parse(event.data);
  if (aisMessage["MessageType"] === "PositionReport") {
    let positionReport = aisMessage["Message"]["PositionReport"];
    const positionMessage = `ShipId: ${positionReport["UserID"]} Latitude: ${positionReport["Latitude"]} Latitude: ${positionReport["Longitude"]}`;
    console.log(positionMessage);
    const span = document.createElement("span");
    span.innerText = positionMessage;
    container.appendChild(span);
  }
};

socket.onclose = function (event) {
  console.log("socket is closed", event);
};

console.log("created socket", socket);
