import asyncio
import websockets
import json

async def connect_ais_stream():
    async with websockets.connect("wss://stream.aisstream.io/v0/stream") as websocket:
        subscribe_message = {"APIKey": "<YOUR API KEY>", "BoundingBoxes": [[[-90, -180], [90, 180]]]}

        subscribe_message_json = json.dumps(subscribe_message)
        await websocket.send(subscribe_message_json)

        async for message_json in websocket:
            message = json.loads(message_json)
            message_type = message["MessageType"]
            print(message["MetaData"])

            if message_type == "PositionReport":
                ais_message = json.loads(message['Message']) # parse the message json
                print(f"ShipId: {ais_message['UserID']} Latitude: {ais_message['Latitude']} Latitude: {ais_message['Longitude']}")


if __name__ == "__main__":
    asyncio.run(asyncio.run(connect_ais_stream()))



