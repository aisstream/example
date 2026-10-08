import asyncio
import websockets
import json
import random
from datetime import datetime, timezone

# Reconnect when the stream stays silent this long, even if the connection looks open.
SILENCE_TIMEOUT_SECONDS = 120

async def connect_ais_stream():

    retry_delay = 1
    # Iterating over connect() opens a new connection each time the previous one ends.
    # Enables compression, which aisstream.io requires to serve full message bandwidth.
    async for websocket in websockets.connect("wss://stream.aisstream.io/v0/stream",
                                              compression="deflate"):
        subscribe_message = {"APIKey": "<YOUR API KEY>", "BoundingBoxes": [[[-11, 178], [30, 74]]]}

        subscribe_message_json = json.dumps(subscribe_message)
        confirmed = False
        try:
            await websocket.send(subscribe_message_json)

            while True:
                message_json = await asyncio.wait_for(websocket.recv(), SILENCE_TIMEOUT_SECONDS)
                message = json.loads(message_json)
                message_type = message["MessageType"]

                if message_type == "SubscriptionConfirmation":
                    confirmed = True
                    retry_delay = 1

                if message_type == "PositionReport":
                    # the message parameter contains a key of the message type which contains the message itself
                    ais_message = message['Message']['PositionReport']
                    print(f"[{datetime.now(timezone.utc)}] ShipId: {ais_message['UserID']} Latitude: {ais_message['Latitude']} Longitude: {ais_message['Longitude']}")
        except asyncio.TimeoutError:
            print(f"[{datetime.now(timezone.utc)}] No message for {SILENCE_TIMEOUT_SECONDS} s")
        except websockets.ConnectionClosed:
            hint = "" if confirmed else " before the subscription was confirmed (check the API key and bounding boxes)"
            print(f"[{datetime.now(timezone.utc)}] Connection closed{hint}")

        # Exponential backoff with jitter, as the aisstream.io documentation recommends.
        print(f"[{datetime.now(timezone.utc)}] Reconnecting in {retry_delay} s")
        await asyncio.sleep(retry_delay + random.random())
        retry_delay = min(retry_delay * 2, 60)

if __name__ == "__main__":
    asyncio.run(connect_ais_stream())



