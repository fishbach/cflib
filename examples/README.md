# Chat Server and Client Examples

These examples demonstrate a simple chat application using cflib's RMI and WebSocket features.

## Architecture

### chatserver
- WebSocket server on port 8080
- Provides a single RMI service (`ChatService`) with:
  - `sendMessage(String)` method - broadcasts messages to all connected clients
  - `newMessage` signal - sent to all clients when a message arrives
- Uses `WSCommManager` for connection management
- Uses `RMIServer` for RMI calls

### chatclient
- WebSocket client connecting to the server
- Maintains client identity via server-assigned ID
- Sends messages to server via RMI
- Receives broadcasts via signal mechanism (Tag 3)
- Interactive command-line interface
