using System;
using System.Net.WebSockets;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace aisStream.SimpleExample
{
    internal class Program
    {
        public static async Task Main(string[] args)
        {
            CancellationTokenSource source = new CancellationTokenSource();
            CancellationToken token = source.Token;
            using (var ws = new ClientWebSocket())
            {
                await ws.ConnectAsync(new Uri("wss://stream.aisstream.io/v0/stream"), token);
                await ws.SendAsync(new ArraySegment<byte>(Encoding.UTF8.GetBytes("{ \"APIKey\": \"<YOUR API KEY>\", \"BoundingBoxes\": [[[-11.0, 178.0], [30.0, 74.0]]]}")), WebSocketMessageType.Text, true, token);
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
                        Console.WriteLine($"Received {Encoding.Default.GetString(buffer, 0, result.Count)}");
                    }
                }
            }
        }
    }
}
