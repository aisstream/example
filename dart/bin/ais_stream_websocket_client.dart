import 'dart:convert';

import 'package:web_socket_channel/web_socket_channel.dart';

class AISStreamWebsocketClient {
  final String _serverUri;

  AISStreamWebsocketClient(this._serverUri);

  void connect() async {
    final channel = WebSocketChannel.connect(
      Uri.parse(_serverUri),
    );
    await channel.ready;
    channel.stream.listen(onMessage);
    channel.sink.add(
      '{"APIKey":"<YOUR API KEY>","BoundingBoxes":[[[-90,-180],[90,180]]]}',
    );
  }

  void onMessage(dynamic message) {
    final jsonString = utf8.decode(message);
    print(jsonString);
  }
}
