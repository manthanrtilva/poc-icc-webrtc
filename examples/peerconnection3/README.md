# WebRTC Data Channel CLI Chat

A simple command-line application for peer-to-peer text chat using WebRTC data channels.

## Features

- **Direct P2P Communication**: Uses WebRTC data channels for real-time messaging
- **Manual Signaling**: Simple copy-paste SDP exchange (no signaling server required)
- **Interactive Chat**: Real-time bidirectional text communication
- **Cross-Platform**: Works on Linux, macOS, and Windows

## Building

From the WebRTC source root directory:

```bash
# Generate build files
gn gen out/Default

# Build the CLI application  
ninja -C out/Default libwebrtc/examples:pclient3
```

## Usage

1. Start the application:
```bash
./out/Default/pclient3
```

2. **On Peer 1 (Initiator)**:
   - Type `offer` to create an offer
   - Copy the SDP offer that appears
   - Send it to Peer 2

3. **On Peer 2 (Responder)**:
   - Type `answer`
   - Paste the received SDP offer
   - Press Enter on empty line to finish
   - Copy the generated SDP answer
   - Send it back to Peer 1

4. **Back on Peer 1**:
   - Type `remote`
   - Paste the received SDP answer
   - Press Enter on empty line to finish

5. **Start Chatting**:
   - On both peers, type `chat` to enter chat mode
   - Start typing messages!
   - Type `exit` to leave chat mode

## Commands

- `offer` - Create offer (for initiator)
- `answer` - Set remote offer and create answer
- `remote` - Set remote answer (for initiator) 
- `chat` - Start chat mode
- `help` - Show help
- `quit` - Exit application

## Example Session

**Peer 1:**
```
> offer
[WebRTC] Data channel created
=== COPY THIS OFFER TO THE OTHER PEER ===
v=0
o=- 123456789 2 IN IP4 127.0.0.1
...
========================================
```

**Peer 2:**
```
> answer
Paste the offer SDP (end with empty line):
v=0
o=- 123456789 2 IN IP4 127.0.0.1
...
[empty line]
=== COPY THIS ANSWER TO THE OTHER PEER ===
v=0
o=- 987654321 2 IN IP4 127.0.0.1
...
=========================================
```

**Peer 1:**
```
> remote
Paste the answer SDP (end with empty line):
v=0
o=- 987654321 2 IN IP4 127.0.0.1
...
[empty line]
[DataChannel] State changed to: OPEN

> chat
=== CHAT MODE (type 'exit' to leave) ===
[You]: Hello!
[Peer]: Hi there!
[You]: exit
=== EXITED CHAT MODE ===
```

## Technical Details

- Uses WebRTC unified plan SDP semantics
- Includes STUN server (stun.l.google.com:19302) for ICE
- Data channel configured for ordered, reliable delivery
- No external dependencies beyond WebRTC SDK

## Troubleshooting

- Make sure both peers complete the offer/answer exchange before trying to chat
- If connection fails, check firewall settings
- For testing on same machine, run from different terminal windows
- The data channel state must be "OPEN" before chat mode works