use serde::{Deserialize, Serialize};
use std::error::Error;
use url::Url;

type Message = serde_json::Value;

fn main() -> Result<(), Box<dyn Error>> {
    // Aisstream
    let aisstream_api_url: Url = Url::parse("wss://stream.aisstream.io/v0/stream")?;
    let aisstream_api_key: String = std::env::var("AISSTREAM_API_KEY")?;

    // WebSocket
    println!("Connecting `{aisstream_api_url}`");
    let (mut socket, _) = tungstenite::connect(aisstream_api_url)?;

    // Authentication
    println!("Authenticating...");
    socket.send(tungstenite::Message::Text(
        serde_json::json!({
            "APIKey": aisstream_api_key,
            "BoundingBoxes": [[[-180, -90], [180, 90]]]
        })
        .to_string(),
    ))?;
    let message = match socket.read()? {
        tungstenite::Message::Binary(message) => {
            match serde_json::from_slice::<AuthMessage>(&message)? {
                AuthMessage::AuthError(message) => {
                    return Err(format!("Authentication error: {message:?}").into());
                }
                AuthMessage::Message(message) => Some(message),
            }
        }
        _ => None,
    };
    println!("Successfully authenticated");
    if let Some(message) = message {
        print_message(&message);
    }

    // Loop
    loop {
        let message = match socket.read()? {
            tungstenite::Message::Binary(message) => serde_json::from_slice::<Message>(&message)?,
            tungstenite::Message::Close(message) => {
                return Err(format!("Connection closed: {message:?}").into());
            }
            _ => continue,
        };
        print_message(&message);
    }
}

fn print_message(message: &Message) {
    println!("Message: {message}");
}

#[derive(Serialize, Deserialize, Clone, Debug)]
#[serde(untagged)]
pub enum AuthMessage {
    AuthError(AuthError),
    Message(Message),
}

#[derive(Serialize, Deserialize, Clone, Debug)]
pub struct AuthError {
    pub error: String,
}
