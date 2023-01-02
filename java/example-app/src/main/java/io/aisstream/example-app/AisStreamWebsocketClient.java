package io.aisstream.example_app;

import java.net.URI;
import java.net.URISyntaxException;
import java.util.Map;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import org.java_websocket.client.WebSocketClient;
import org.java_websocket.drafts.Draft;
import org.java_websocket.handshake.ServerHandshake;

/**
 * This example demonstrates how to create a websocket connection to a server. Only the most
 * important callbacks are overloaded.
 */
public class AisStreamWebsocketClient extends WebSocketClient {

  public AisStreamWebsocketClient(URI serverURI) {
    super(serverURI);
  }

  @Override
  public void onOpen(ServerHandshake handshakedata) {
    // send subscription message upon connection
    send("{\"APIKey\":\"<YOUR API KEY>\",\"BoundingBoxes\":[[[-90,-180],[90,180]]]}");
  }


  @Override
  public void onMessage(ByteBuffer message) {
    String jsonString = StandardCharsets.UTF_8.decode(message).toString();
    System.out.println(jsonString);
  }

  @Override
    public void onMessage(String message) {
      // unused as aisstream.io returns messages as byte buffers
    }

  @Override
  public void onClose(int code, String reason, boolean remote) {
    // The close codes are documented in class org.java_websocket.framing.CloseFrame
    System.out.println(
        "Connection closed by " + (remote ? "remote peer" : "us") + " Code: " + code + " Reason: "
            + reason);
  }

  @Override
  public void onError(Exception ex) {
    ex.printStackTrace();
  }
}
