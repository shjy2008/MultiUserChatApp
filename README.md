
# Multi-User Chat App

A TCP-based multi-user chat application that allows multiple clients to join a shared chat room, send messages, change usernames, and receive real-time notifications.

## Features

- Connect multiple clients simultaneously via TCP
- Auto-assigned default usernames (`User1`, `User2`, ...)
- Change your display name at any time
- Broadcast notifications when users join, leave, or rename themselves
- Live chatter count updates on disconnect

## Project Structure

| File | Description |
|------|-------------|
| `chatserver.c` | Server that manages connections and routes messages between clients |
| `chatclient.c` | Client for connecting, sending, and receiving messages |
| `Makefile` | Compiles both `chatserver.c` and `chatclient.c` |
| `unp.h` | Required header file (unmodified) |

## Prerequisites

- Linux or macOS
- The [unpv13e](https://github.com/unpbook/unpv13e) repository cloned and built on your machine

## Setup & Compilation

1. Clone the unpv13e repository if you haven't already:
   ```bash
   git clone https://github.com/unpbook/unpv13e.git
   ```

2. Create a subfolder inside the `unpv13e` directory and place all four files (`chatserver.c`, `chatclient.c`, `Makefile`, `unp.h`) into it.

3. Navigate to that subfolder and compile:
   ```bash
   make
   ```

## Usage

### Start the Server

```bash
./chatserver 127.0.0.1 65530
```

The server will start listening on `127.0.0.1` port `65530`.

### Connect a Client

Open one or more new terminal windows and run:

```bash
./chatclient 127.0.0.1 65530
```

- If you're the first to join, you'll see: `You are the first chatter!`
- Otherwise, the names of all current chatters will be displayed.
- Your default username (e.g., `User1`) will be shown on connect.

### Sending Messages

Type any message and press **Enter** — it will be broadcast to all other chatters as:

```
name: your message
```

<img width="1236" height="720" alt="image" src="https://github.com/user-attachments/assets/0d494cf3-8f41-487b-abc1-ad05df256029" />

### Changing Your Name

```
-name=new_name
```

- You'll see: `Your name is changed to: new_name`
- All other chatters will see: `User1's name is changed to: new_name`
- Names longer than 40 characters will be truncated to 40.

### Leaving the Chat

Press **Ctrl+C** in the client terminal. All remaining chatters (and the server) will see:

```
The chatter xxx leaves the chat room. Current chatter count: 1
```

### Stopping the Server

Press **Ctrl+C** in the server terminal. Clients will no longer be able to connect after this.
```

<img width="1152" height="720" alt="image" src="https://github.com/user-attachments/assets/8a52eb3c-f356-41e0-b458-a1c0e4affb30" />
