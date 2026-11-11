
CancellationTokenSource source = new();
var token = source.Token;

var apiKey = Environment.GetEnvironmentVariable("AISSTREAM_API_KEY")
    ?? throw new InvalidOperationException(
        "AISSTREAM_API_KEY environment variable is not set.");

using var ws = new ClientWebSocket();

await ws.ConnectAsync(new Uri("wss://stream.aisstream.io/v0/stream"), token);

await ws.SendAsync(new ArraySegment<byte>(Encoding.UTF8.GetBytes($"{{ \"APIKey\": \"{apiKey}\", \"BoundingBoxes\": [[[-11.0, 178.0], [30.0, 74.0]]]}}")), WebSocketMessageType.Text, true, token);

byte[] buffer = new byte[4096];

while (ws.State == WebSocketState.Open)
{
    var result = await ws.ReceiveAsync(new ArraySegment<byte>(buffer), token);
    if (result.MessageType == WebSocketMessageType.Close)
    {
        await ws.CloseAsync(WebSocketCloseStatus.NormalClosure, string.Empty, token);
    }
    else
    {
        Console.WriteLine($"Received {Encoding.UTF8.GetString(buffer, 0, result.Count)}");
    }
}
