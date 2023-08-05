import asyncio
import websockets
import json
import ssl
import pathlib
from datetime import datetime, timezone

## unsecure ssl approach - TESTING ONLY
ssl_context = ssl.SSLContext()
ssl_context.check_hostname = False
ssl_context.verify_mode = ssl.CERT_NONE

async def connect_ais_stream():

    async with websockets.connect("wss://stream.aisstream.io/v0/stream",ssl=ssl_context) as websocket:
        subscribe_message = {"APIKey": '<YOUR API KEY>', "BoundingBoxes": [[[-180, -90], [180, 90]]]}

        subscribe_message_json = json.dumps(subscribe_message)
        await websocket.send(subscribe_message_json)

        async for message_json in websocket:
            message = json.loads(message_json)
            message_type = message["MessageType"]

            if message_type == "PositionReport":
                # the message parameter contains a key of the message type which contains the message itself
                ais_message = message['Message']['PositionReport']
                print(f"[{datetime.now(timezone.utc)}] ShipId: {ais_message['UserID']} Latitude: {ais_message['Latitude']} Longitude: {ais_message['Longitude']}")

if __name__ == "__main__":
    asyncio.run(connect_ais_stream())
