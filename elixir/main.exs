Mix.install([{:websockex, "~> 0.5.1"}])

defmodule AisStream do
  use WebSockex

  def start_link(api_key) do
    WebSockex.start_link("wss://stream.aisstream.io/v0/stream", __MODULE__, api_key,
      handle_initial_conn_failure: true
    )
  end

  # aisstream closes the connection if the subscription doesn't arrive within a few seconds,
  # on every (re)connection. handle_connect/2 can't send frames, so we message ourselves.
  @impl true
  def handle_connect(_conn, api_key) do
    send(self(), :subscribe)
    {:ok, api_key}
  end

  @impl true
  def handle_info(:subscribe, api_key) do
    subscription = %{
      APIKey: api_key,
      # WebSockex doesn't support permessage-deflate, which aisstream needs to serve
      # full bandwidth, so keep the area small. For the whole world use a client with compression.
      BoundingBoxes: [[[43.0, 7.5], [44.6, 10.5]]],
      FilterMessageTypes: ["PositionReport"]
    }

    {:reply, {:text, JSON.encode!(subscription)}, api_key}
  end

  # aisstream sends JSON in binary frames
  @impl true
  def handle_frame({_type, message}, api_key) do
    case JSON.decode!(message) do
      %{"error" => error} ->
        IO.puts("Error: #{error}")
        System.halt(1)

      %{"MessageType" => "PositionReport", "Message" => %{"PositionReport" => report}} ->
        IO.puts(
          "ShipId: #{report["UserID"]} Latitude: #{report["Latitude"]} Longitude: #{report["Longitude"]}"
        )

      _ ->
        :ok
    end

    {:ok, api_key}
  end

  @impl true
  def handle_disconnect(%{attempt_number: attempt, reason: reason}, api_key) do
    # aisstream also closes without an error message when the API key is invalid
    IO.puts("Disconnected (#{inspect(reason)}), reconnecting...")
    Process.sleep(min(attempt * 1_000, 30_000))
    {:reconnect, api_key}
  end
end

{:ok, _pid} = AisStream.start_link(System.fetch_env!("AISSTREAM_API_KEY"))
Process.sleep(:infinity)
