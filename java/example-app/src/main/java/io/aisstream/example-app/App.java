package io.aisstream.example_app;

import java.net.URI;
import java.net.URISyntaxException;
import java.util.Map;

// example connecting to aisstream.io using the popular websocket library from http://github.com/TooTallNate/Java-WebSocket
public class App {
    public static void main(String[] args) throws URISyntaxException{
        System.out.println("starting");
        AisStreamWebsocketClient client = new AisStreamWebsocketClient(new URI(
            "wss://stream.aisstream.io/v0/stream"));
        client.connect();
    }
}