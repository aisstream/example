import asyncio
import websockets
import json
from datetime import datetime, timezone

async def connect_ais_stream():

    async with websockets.connect("wss://stream.aisstream.io/v0/stream") as websocket:
        subscribe_message = {"APIKey": "<YOUR API KEY>", "BoundingBoxes": [[[-11, -178], [30, 74]]], "FiltersShipMMSI": ["538007480", "636015988", "316003701"], "FilterMessageTypes": ["PositionReport"]}

        subscribe_message_json = json.dumps(subscribe_message)
        await websocket.send(subscribe_message_json)

        async for message_json in websocket:
            message = json.loads(message_json)
            ais_message = message['Message']['PositionReport']
            print(f"[{datetime.now(timezone.utc)}] ShipId: {ais_message['UserID']} Latitude: {ais_message['Latitude']} Longitude: {ais_message['Longitude']}")

if __name__ == "__main__":
    asyncio.run(connect_ais_stream())



