import 'ais_stream_websocket_client.dart';

void main() {
  final aissstream = AISStreamWebsocketClient(
    'wss://stream.aisstream.io/v0/stream',
  );
  aissstream.connect();
}
